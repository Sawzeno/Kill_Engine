#pragma once

#include"Core.h"
#include"spdlog/spdlog.h"
#include"spdlog/fmt/ostr.h"


namespace Kei {

	class KEI_API Log
	{

	public:

		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }

		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }


	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};

}

//CORE LOG MACROS
#define KEI_CORE_TRACE(...)		::Kei::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define KEI_CORE_INFO(...)		::Kei::Log::GetCoreLogger()->info(__VA_ARGS__)
#define KEI_CORE_WARN(...)		::Kei::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define KEI_CORE_ERROR(...)		::Kei::Log::GetCoreLogger()->error(__VA_ARGS__)
#define KEI_CORE_FATAL(...)		::Kei::Log::GetCoreLogger()->fatal(__VA_ARGS__)
										  
//CLIENT LOG MACROS						  
#define KEI_TRACE(...)			::Kei::Log::GetClientLogger()->trace(__VA_ARGS__)
#define KEI_INFO(...)			::Kei::Log::GetClientLogger()->info(__VA_ARGS__)
#define KEI_WARN(...)			::Kei::Log::GetClientLogger()->warn(__VA_ARGS__)
#define KEI_ERROR(...)			::Kei::Log::GetClientLogger()->error(__VA_ARGS__)
#define KEI_FATAL(...)			::Kei::Log::GetClientLogger()->fatal(__VA_ARGS__)