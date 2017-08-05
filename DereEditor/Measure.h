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

	int getLengthTick();

	double getLength();

	void setLength(double length);

	/*
	DelesteŒ`®‚ğ“Ç‚İ‚ñ‚¾ê‡•K‚¸ŒÄ‚Ô
	*/
	void culcTick();

	static void updateMeasureState(std::vector<std::shared_ptr<Measure>> &measures) {
		double currentLength = 1.0;
		for (auto& measure : measures) {
			if (measure->Rhythm != nullptr)
				currentLength = measure->Rhythm->getLength();
			measure->setLength(currentLength);
		}
	}
};
