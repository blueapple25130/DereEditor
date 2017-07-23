#include "Measure.h"

std::vector<std::shared_ptr<Note>>& Measure::getNotes() {
	return m_notes;
}

double Measure::getRealTick() {
	return m_length * 768;
}

double Measure::getLength() {
	return m_length;
}

void Measure::setLength(double length) {
	m_length = length;
}

void Measure::setLength(int numer, int denom) {
	m_numer = numer;
	m_denom = denom;
	m_length = static_cast<double>(m_numer) / m_denom;
}