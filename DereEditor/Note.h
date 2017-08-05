#pragma once
#include <Siv3D.hpp>

enum class NoteType {
	LFlick = 1,
	Tap,
	RFlick,
	Long,
	Slide,
};

class Note {
protected:
	Point m_position;
public:
	double Time;
	int Tick;

	Note(int tick);
	Note() = default;
	virtual ~Note() = default;

	virtual void draw() const = 0;

	void setPosition(int x, int y);
};
