#pragma once
#include "Note.h"

class Measure {
private:
	double m_length = 1.0;
	int m_numer = 4;
	int m_denom = 4;
	std::vector<std::shared_ptr<Note>> m_notes;
public:

	Measure() = default;
	~Measure() = default;

	double BeginY;
	double EndY;
	double LastLineY;

	std::vector<std::shared_ptr<Note>>& getNotes();

	double getRealTick();

	double getLength();

	void setLength(double length);

	void setLength(int numer, int denom);
};
