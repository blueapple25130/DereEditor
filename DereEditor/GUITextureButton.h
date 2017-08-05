#pragma once
#include <Siv3D.hpp>

class GUITextureButton : public IWidget
{
private:

	double m_pressedPressure = 0.0;

	bool m_pressed = false;

	bool m_mouseOver = false;

	bool m_pushed = false;

	bool m_enabled = true;

	Size ItemSize() const;

	Rect MarginBox(const Point& offset) const;

	Rect PaddingBox(const Point& offset) const;

	Rect ItemBox(const Point& offset) const;

public:
	
	TextureRegion m_textureRegion;

	static const String& Name()
	{
		static const String name = L"TextureButton";
		return name;
	}

	static std::shared_ptr<GUITextureButton> Create(const TextureRegion& texture, const WidgetStyle& style = WidgetStyle())
	{
		return std::make_shared<GUITextureButton>(texture, true, style);
	}

	static std::shared_ptr<GUITextureButton> Create(const TextureRegion& texture, bool enabled, const WidgetStyle& style = WidgetStyle())
	{
		return std::make_shared<GUITextureButton>(texture, enabled, style);
	}

	GUITextureButton();

	GUITextureButton(const TextureRegion& texture, bool enabled, const WidgetStyle& style);

	const String& getWidgetName() const override;

	Size getSize() const override;

	bool forceNewLineBefore() const override;

	bool forceNewLineAfter() const override;

	void update(const WidgetState& state) override;

	void draw(const WidgetState& state) const override;

	bool& getEnabled();

	Property_Get(bool, mouseOver) const
	{
		return m_mouseOver;
	}

	Property_Get(bool, pressed) const
	{
		return m_pressed;
	}

	Property_Get(bool, pushed) const
	{
		return m_pushed;
	}
};

class GUITextureButtonWrapper
{
public:

	using WidgetType = GUITextureButton;

private:

	friend class GUI;

	std::shared_ptr<WidgetType> m_widget;

	GUITextureButtonWrapper(const std::shared_ptr<WidgetType>& widget);

	GUIButtonWrapper& operator = (const GUIButtonWrapper&) = delete;

	static const String& WidgetTypeName()
	{
		return WidgetType::Name();
	}

public:

	bool& enabled;

	WidgetStyle& style;

	Property_Get(bool, mouseOver) const
	{
		return m_widget->mouseOver;
	}

	Property_Get(bool, pressed) const
	{
		return m_widget->pressed;
	}

	Property_Get(bool, pushed) const
	{
		return m_widget->pushed;
	}

	void setTexture(const TextureRegion& _texture);

};