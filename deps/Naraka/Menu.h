#pragma once
#include <fstream>
#include <sstream>
#include <string>
#include "nav_elements.h"
#include "etc_elements.h"
#include "Data.h"
#include"ESP.h"

void DrawRun();

void 读取模拟振配置();

void 保存模拟振配置();

void 保存活化配置();

void 读取活化配置();

void 保存其他参数();

void 读取其他配置();

void 读取Kmbox配置();
