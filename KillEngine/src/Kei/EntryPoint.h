#pragma once


#ifdef KEI_PLATFORM_WINDOWS

extern Kei::Application* Kei::CreateApplication();

int main(int argc, char** argv)
{
	Kei::Log::Init();
	KEI_CORE_ERROR("Kill Switch Disabled , now entering extinction mode");

	int threat = 5;
	KEI_INFO("Threat level{0}", threat);
	auto app = Kei::CreateApplication();
	app->Run();
	delete app;
}

#endif