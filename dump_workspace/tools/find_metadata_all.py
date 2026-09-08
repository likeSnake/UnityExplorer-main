import sys
import struct

def main():
    img = sys.argv[1]
    with open(img, 'rb') as f:
        data = f.read()
    print(f'[info] loaded {len(data)} bytes')

    # 1. find ALL FAB11BAF occurrences
    print('\n=== ALL FAB11BAF occurrences ===')
    pat = bytes([0xAF, 0x1B, 0xB1, 0xFA])
    start = 0
    hits = []
    while True:
        i = data.find(pat, start)
        if i < 0:
            break
        hits.append(i)
        start = i + 1
    print(f'total hits: {len(hits)}')
    for h in hits[:50]:
        # check what follows: next 4 bytes as version
        ver = struct.unpack_from('<I', data, h + 4)[0] if h + 8 <= len(data) else 0
        print(f'  0x{h:X}  next_dword(version?)={ver} (0x{ver:X})  section guess: 0x{h//0x1000:X}000')

    # 2. look around Assembly-CSharp.dll string - is it in a metadata string table?
    print('\n=== context around Assembly-CSharp.dll (0xA936048) ===')
    off = 0xA936048
    # back up 0x40
    s = max(0, off - 0x40)
    chunk = data[s:s+0x100]
    for i in range(0, len(chunk), 16):
        c = chunk[i:i+16]
        hexs = ' '.join(f'{b:02X}' for b in c)
        asc = ''.join(chr(b) if 0x20 <= b < 0x7f else '.' for b in c)
        print(f'  {s+i:08X}: {hexs:<48} {asc}')

    # 3. Are there other assembly name strings near it? (string table signature)
    print('\n=== search other known assembly names ===')
    for name in ['Assembly-CSharp-firstpass.dll', 'mscorlib.dll', 'System.dll', 'UnityEngine.UI.dll', 'Assembly-CSharp']:
        b = name.encode('ascii')
        i = data.find(b)
        print(f'  {name}: {"@ 0x%X" % i if i >= 0 else "not found"}')

if __name__ == '__main__':
    main()
