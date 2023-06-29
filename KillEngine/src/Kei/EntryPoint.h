#pragma once


#ifdef KEI_PLATFORM_WINDOWS

extern Kei::Application* Kei::CreateApplication();

int main(int argc, char** argv)
{
	printf("Rising Blood Lust \n");
	auto app = Kei::CreateApplication();
	app->Run();
	delete app;
}

#endif