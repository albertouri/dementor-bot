#pragma once

#include <BWAPI.h>

#include "UnitGroup.h"

#include "UnitAgent.h"

namespace dementor
{
	class VultureAgent : public UnitAgent
	{
	private:
		int potentialAttack;
		int potentialMine;
		int potentialRun;
		int potentialRunBuildings;
		int potentialMineOffset;
		int potentialMineTank;

		int runLenght;
		int tooBigDistance;

		int placeMineStart;

		bool isRunningAway;
		bool isPlantingMine;

		std::set<BWAPI::Unit*> interests;

		BWAPI::Unit *target;

		BWAPI::Position safePlace;

		void pickTarget();
		UnitGroup getEnemyUnitsInRadius(int radius);
		
		bool canRunAway();
		void runAway();
		void runAwayFromUnits(UnitGroup units);
		
		bool canPlantMine();
		bool shouldPlantMine();
		void plantMine();

		void executeHigherOrder();
	public:
		VultureAgent(BWAPI::Unit *vulture);
		~VultureAgent(void);

		void update(void);
		void draw(void);
	};
}