import sys
import struct

def dump_hex(data, off, length):
    for i in range(0, length, 16):
        chunk = data[off+i:off+i+16]
        hexs = ' '.join(f'{b:02X}' for b in chunk)
        asc = ''.join(chr(b) if 0x20 <= b < 0x7f else '.' for b in chunk)
        print(f'  {off+i:08X}: {hexs:<48} {asc}')

def main():
    img = sys.argv[1]
    magic_off = int(sys.argv[2], 16) if len(sys.argv) > 2 else None
    with open(img, 'rb') as f:
        data = f.read()
    print(f'[info] loaded {len(data)} bytes')

    # Locate FAB11BAF if not given
    if magic_off is None:
        idx = data.find(bytes([0xAF, 0x1B, 0xB1, 0xFA]))
        if idx < 0:
            print('FAB11BAF not found')
            return
        magic_off = idx
        print(f'[info] FAB11BAF @ 0x{magic_off:X}')

    print(f'\n=== metadata header at 0x{magic_off:X} (first 0x100 bytes) ===')
    dump_hex(data, magic_off, 0x100)

    # Parse il2cpp metadata header fields (v24-ish layout, offsets per Il2CppInspector)
    # https://github.com/Il2CppInspector/Il2CppInspector - GlobalMetadataHeader v24
    # 0x00 magic, 0x04 version, 0x08 stringLiteralOffset/size, 0x10 stringLiteralDataOffset/size
    # 0x18 stringOffset, 0x20 stringSize, 0x28 eventsOffset, 0x30 eventsSize ...
    # we read key ones defensively
    magic, version = struct.unpack_from('<II', data, magic_off)
    print(f'\nmagic=0x{magic:08X} version={version}')
    if magic != 0xFAB11BAF:
        print('!! not FAB11BAF at this offset (maybe false positive), abort')
        return
    # dump first 0x80 dwords as offsets for manual interpretation
    print('\n=== header dwords (LE) ===')
    for i in range(0, 0x90, 8):
        a, b = struct.unpack_from('<QQ', data, magic_off + i)
        print(f'  +0x{i:02X}: {a:016X}  {b:016X}')

if __name__ == '__main__':
    main()
