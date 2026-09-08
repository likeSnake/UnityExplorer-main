#pragma once
#include <iostream>

template<typename... Args>
inline void MyLog(Args&&... args) {
	if (Function::Flag::开发日志)
	{
		(std::cout << ... << args) << std::endl;
	}
}
template<typename... Args>
inline void MyLogTrue(Args&&... args) {
	(std::cout << ... << args) << std::endl;
}