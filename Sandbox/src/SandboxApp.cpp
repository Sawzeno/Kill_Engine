#include"kei.h"

class Sandbox : public Kei::Application
{
public:
	Sandbox()
	{

	}

	~Sandbox()
	{

	}
};

Kei::Application* Kei::CreateApplication()
{
	return new Sandbox();
}