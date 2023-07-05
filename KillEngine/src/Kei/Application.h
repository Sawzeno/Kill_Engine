#pragma once

#include"Core.h"
#include"Events/Event.h"

namespace Kei {

	class KEI_API Application
	{
	private:
	public:

		Application();

		void Run();

		virtual ~Application();

	};

	// TO BE DEFINED IN CLIENT
	Application* CreateApplication();
}

