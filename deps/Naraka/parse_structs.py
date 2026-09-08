import re
import os

files = ['Offset.h', 'Offset_updated.h']
for f in files:
    if not os.path.exists(f): continue
    print(f"--- {f} ---")
    try:
        content = open(f, 'rb').read().decode('gbk', errors='ignore')
        
        # Search for struct members assignments
        # e.g. uint32_t AllAliveCharList = 0x58;
        matches = re.findall(r'uint32_t\s+(\w+)\s*=\s*(0x[0-9a-fA-F]+);', content)
        for name, val in matches:
            print(f'{name} = {val}')
            
        # Also just grep for "AllAliveCharList" to see context
        if "AllAliveCharList" in content:
             print("Found 'AllAliveCharList' in file.")
             # print surrounding lines
             lines = content.splitlines()
             for i, line in enumerate(lines):
                 if "AllAliveCharList" in line:
                     print(f"Line {i}: {line.strip()}")

    except Exception as e:
        print(f"Error reading {f}: {e}")
