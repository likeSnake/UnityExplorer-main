import sys
import time

def search(path, patterns):
    t0 = time.time()
    with open(path, 'rb') as f:
        data = f.read()
    print(f"[info] loaded {len(data)} bytes in {time.time()-t0:.1f}s")
    for name, pat in patterns:
        pat_b = pat.encode('ascii') if isinstance(pat, str) else bytes(pat)
        t1 = time.time()
        idx = data.find(pat_b)
        print(f"  '{name}' ({pat_b.hex()}): {'@ 0x%X' % idx if idx >= 0 else 'NOT FOUND'}  ({time.time()-t1:.2f}s)")
        # 找所有出现（限制数量）
        if idx >= 0:
            all_idx = []
            start = 0
            while True:
                i = data.find(pat_b, start)
                if i < 0:
                    break
                all_idx.append(i)
                start = i + 1
                if len(all_idx) >= 20:
                    break
            print(f"    first few occurrences: {['0x%X' % i for i in all_idx]}")
    print(f"[done] total {time.time()-t0:.1f}s")

if __name__ == '__main__':
    img = sys.argv[1]
    patterns = [
        ('Assembly-CSharp.dll', 'Assembly-CSharp.dll'),
        ('global-metadata.dat', 'global-metadata.dat'),
        ('FAB11BAF', bytes([0xAF, 0x1B, 0xB1, 0xFA])),
        ('il2cpp_gc', 'il2cpp_gc'),
        ('metadataVersion', 'metadataVersion'),
        ('UnityEngine.CoreModule', 'UnityEngine.CoreModule'),
        ('GameAssembly.dll', 'GameAssembly.dll'),
        ('0xFAB11BAF as string', 'FAB11BAF'),
    ]
    search(img, patterns)
