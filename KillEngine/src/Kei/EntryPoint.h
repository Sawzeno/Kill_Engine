#pragma once

#include<Kei.h>


#ifdef KEI_PLATFORM_WINDOWS

extern Kei::Application* Kei::CreateApplication();

int main(int argc, char** argv)
{
	auto app = Kei::CreateApplication();

	app->Run();
	delete app;
}

#endif