#pragma once

#include <BWAPI.h>

#include <UnitGroup.h>

#include "SquadManager.h"
#include "UnitCount.h"

class SquadManager;

class Squad
{
protected:
	const static int drawSquadRadius;

	bool debug;

	int id;

	BWAPI::Position target;
	BWAPI::Order order;
	BWAPI::Position center;

	SquadManager *manager;

	std::map<BWAPI::UnitType, UnitCount*> *composition;
	UnitGroup units;

	virtual void calculateCenter();

	bool enoughUnits;
	bool optimalUnits;
	void draw();
public:
	Squad(SquadManager *manager, std::map<BWAPI::UnitType, UnitCount*> *composition);

	void requestUnits();
	virtual void update();

	int getId();

	void removeUnit(BWAPI::Unit *unit);
	bool hasEnoughUnits();
	bool hasOptimalUnits();

	UnitGroup getUnits();

	virtual void moveTo(BWAPI::Position target);
	virtual void attackTo(BWAPI::Position target);

	void setDebug(bool debug);
};