#pragma once

#include <BWAPI.h>

#include <UnitGroup.h>

#include "SquadManager.h"
#include "Squad.h"
#include "UnitCount.h"

class VultureSquad : public Squad
{
protected:
	int potentialAttack;
	int potentialRun;
	int potentialMine;

	void update(BWAPI::Unit *unit);
	void draw(BWAPI::Unit *unit);

	bool isMineNearby(BWAPI::Unit *unit);
	void layMine(BWAPI::Unit *unit);
public:
	VultureSquad(SquadManager *manager, std::map<BWAPI::UnitType, UnitCount*> *composition);
	void update();
	void moveTo(BWAPI::Position position);
	void attackTo(BWAPI::Position position);
};