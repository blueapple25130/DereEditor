#include "Rhythm.h"


Rhythm::Rhythm(int numer, int denom)
	:
	m_numer(numer),
	m_denom(denom)
{

}

double Rhythm::getLength() {
	return static_cast<double>(m_numer) / m_denom;
}

String Rhythm::toString() {
	return Format(m_numer, L"/", m_denom);
}