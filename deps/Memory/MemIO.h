#pragma once

namespace MemIO
{
	void Init(void* hVmm);
	// Clear all process/page translation state. This must be called only after
	// in-flight MemIO operations have drained.
	void ResetAll();
	// Idempotent alias used by the DMA owner during disconnect/shutdown.
	void Shutdown();
	bool SetProcessId(unsigned __int32 ProcessId, unsigned __int64 NewDTB);
	void Clear();
	bool Read(unsigned __int64 Address, void* Buffer, unsigned __int32 Size, bool Cache);
	unsigned __int64 Vir2Phy(unsigned __int64 Address);
}
