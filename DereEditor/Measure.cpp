#include "Measure.h"
#include "PlayableNote.h"

std::vector<std::shared_ptr<Note>>& Measure::getNotes() {
	return m_notes;
}

int Measure::getLengthTick() {
	return static_cast<int>(m_length * 768);
}

double Measure::getLength() {
	return m_length;
}

void Measure::setLength(double length) {
	m_length = length;
}

void Measure::culcTick() {
	for (auto& note : m_notes) {
		if (typeid(*note) == typeid(PlayableNote)) {
			note->Tick = static_cast<int>(std::dynamic_pointer_cast<PlayableNote>(note)->getDelesteCharPos() * m_length * 768);
		}
	}
}