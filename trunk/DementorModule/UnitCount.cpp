#include "UnitCount.h"

UnitCount::UnitCount(int max, int min)
:
	max(max),
	min(min),
	actual(0)
{
}

int UnitCount::getMax()
{
	return max;
}

int UnitCount::getMin()
{
	return min;
}

int UnitCount::getActual()
{
	return actual;
}

void UnitCount::setActual(int actual)
{
	this->actual = actual;
}

bool UnitCount::hasEnough()
{
	return (actual >= min);
}

bool UnitCount::hasMax()
{
	return (actual == max);
}

void UnitCount::addOne()
{
	actual++;
}

void UnitCount::substractOne()
{
	actual--;
}