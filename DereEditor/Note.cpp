#include "Note.h"


Note::Note(int tick, int lane) {
	Tick = tick;
	Lane = lane;
}


void Note::setPosition(int x, int y) {
	m_position.x = x;
	m_position.y = y;
}
