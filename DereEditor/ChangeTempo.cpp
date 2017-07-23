#include "ChangeTempo.h"


ChangeTempo::ChangeTempo(int tick, double tempo) :
	Note(tick, 5)
{
	m_tempo = tempo;
}


void ChangeTempo::draw() const {
	FontAsset(L"editor")(m_tempo).drawAt(m_position);
}

double ChangeTempo::getTempo() {
	return m_tempo;
}
