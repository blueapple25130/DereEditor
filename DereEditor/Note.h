#pragma once
#include <Siv3D.hpp>

enum class NoteType {
	Tap,
	Long,
	Slide,
	LFlick,
	RFlick,
};

class Note {
protected:
	Point m_position;
public:
	double Time;
	int Tick;
	int Lane;

	Note(int tick, int lane);

	virtual void draw() const = 0;

	void setPosition(int x, int y);
};
