#pragma once

#include <BWAPI.h>

#include <UnitGroup.h>

#include "SquadManager.h"
#include "Squad.h"
#include "UnitCount.h"

class InfantrySquad : public Squad
{
private:
	void update(BWAPI::Unit *unit);
	void draw(BWAPI::Unit *unit);

	void updateMedic(BWAPI::Unit *unit);
	void updateInfantry(BWAPI::Unit *unit);

	int potentialEnemy;
	int potentialHelp;

	bool isMineNearby(BWAPI::Unit *unit);
	void layMine(BWAPI::Unit *unit);
public:
	InfantrySquad(SquadManager *manager, std::map<BWAPI::UnitType, UnitCount*> *composition);
	void update();
	void moveTo(BWAPI::Position position);
	void attackTo(BWAPI::Position position);
};