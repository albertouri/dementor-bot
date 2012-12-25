#pragma once

#include <list>
#include <set>

#include "BWAPI.h"

#include "AIModule.h"

namespace dementor
{
	class AIModule;

	class MapStateManager
	{
	private:
		AIModule *bot;

		const unsigned int MINIMAL_DISTANCE_TO_ANOTHER_TARGET;

		std::set<BWTA::BaseLocation*> startLocations;

		std::list<BWTA::BaseLocation*> baseLocations;
		std::list<BWAPI::Position> enemyBuildings;
		std::set<BWAPI::Position> enemyBuildingsSet;

		BWAPI::Position plannedExpandPosition;
		BWAPI::Position fallbackAttackPlace;

		bool enemyAttackFlyerSeen;

		void checkIfItsAttackFlyer(BWAPI::Unit *unit);

		bool hasSavedPosition(BWAPI::Position position);
		void savePosition(BWAPI::Position position);
		void removePosition(BWAPI::Position position);
	public:
		MapStateManager(AIModule *bot);

		BWAPI::Position requestTarget();
		BWAPI::Position requestAnotherTarget();
		void saveEnemyBuilding(BWAPI::Unit *enemyBuilding);
		bool hasEnemyBuilding();
		BWAPI::Position getEnemyBuildingPosition();
		BWAPI::Position getAnotherEnemyBuildingPosition(BWAPI::Position anotherPosition);

		bool hasEnemyAttackFlyers();

		void setFallbackAttackPlace(BWAPI::Position position);
		void onUnitDiscover(BWAPI::Unit *unit);

		void setPlannedExpandPosition(BWAPI::Position position);

		void update();
		void draw();
	};
}