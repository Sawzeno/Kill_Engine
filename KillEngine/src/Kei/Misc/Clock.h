#pragma once
#include<iostream>
#include<string>
#include<list>


class Subject;

class Observer {
public:
	virtual ~Observer() = default;
	virtual void Update(Subject* TheChangedSubject) = 0;
};

class Subject {
public:
	Subject();
	virtual ~Subject();
	virtual void Attach(Observer* o);
	virtual void Notify();
private:
	std::list<Observer*>* _Observers;
};
Subject::Subject() {
	_Observers = new std::list<Observer*>();
}

Subject::~Subject() {
	delete _Observers;
}

void Subject::Attach(Observer* o) {
	_Observers->push_front(o);
}

void Subject::Notify() {
	for (std::list<Observer*>::iterator it = _Observers->begin(); it != _Observers->end(); it++) {
		(*it)->Update(this);
	}
}

class Widget {
public:
	void Draw() {
		std::cout << "Clock Initialised";
	};
};

class Clock :public Subject {
public:
	virtual ~Clock() = default;
	void Tick() { return Notify(); }
};

class Engine :public Widget, public Observer {
public:

	Engine(Clock*);
	virtual void Update(Subject*);
	virtual void Draw();
	virtual ~Engine() = default;
private:
	Clock* m_Clock;
};

Engine::Engine(Clock* s) {
	m_Clock = s;
	m_Clock->Attach(this);
}

void Engine::Update(Subject* TheChangedSubject) {
	if (TheChangedSubject == m_Clock) {
		Draw();
	}
}

void Engine::Draw() {
	std::cout << "ClockWork";
}






