#include "GUITextureButton.h"


Size GUITextureButton::ItemSize() const
{
	return m_textureRegion.size.asPoint();
}

Rect GUITextureButton::MarginBox(const Point& offset) const
{
	const Size item = ItemSize();

	Rect rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = m_style.margin.left + Max(m_style.padding.left, 0) + item.x + Max(m_style.padding.right, 0) + m_style.margin.right;
	rect.h = m_style.margin.top + Max(m_style.padding.top, 0) + item.y + Max(m_style.padding.bottom, 0) + m_style.margin.bottom;

	return rect.moveBy(offset);
}

Rect GUITextureButton::PaddingBox(const Point& offset) const
{
	const Size item = ItemSize();

	Rect rect;
	rect.x = m_style.margin.left;
	rect.y = m_style.margin.top;
	rect.w = Max(m_style.padding.left, 0) + item.x + Max(m_style.padding.right, 0);
	rect.h = Max(m_style.padding.top, 0) + item.y + Max(m_style.padding.bottom, 0);

	return rect.moveBy(offset);
}

Rect GUITextureButton::ItemBox(const Point& offset) const
{
	const Point item = ItemSize();

	Rect rect;
	rect.x = m_style.margin.left + Max(m_style.padding.left, 0);
	rect.y = m_style.margin.top + Max(m_style.padding.top, 0);
	rect.w = item.x;
	rect.h = item.y;

	return rect.moveBy(offset);
}


GUITextureButton::GUITextureButton() {}

GUITextureButton::GUITextureButton(const TextureRegion& texture, bool enabled, const WidgetStyle& style)
	: IWidget(style)
	, m_textureRegion(texture)
	, m_enabled(enabled) {}

const String& GUITextureButton::getWidgetName() const
{
	return Name();
}

Size GUITextureButton::getSize() const
{
	return MarginBox(Point::Zero).size;
}

bool GUITextureButton::forceNewLineBefore() const
{
	return false;
}

bool GUITextureButton::forceNewLineAfter() const
{
	return false;
}

void GUITextureButton::update(const WidgetState& state)
{
	const bool active = state.active;

	const Rect itemBox = ItemBox(state.currentPos);

	m_mouseOver = active && m_enabled && itemBox.mouseOver;

	m_pushed = active && m_enabled && m_pressed && itemBox.leftReleased;

	if (m_enabled && itemBox.leftClicked)
	{
		m_pressed = true;
	}
	else if (!active || !m_enabled || !Input::MouseL.pressed)
	{
		m_pressed = false;
	}

	if (!active || !m_enabled)
	{
		m_pressedPressure = 0.0;

		return;
	}

	if (m_pressed && itemBox.leftPressed)
	{
		m_pressedPressure = Min(m_pressedPressure + 0.25, 1.0);
	}
	else if (m_mouseOver)
	{
		m_pressedPressure = Min(m_pressedPressure + 0.04, 0.2);
	}
	else if (m_pressed)
	{
		m_pressedPressure = Max(m_pressedPressure - 0.1, 0.4);
	}
	else
	{
		m_pressedPressure = Max(m_pressedPressure - 0.08, 0.0);
	}
}

void GUITextureButton::draw(const WidgetState& state) const
{
	const Rect itemBox = ItemBox(state.currentPos);

	if (m_enabled)
	{
		if (!m_pressed)
		{
			itemBox.stretched(0, -1, 0, -1).drawShadow({ 0, 3.2 }, 7, -2);
		}

		m_textureRegion.draw(itemBox.pos, Color(m_pressed ? 240 : 255));

		if (m_mouseOver)
		{
			itemBox.draw(Color(255, 40));
		}
	}
	else
	{
		m_textureRegion.draw(itemBox.pos).draw(Color(255, 127));
	}

	if (state.showBoxModel)
	{
		MarginBox(state.currentPos).drawFrame(1, 0, Color(255, 0, 0, 255));

		PaddingBox(state.currentPos).drawFrame(1, 0, Color(0, 127, 0, 255));

		ItemBox(state.currentPos).drawFrame(1, 0, Color(0, 0, 127, 255));
	}
}

bool& GUITextureButton::getEnabled()
{
	return m_enabled;
}

GUITextureButtonWrapper::GUITextureButtonWrapper(const std::shared_ptr<WidgetType>& widget)
	: m_widget(widget ? widget : std::make_shared<WidgetType>())
	, enabled(m_widget->getEnabled())
	, style(m_widget->m_style) {}

void GUITextureButtonWrapper::setTexture(const TextureRegion& texture) {
	m_widget->m_textureRegion = texture;
}