import sys
import struct

GA_BASE = 0x7FF8EC7D0000  # plain variant base

def read_q(data, rva):
    if rva + 8 > len(data):
        return None
    return struct.unpack_from('<Q', data, rva)[0]

def read_cstr(data, rva, maxlen=256):
    if rva >= len(data):
        return None
    end = data.find(b'\x00', rva, rva+maxlen)
    if end < 0:
        end = rva+maxlen
    try:
        s = data[rva:end].decode('utf-8', 'replace')
        return s if s and all(0x20 <= ord(c) < 0x7f for c in s) else None
    except Exception:
        return None

def klass_name(data, klass_rva):
    """read klass->name (name ptr @ +0x10 per profile v24) and namespace @+0x18"""
    name_ptr = read_q(data, klass_rva + 0x10)
    if name_ptr is None or not (GA_BASE <= name_ptr < GA_BASE + len(data)):
        return None
    name = read_cstr(data, name_ptr - GA_BASE)
    if not name:
        return None
    ns_ptr = read_q(data, klass_rva + 0x18)
    ns = ''
    if ns_ptr and GA_BASE <= ns_ptr < GA_BASE + len(data):
        ns = read_cstr(data, ns_ptr - GA_BASE) or ''
    return (ns + '.' + name) if ns else name

def main():
    img = sys.argv[1]
    with open(img, 'rb') as f:
        data = f.read()
    print(f'[info] loaded {len(data)} bytes')

    # 1. find anchor names
    anchors = ['System.Object', 'System.String', 'UnityEngine.Object', 'UnityEngine.Transform']
    print('\n=== step1: find anchor klass pointers ===')
    anchor_rvas = {}
    for an in anchors:
        needle = an.encode('ascii')
        i = data.find(needle)
        if i < 0:
            print(f'  {an}: string not found')
            continue
        # string at rva i; now find who points to it (klass.name_ptr == GA_BASE+i)
        target_va = GA_BASE + i
        klass_hits = []
        # search backward: scan data region for qword == target_va
        # to be fast, scan only first 200MB at 8 stride
        step = 0
        for off in range(0, len(data) - 8, 8):
            if read_q(data, off) == target_va:
                klass_hits.append(off)
                if len(klass_hits) >= 4:
                    break
        print(f'  {an}: string@0x{i:X}, ptr refs found at: {["0x%X" % h for h in klass_hits]}')
        anchor_rvas[an] = (i, klass_hits)

    # 2. verify: klass_hits[0] should be the klass structure; its +0x10 points to name string
    print('\n=== step2: verify klass structure for System.Object ===')
    if 'System.Object' in anchor_rvas:
        i, hits = anchor_rvas['System.Object']
        for h in hits[:3]:
            nm = klass_name(data, h)
            print(f'  klass candidate @ RVA 0x{h:X}: name="{nm}"')

    # 3. scan for TypeInfo table: dense array of Il2CppClass* where entries point
    #    into image and klass names include anchors. Table = g_ctx.typeInfoTable.
    #    We find it by: scanning .data for an 8-aligned pointer that points to a klass
    #    whose name == System.Object AND System.String AND UnityEngine.Object at
    #    consecutive-ish indices... Simpler: find ALL pointers to the 4 anchor klass structs.
    print('\n=== step3: locate TypeInfo table via anchor klass pointers ===')
    # first, get the actual klass rva of System.Object (from step2 verified)
    anchor_klass = {}
    for an in anchors:
        if an not in anchor_rvas:
            continue
        i, hits = anchor_rvas[an]
        for h in hits:
            if klass_name(data, h) == an:
                anchor_klass[an] = h
                break
    print('  anchor klass RVAs:', {k: hex(v) for k, v in anchor_klass.items()})
    if not anchor_klass:
        print('  !! could not verify anchor klass structures')
        return

    # TypeInfo table = array of qwords; for each anchor, find all slots pointing to its klass
    # then intersect: a candidate table start T where T[idx0]=klassObj, T[idx1]=klassString...
    # Without knowing indices, find tables by: scan for qword == anchor_klass_va (System.Object klass)
    # and check neighbors also point to valid klass structures.
    target_va = GA_BASE + anchor_klass['System.Object']
    print(f'  System.Object klass VA = 0x{target_va:X}')
    candidates = []
    for off in range(0, len(data) - 8, 8):
        if read_q(data, off) == target_va:
            # check a few neighbors are valid klass ptrs
            n_ok = 0
            for j in range(-3, 4):
                if j == 0:
                    continue
                p = read_q(data, off + j*8)
                if p and GA_BASE <= p < GA_BASE + len(data):
                    nm = klass_name(data, p - GA_BASE)
                    if nm:
                        n_ok += 1
            if n_ok >= 3:
                candidates.append((off, n_ok))
    print(f'  candidate table slots (pointer to System.Object klass + valid neighbors): {len(candidates)}')
    for off, n_ok in candidates[:10]:
        # dump neighbor names
        names = []
        for j in range(-2, 3):
            p = read_q(data, off + j*8)
            nm = klass_name(data, p - GA_BASE) if p and GA_BASE <= p < GA_BASE + len(data) else '?'
            names.append(nm[:40])
        print(f'    slot RVA 0x{off:X} (neighbors ok={n_ok}): {names}')

if __name__ == '__main__':
    main()
