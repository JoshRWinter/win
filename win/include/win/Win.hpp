#pragma once

// redifined platform macros for conditional compilation
#if defined __linux__
#define WINPLAT_LINUX
#define WIN_USE_OPENGL
#include <iostream>

#elif defined _WIN32
#define WINPLAT_WINDOWS
#define WIN_USE_OPENGL
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#else
#error "unsupported platform"
#endif

#include <string>

#define WIN_NO_COPY_MOVE(classname) \
	classname(const classname&) = delete; \
	classname(classname&&) = delete; \
	void operator=(const classname&) = delete; \
	void operator=(classname&&) = delete

#define WIN_NO_COPY(classname) \
	classname(const classname&) = delete; \
	void operator=(const classname&) = delete

namespace win
{

[[noreturn]] inline void bug(const std::string &msg)
{
#ifdef WINPLAT_WINDOWS
	MessageBox(NULL, ("IMPLEMENTATION BUG:\n" + msg).c_str(), "BUG", MB_ICONEXCLAMATION);
#else
	std::cerr << "IMPLEMENTATION BUG:\n=================\n" << msg << "\n=================" << std::endl;
#endif
	std::abort();
}

}

#ifndef NDEBUG

#include <chrono>

#define WIN_TOKEN_APPEND2(a, b) a##b
#define WIN_TOKEN_APPEND(a, b) WIN_TOKEN_APPEND2(a, b)

#define WIN_BENCHMARK(name, x, thresholdms) \
	const auto WIN_TOKEN_APPEND(win_benchmark_start, __LINE__) = std::chrono::high_resolution_clock::now(); \
	x; \
	const float WIN_TOKEN_APPEND(win_benchmark_elapsed_ms, __LINE__) = std::chrono::duration<float, std::milli>(std::chrono::high_resolution_clock::now() - WIN_TOKEN_APPEND(win_benchmark_start, __LINE__)).count(); \
	if (WIN_TOKEN_APPEND(win_benchmark_elapsed_ms, __LINE__) >= thresholdms) fprintf(stderr, "%s: %.4fms\n", name, WIN_TOKEN_APPEND(win_benchmark_elapsed_ms, __LINE__))

#define WIN_INTERVAL(name, thresholdms) \
	static auto WIN_TOKEN_APPEND(win_interval_last_time, __LINE__) = std::chrono::high_resolution_clock::now(); \
	const auto WIN_TOKEN_APPEND(win_interval_current_time, __LINE__) = std::chrono::high_resolution_clock::now(); \
	const auto WIN_TOKEN_APPEND(win_interval_elapsed_ms, __LINE__) = std::chrono::duration<float, std::milli>(WIN_TOKEN_APPEND(win_interval_current_time, __LINE__) - WIN_TOKEN_APPEND(win_interval_last_time, __LINE__)).count(); \
	if (WIN_TOKEN_APPEND(win_interval_elapsed_ms, __LINE__) >= thresholdms) fprintf(stderr, "%s: %.4fms\n", name, WIN_TOKEN_APPEND(win_interval_elapsed_ms, __LINE__)); \
	WIN_TOKEN_APPEND(win_interval_last_time, __LINE__) = WIN_TOKEN_APPEND(win_interval_current_time, __LINE__)
#else
#define WIN_BENCHMARK(name, x, thresholdms) x
#define WIN_INTERVAL(name, thresholdms)
#endif
