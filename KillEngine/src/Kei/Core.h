#pragma once

#ifdef KEI_PLATFORM_WINDOWS
#ifdef KEI_BUILD_DLL
#define KEI_API __declspec(dllexport)
#else
#define KEI_API __declspec(dllimport)
#endif // KEI_BUILD_KEI
#else
#error KILL ENGINE ONLY RUNS ON WINDOWS !
#endif