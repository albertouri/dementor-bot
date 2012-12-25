#pragma once

#include <BWAPI.h>

namespace dementor {

class Vector
{
private:
	double x, y;
public:
	static Vector fromPositions(const BWAPI::Position &from, const BWAPI::Position &to);
	static Vector fromVectors(const Vector &from, const Vector &to);

	Vector(int x, int y);
	Vector(double x, double y);
	
	~Vector(void);

	double getLength() const;
	Vector operator+(const Vector &vector) const;
	Vector operator-(const Vector &vector) const;
	Vector operator*(double coeficient) const;

	Vector normalize(void) const;

	int getX() const;
	int getY() const;
};

}

BWAPI::Position operator+(const BWAPI::Position &position, const dementor::Vector &vector);