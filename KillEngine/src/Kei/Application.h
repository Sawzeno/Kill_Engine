#pragma once

#include"Core.h"

namespace Kei {

	class KEI_API Application
	{
	public:
		Application();

		void Run();

		virtual ~Application();

	};

	// TO BE DEFINED IN CLIENT
	Application* CreateApplication();
}

