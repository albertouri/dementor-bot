#include "Vector.h"
#include <cmath>

namespace dementor {
	Vector Vector::fromPositions(const BWAPI::Position &from, const BWAPI::Position &to) {
		return Vector(to.x() - from.x(), to.y() - from.y());
	}

	Vector Vector::fromVectors(const Vector &from, const Vector &to) {
		return Vector(to.x - from.x, to.y - from.y);
	}

	Vector::Vector(int x, int y)
	{
		this->x = (double)x;
		this->y = (double)y;
	}

	Vector::Vector(double x, double y)
	{
		this->x = x;
		this->y = y;
	}

	Vector::~Vector(void)
	{
	}

	int Vector::getX() const {
		return this->x;
	}

	int Vector::getY() const {
		return this->y;
	}

	double Vector::getLength() const {
		return sqrt((double)(this->x*this->x + this->y*this->y));
	}

	Vector Vector::operator+(const Vector &vec) const {
		return Vector(vec.x + this->x, vec.y + this->y);
	}

	Vector Vector::operator-(const Vector &vec) const {
		return Vector(this->x - vec.x, this->y - vec.y);
	}

	Vector Vector::operator*(double coeficient) const {
		return Vector(this->x * coeficient, this->y * coeficient);
	}

	Vector Vector::normalize(void) const {
		double l = getLength();
		if(l == 0)
			return Vector(0, 0);
		return Vector((double)this->x * (1/l), (double)this->y * (1/l));
	}
}

BWAPI::Position operator+(const BWAPI::Position &position, const dementor::Vector &vector) {
	return BWAPI::Position(position.x()+vector.getX(), position.y()+vector.getY());
}