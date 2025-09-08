#pragma once

#define _CRT_SECURE_NO_WARNINGS

#define _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING
#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN 

#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>

#include <VkBootstrap.h>

#include "vk_mem_alloc.h"

#include <string>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <thread>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <array>
#include <optional>
#include <unordered_set>
#include <set>
#include <unordered_map>
#include <map>
#include <regex>
#include <future>
#include <typeindex>
#include <random>
#include <filesystem>

// undef some dumb macros
#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif
#ifdef SendMessage
#undef SendMessage
#endif