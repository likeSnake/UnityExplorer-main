#pragma once
#include <stddef.h>
#if defined(HERO_CATALOG_BUILD)
#define HERO_CATALOG_API __declspec(dllexport)
#else
#define HERO_CATALOG_API __declspec(dllimport)
#endif

// File-based API. Does not attach to a process. All paths are UTF-16.
// Result receives the output directory on success, or an error on failure.
// 0: success, 1: invalid argument, 2: input/output failure, 3: result buffer too small.
// Callers must provide at least 32768 wchar_t elements before any work starts.
extern "C" HERO_CATALOG_API unsigned __cdecl HeroCatalogVersion() noexcept;
extern "C" HERO_CATALOG_API int __cdecl ExportHeroCatalogW(
    const wchar_t* configDirectory, const wchar_t* outputRoot,
    wchar_t* result, size_t resultCapacity) noexcept;
