#pragma once
#define _WINSOCKAPI_
#include <iostream>
#include <Windows.h>
#include <vector>
#include <thread>
#include <array>
#include <Shlwapi.h>
#include <regex>
#include <string>
#include <iomanip>
#include <mutex>  
#include <condition_variable>
#include <chrono>
#include <algorithm>
#include "xorstr.h"
#include "Vector.h"
#include "json.hpp"
#include "kmboxNet.h"
#include "Memory/Memory.h"
#include "Offset.h"
#include "HidTable.h"
#include "OS-ImGui/OS-ImGui.h"
#pragma comment(lib, "libs/FTD3XX.lib")
#pragma comment(lib, "libs/vmm.lib")
#pragma comment(lib, "libs/leechcore.lib")
#pragma comment(lib, "libs/VMProtectSDK64.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib,"ws2_32.lib")
using namespace std;

