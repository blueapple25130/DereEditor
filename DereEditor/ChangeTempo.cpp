#include "ChangeTempo.h"


ChangeTempo::ChangeTempo(int tick, double tempo) :
	Note(tick)
{
	m_tempo = tempo;
}


void ChangeTempo::draw() const {
	FontAsset(L"editor")(m_tempo).drawAt(m_position,Palette::Aqua);
}

double ChangeTempo::getTempo() {
	return m_tempo;
}

double ChangeTempo::getY() {
	return m_position.y;
}
