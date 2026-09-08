import re
import os

files = ['Offset.h', 'Offset_updated.h']
for f in files:
    if not os.path.exists(f): continue
    print(f"--- {f} ---")
    try:
        content = open(f, 'rb').read().decode('gbk', errors='ignore')
        matches = re.findall(r'(?:inline\s+)?uint64_t\s+(\w+)\s*=\s*(0x[0-9a-fA-F]+);', content)
        # Also static const or just names inside namespace
        # Actually header might be: inline uint64_t Name = 0x...;
        
        for name, val in matches:
            print(f'{name} = {val}')
            
        # Try to find struct offsets: uint32_t Name = 0x...;
        matches32 = re.findall(r'(?:inline\s+)?uint32_t\s+(\w+)\s*=\s*(0x[0-9a-fA-F]+);', content)
        for name, val in matches32:
            print(f'{name} = {val}')

    except Exception as e:
        print(f"Error reading {f}: {e}")
