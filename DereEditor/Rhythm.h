#pragma once
#include <Siv3D.hpp>

class Rhythm
{
private:
	int m_numer;
	int m_denom;
public:
	Rhythm(int numer, int denom);
	Rhythm() = default;
	~Rhythm() = default;

	double getLength();
	String toString();
};

