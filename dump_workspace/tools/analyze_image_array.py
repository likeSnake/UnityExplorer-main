import sys
import struct

def cstr(data, off, maxlen=200):
    end = data.find(b'\x00', off, off+maxlen)
    if end < 0:
        end = off+maxlen
    try:
        return data[off:end].decode('utf-8', 'replace')
    except Exception:
        return '?'

def main():
    img = sys.argv[1]
    ga_base = 0x7FF8EC7D0000  # from probe (plain variant)
    with open(img, 'rb') as f:
        data = f.read()
    print(f'[info] loaded {len(data)} bytes')

    # We believe Il2CppImage array is at file offset ~0xA936000 region.
    # Structure (v24): Il2CppImage { void* nameIndex; Il2CppString* name; int typeStart;
    #                                uint32_t typeCount; Il2CppTypeDefinition* types; ... }
    # Actually from sdk_runner: imagesSize % 0x28 == 0 -> 0x28 byte entries.
    # Entry layout seen at 0xA935FF0:
    #   0xA935FF0: 0x7FF8F7106048 -> VA of name string "Assembly-CSharp.dll" (file 0xA936048)
    #   0xA935FF8: 0x0006A85B       (??? maybe nameIndex or token)
    #   0xA936000: 0x7FF8FA1CFEA0 -> VA (RVA 0xD9FFEA0)
    #   0xA936008: 0x7FF8F7106060 -> VA (RVA 0xA936060)  ??? 
    #   0xA936010: 0x00000071      -> typeStart? 113
    #   0xA936018: 0x7FF8F72B01D0 -> VA (RVA 0xAAE01D0)
    #   0xA936020: 0x00000214      -> typeCount? 532
    # This looks like a 0x28-byte entry starting at 0xA935FF0? Let's test stride from earlier:
    # entry N   @ 0xA935FF0..0xA936017 (0x28 bytes)
    # entry N+1 @ 0xA936018
    # Actually 0xA936018+0x10 = 0xA936028 has 0x7FF8F72B0C70 and +0x18 = 0x3B03?? unclear.

    # Find the start of the image array: scan backwards in steps of 0x28 from 0xA935FF0
    # checking that each entry has a name ptr pointing to a readable string in image.
    print('\n=== scan backward for image array start (stride 0x28) ===')
    start = 0xA935FF0
    while start > 0:
        # entry at start: +0x08 should be a VA pointing into image (>= ga_base, < ga_base+0x19E59000)
        name_va = struct.unpack_from('<Q', data, start + 8)[0]
        rva = name_va - ga_base
        if not (0 <= rva < len(data)):
            break
        name = cstr(data, rva)
        if not name or not name.isprintable():
            break
        print(f'  entry @ file 0x{start:X} (RVA 0x{start:X}): name_va=0x{name_va:X} name="{name}"')
        start -= 0x28
    print(f'\narray start candidate: 0x{start+0x28:X}')

    # Now walk forward to count entries
    print('\n=== walk forward (first 15 + count) ===')
    pos = start + 0x28
    entries = []
    while pos + 0x28 <= len(data):
        name_va = struct.unpack_from('<Q', data, pos + 8)[0]
        rva = name_va - ga_base
        if not (0 <= rva < len(data)):
            break
        name = cstr(data, rva)
        if not name or not name.isprintable() or len(name) > 120:
            break
        type_count = struct.unpack_from('<I', data, pos + 0x20)[0] if pos+0x24 <= len(data) else 0
        entries.append((pos, name, type_count))
        pos += 0x28
        if len(entries) >= 15:
            break
    for e in entries:
        print(f'  {e[0]:08X}: "{e[1]}" typeCount={e[2]}')

    # Now jump to a large stride to find the true count
    print('\n=== estimate array size: keep walking up to 200 entries or until invalid ===')
    pos2 = start + 0x28
    count = 0
    names = []
    while pos2 + 0x28 <= len(data) and count < 200:
        name_va = struct.unpack_from('<Q', data, pos2 + 8)[0]
        rva = name_va - ga_base
        if not (0 <= rva < len(data)):
            break
        name = cstr(data, rva)
        if not name or not name.isprintable() or len(name) > 150:
            break
        names.append(name)
        count += 1
        pos2 += 0x28
    print(f'walked {count} entries')
    for n in names:
        print(f'  "{n}"')

if __name__ == '__main__':
    main()
