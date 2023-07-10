#include"Keipch.h"
#include "Application.h"
#include"Kei/Events/KeyEvent.h"
#include"Kei/Events/MouseEvent.h"
#include"Kei/Events/ApplicationEvent.h"
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
		KeyPressedEvent   e(Key::A, true);
		MouseMovedEvent	  f(1.0, 2.0);
		WindowResizeEvent g(100, 100);
		KEI_TRACE(e);
		KEI_TRACE(f);
		KEI_TRACE(g);
		while (true);
	}

}