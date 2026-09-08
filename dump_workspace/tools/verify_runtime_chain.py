import sys
import struct

GA_BASE = 0x7FF8EC7D0000  # plain variant base from probe

def read_q(data, rva):
    if rva + 8 > len(data):
        return None
    return struct.unpack_from('<Q', data, rva)[0]

def read_u32(data, rva):
    if rva + 4 > len(data):
        return None
    return struct.unpack_from('<I', data, rva)[0]

def read_cstr(data, rva, maxlen=256):
    if rva >= len(data):
        return None
    end = data.find(b'\x00', rva, rva+maxlen)
    if end < 0:
        end = rva+maxlen
    try:
        s = data[rva:end].decode('utf-8', 'replace')
        return s if all(0x20 <= ord(c) < 0x7f or c in 'éüöä' for c in s) else None
    except Exception:
        return None

def main():
    img = sys.argv[1]
    with open(img, 'rb') as f:
        data = f.read()
    print(f'[info] loaded {len(data)} bytes, GA_BASE=0x{GA_BASE:X}')

    # MetadataRegistration v24 layout (offsets from registration_types.hpp):
    #   +0x30 typesCount, +0x38 types, +0x50 fieldOffsetsCount, +0x58 fieldOffsets,
    #   +0x60 typeDefinitionsSizesCount, +0x68 typeDefinitionsSizes
    # We scan .data section (which in this memory image is a contiguous RVA range)
    # for a structure where:
    #   fieldOffsetsCount(0x50) == typeDefinitionsSizesCount(0x60)
    #   typesCount(0x30) in sane range
    #   types/fieldOffsets/typeDefinitionsSizes point into image
    # Since file offset == RVA in this memory image, we scan all file offsets.

    # restrict scan to plausible .data region (after .text/il2cpp code): scan whole file
    # stride 8 for speed; structure must have all QWORD fields readable
    print('\n=== scanning for Il2CppMetadataRegistration (v24 layout) ===')
    found = []
    struct_end = 0x70
    for off in range(0, len(data) - struct_end, 8):
        # +0x30 typesCount
        tc = read_q(data, off + 0x30)
        if tc is None or not (1000 <= tc <= 4000000):
            continue
        types_ptr = read_q(data, off + 0x38)
        if types_ptr is None or not (GA_BASE <= types_ptr < GA_BASE + len(data)):
            continue
        # +0x50 fieldOffsetsCount, +0x60 typeDefSizesCount must match
        foc = read_q(data, off + 0x50)
        tsc = read_q(data, off + 0x60)
        if foc is None or tsc is None or foc != tsc or foc == 0:
            continue
        fo_ptr = read_q(data, off + 0x58)
        ts_ptr = read_q(data, off + 0x68)
        if fo_ptr is None or ts_ptr is None:
            continue
        if not (GA_BASE <= fo_ptr < GA_BASE + len(data)):
            continue
        if not (GA_BASE <= ts_ptr < GA_BASE + len(data)):
            continue
        # types array: first 3 entries must be pointers into image (Il2CppType.data)
        ok = True
        trva = types_ptr - GA_BASE
        for j in range(3):
            entry = trva + j * 0x10
            d = read_q(data, entry)
            if d is None or not (GA_BASE <= d < GA_BASE + len(data)):
                ok = False
                break
        if not ok:
            continue
        found.append(off)
        print(f'  candidate metadataRegistration @ RVA 0x{off:X} typesCount={tc} fieldOffsetsCount={foc}')
        if len(found) >= 8:
            break

    if not found:
        print('  !! no metadataRegistration found')
        return

    reg = found[0]
    tc = read_q(data, reg + 0x30)
    types_ptr = read_q(data, reg + 0x38)
    fo_ptr = read_q(data, reg + 0x58)
    print(f'\n=== using reg @ 0x{reg:X} typesCount={tc} types=0x{types_ptr:X} fieldOffsets=0x{fo_ptr:X} ===')

    # Walk types array: each Il2CppType is 0x10 bytes: [0]=data/klass ptr, [8]=bits
    # typeEnum = (bits >> 16) & 0xFF ; CLASS=0x12 VALUETYPE=0x11
    trva = types_ptr - GA_BASE
    print('\n=== sample first 15 Il2CppType entries ===')
    classes = []
    for j in range(min(200000, tc)):
        entry = trva + j * 0x10
        if entry + 0x10 > len(data):
            break
        d = read_q(data, entry)
        bits = read_q(data, entry + 8)
        type_enum = (bits >> 16) & 0xFF if bits is not None else 0
        if d is None or not (GA_BASE <= d < GA_BASE + len(data)):
            continue
        # d is Il2CppClass*: klass.name @ +0x10 (profile), namespace @ +0x18
        klass_rva = d - GA_BASE
        name_ptr = read_q(data, klass_rva + 0x10)
        ns_ptr = read_q(data, klass_rva + 0x18)
        if name_ptr is None or not (GA_BASE <= name_ptr < GA_BASE + len(data)):
            continue
        name = read_cstr(data, name_ptr - GA_BASE)
        ns = read_cstr(data, ns_ptr - GA_BASE) if ns_ptr and GA_BASE <= ns_ptr < GA_BASE + len(data) else ''
        if name and j < 15:
            print(f'  [{j}] typeEnum=0x{type_enum:X} klass=0x{d:X} name="{ns}.{name}"')
        if name:
            classes.append((j, d, ns or '', name))
        if j < 200000 and len(classes) >= 200000:
            break

    print(f'\n=== resolved {len(classes)} classes from types array ===')
    print('sample first 30:')
    for c in classes[:30]:
        print(f'  [{c[0]}] 0x{c[1]:X} {c[2]}.{c[3]}')

    # verify a game class exists
    print('\n=== search for game classes (CharacterManager etc) ===')
    for target in ['CharacterManager', 'GameEntity', 'ActorModel', 'BuffManager', 'PlayerView']:
        hits = [c for c in classes if target.lower() in c[3].lower()]
        print(f'  {target}: {len(hits)} hits')
        for h in hits[:3]:
            print(f'    [{h[0]}] 0x{h[1]:X} {h[2]}.{h[3]}')

if __name__ == '__main__':
    main()
