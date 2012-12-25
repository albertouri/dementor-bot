#pragma once

#include <BWAPI.h>

#include "UnitAgent.h"
#include "MapStateManager.h"

namespace dementor
{
	class GroundScoutAgent : public UnitAgent
	{
	private:
		BWAPI::Game *game;

		MapStateManager *mapStateManager;

		BWAPI::Position target;
		bool randomTarget;
		bool runningAway;

		int lastTargetPickTime;
		int targetPickTimeLimit;

		int safeRadius;
		int scoutRadius;
		int nearTargetRadius;

		bool isRunningAway();
		bool isSafeHere();
		bool hasTarget();
		bool hasFoundEnemyBuildings();
		bool hasFoundEnemyUnits();
		bool isNearTarget();
		bool isTargetPositionEmpty();

		void stopRunningAway();
		void goToTarget();
		void pickTarget();
		void runAway();
		void clearTarget();

		UnitGroup getEnemyBuildingsInRadius(int radius);
	public:
		GroundScoutAgent(MapStateManager *mapStateManager, BWAPI::Unit *scout);
		~GroundScoutAgent(void);

		void update(void);
		void draw(void);
	};
}