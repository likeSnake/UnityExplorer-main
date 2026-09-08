// pch.h：这是一个预编译的头文件。
// 下面列出的文件仅编译一次，从而提高了未来构建的构建性能。
// 这也会影响 Intelli Sense 性能，包括代码完成和许多代码浏览功能。
// 但是，如果在构建之间更新了此处列出的任何一个文件，则将全部重新编译。
// 请勿在此处添加您将频繁更新的文件，因为这会抵消性能优势。
#pragma once
#ifndef PCH_H
#define PCH_H

//DMA
#include "../libs/vmmdll.h"

// add headers that you want to pre-compile here

#include <Windows.h>
#include <cstdio>
#include <sstream>
#include <fstream>
#include <filesystem>


#define DEBUG_INFO
#ifdef DEBUG_INFO
#define LOG(fmt, ...) std::printf(fmt, ##__VA_ARGS__)
#define LOGW(fmt, ...) std::wprintf(fmt, ##__VA_ARGS__)
#else
#define LOG
#define LOGW
#endif

#define THROW_EXCEPTION
#ifdef THROW_EXCEPTION
#define THROW(fmt, ...) throw std::runtime_error(fmt, ##__VA_ARGS__)
#endif

#endif //PCH_H
