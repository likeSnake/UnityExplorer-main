// Single implementation translation unit for the shared DMA adapter.
// Keeping Mem.cpp/MemIO.cpp out of the public header prevents duplicate
// definitions when a server is built from multiple source files.
#include "MetickAdapter.hpp"

#include "../../deps/Memory/Mem.cpp"
#include "../../deps/Memory/MemIO.cpp"
