#pragma once
#include "Note.h"

class ChangeTempo : public Note {
private:
	double m_tempo;
public:
	ChangeTempo(int tick, double tempo);

	~ChangeTempo() = default;

	virtual void draw() const;

	double getTempo();
};
