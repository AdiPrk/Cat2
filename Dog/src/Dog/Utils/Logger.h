#pragma once

namespace Dog {
	class Logger
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetLogger() { return sLogger; }

	private:
		static std::shared_ptr<spdlog::logger> sLogger;
	};
}

// Log & Assert macros
#ifdef _DEBUG
#define DOG_ASSERT(x, ...) { if(!(x)) { DOG_CRITICAL("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#define DOG_STATIC_ASSERT(x, ...) static_assert(x, __VA_ARGS__)

#define DOG_TRACE(...)    ::Dog::Logger::GetLogger()->trace(__VA_ARGS__)
#define DOG_INFO(...)     ::Dog::Logger::GetLogger()->info(__VA_ARGS__)
#define DOG_WARN(...)     ::Dog::Logger::GetLogger()->warn(__VA_ARGS__)
#define DOG_ERROR(...)    ::Dog::Logger::GetLogger()->error(__VA_ARGS__)
#define DOG_CRITICAL(...) ::Dog::Logger::GetLogger()->critical(__VA_ARGS__)
#else
#define DOG_ASSERT(...)
#define DOG_STATIC_ASSERT(...)
#define DOG_TRACE(...)
#define DOG_INFO(...)
#define DOG_WARN(...)
// Keep ERROR and CRITICAL for release builds as well
#define DOG_ERROR(...)    ::Dog::Logger::GetLogger()->error(__VA_ARGS__)
#define DOG_CRITICAL(...) ::Dog::Logger::GetLogger()->critical(__VA_ARGS__)
#endif