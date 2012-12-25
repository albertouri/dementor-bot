#pragma once

class UnitCount
{
private:
	int max, min, actual;
public:
	UnitCount(int max, int min);
	int getMax();
	int getMin();
	int getActual();
	void setActual(int count);
	bool hasEnough();
	bool hasMax();
	void addOne();
	void substractOne();
};