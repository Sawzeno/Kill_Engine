#include "Application.h"
#include"Kei/Events/KeyEvent.h"
#include"Log.h"
#include"Keycodes.h"

namespace Kei {

	Application::Application() {

	}

	Application::~Application() {

	}
	void Application::Run() {
		
		Log log;
		log.Init();
		KeyPressedEvent e(Key::A, true);
		//KEI_TRACE("{}", e.ToString());
		KEI_TRACE(e);
		while (true);
	}

}