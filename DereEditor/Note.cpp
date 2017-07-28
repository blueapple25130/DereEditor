#include "Note.h"


Note::Note(int tick) {
	Tick = tick;
}


void Note::setPosition(int x, int y) {
	m_position.x = x;
	m_position.y = y;
}
