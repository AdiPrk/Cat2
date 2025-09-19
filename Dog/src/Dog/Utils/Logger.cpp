#include <PCH/pch.h>

#include "Logger.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace Dog {

	std::shared_ptr<spdlog::logger> Logger::sLogger;

	void Logger::Init()
	{
		spdlog::set_pattern("%^[%T] %n: %v%$");

		sLogger = spdlog::stdout_color_mt("DOG");
	}
}
