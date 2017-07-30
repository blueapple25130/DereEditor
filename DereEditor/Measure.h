#pragma once
#include "Note.h"
#include "Rhythm.h"

class Measure {
private:
	double m_length = 1.0;
	std::vector<std::shared_ptr<Note>> m_notes;
public:

	Measure() = default;
	~Measure() = default;

	double BeginY;
	double EndY;
	double LastLineY;
	std::shared_ptr<Rhythm> Rhythm = nullptr;

	std::vector<std::shared_ptr<Note>>& getNotes();

	double getRealTick();

	double getLength();

	void setLength(double length);
};
