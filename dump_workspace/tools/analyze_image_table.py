import sys
import struct

def main():
    img = sys.argv[1]
    with open(img, 'rb') as f:
        data = f.read()
    print(f'[info] loaded {len(data)} bytes')

    # We found assembly name strings around 0xA8C8800-0xBFAC1A8.
    # Check structure: look for an array of 0x28-byte entries with name pointers.
    # The string 'mscorlib.dll' @ 0xA8C8800 and 'System.dll' @ 0xA8CA343 are close.
    # Find the region start: scan backwards for a run of pointers that all point into module.
    # First, dump 0x300 bytes around 0xA8C8700 (before mscorlib.dll)
    print('=== region around mscorlib.dll (0xA8C8800) -0x100..+0x200 ===')
    base = 0xA8C8700
    for off in range(0, 0x300, 16):
        c = data[base+off:base+off+16]
        hexs = ' '.join(f'{b:02X}' for b in c)
        asc = ''.join(chr(b) if 0x20 <= b < 0x7f else '.' for b in c)
        print(f'  {base+off:08X}: {hexs:<48} {asc}')

    # Now examine what looks like an image table: at 0xA936048 we saw Assembly-CSharp.dll
    # preceded by pointers 0x7FF8F7106060 etc (runtime VA!). These look like VA pointers.
    # 0xA936008: 0x7FF8F7106060 -> this is an absolute VA (base 0x7FF8EC7D0000 from earlier probe)
    print('\n=== candidate VA base check ===')
    # from probe: GameAssembly base = 0x7FF8EC7D0000 (plain variant)
    ga_base = 0x7FF8EC7D0000
    ptr1 = 0x7FF8F7106060
    print(f'ptr 0x{ptr1:X} - base 0x{ga_base:X} = RVA 0x{ptr1-ga_base:X}')
    # 0xA936008 in file -> which RVA? if file offset == RVA (raw), then VA = ga_base + 0xA936008
    rva_of_region = 0xA936008
    print(f'file 0xA936008 as RVA -> VA = 0x{ga_base + rva_of_region:X}')
    print(f'ptr1 0x{ptr1:X} vs computed region VA 0x{ga_base + rva_of_region:X}')

    # Dump pointers before Assembly-CSharp.dll as QWORDs
    print('\n=== qwords before Assembly-CSharp.dll (0xA935FC0..0xA936048) ===')
    for off in range(0xA935FC0, 0xA936048, 8):
        v = struct.unpack_from('<Q', data, off)[0]
        print(f'  {off:08X}: {v:016X}  (RVA {v-ga_base:X})' if v >= ga_base else f'  {off:08X}: {v:016X}')

if __name__ == '__main__':
    main()
