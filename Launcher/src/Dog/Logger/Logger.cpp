#include <PCH/pch.h>

#include "Logger.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace Dog {

	std::shared_ptr<spdlog::logger> Logger::s_Logger;

	void Logger::Init() {
		spdlog::set_pattern("%^[%T] %n: %v%$");

		s_Logger = spdlog::stdout_color_mt("DOG");
	}
}
