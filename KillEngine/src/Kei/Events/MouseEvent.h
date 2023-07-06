#pragma once

#include"Event.h"
#include"Kei/MouseCodes.h"

#include<sstream>

namespace Kei {

	class KEI_API MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(const float x_input, const float y_input) :X(x_input), Y(y_input) {}

		float GetX() const { return X; }
		float GetY() const { return Y; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseMovedEvent: " << X << ", " << Y;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseMoved)
			EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
	private:
		float X, Y;
	};

	class KEI_API MouseScrolledEvent :public Event
	{
	public:
		MouseScrolledEvent(const float x_input, const float y_input)
			: X(x_input), Y(y_input) {}

		float GetX() const { return X; }
		float GetY() const { return Y; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseScrolledEvent: " << GetX() << ", " << GetY();
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseScrolled)
			EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
	private:
		float X, Y;
	};

	class KEI_API MouseButtonEvent :public Event
	{
	public:
		MouseCode GetMouseButton() const { return m_Button; }

		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput | EventCategoryMouseButton)

	protected:
		MouseButtonEvent(const MouseCode button) :m_Button(button) {}

		MouseCode m_Button;
	};

	class KEI_API MouseButtonPressedEvent :public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(const MouseCode button)
			: MouseButtonEvent(button) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonPressedEvent: " << m_Button;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonPressed)
	};

	class KEI_API MouseButtonReleasedEvent :public MouseButtonEvent
	{
	public:
		MouseButtonReleasedEvent(const MouseCode button)
			: MouseButtonEvent(button) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonReleasedEvent: " << m_Button;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonReleased)
	};

}