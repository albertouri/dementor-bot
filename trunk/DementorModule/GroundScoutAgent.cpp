#include "GroundScoutAgent.h"

#include <cstdlib>

#include "BWAPI.h"

#include "UnitGroupManager.h"

#include "Vector.h"
#include "MapStateManager.h"

namespace dementor
{
	GroundScoutAgent::GroundScoutAgent(MapStateManager *mapStateManager, BWAPI::Unit *scout)
	:
		mapStateManager(mapStateManager),
		UnitAgent(scout)
	{
		game = BWAPI::Broodwar;

		target = BWAPI::Positions::None;
		randomTarget = false;

		safeRadius = 300;
		scoutRadius = unit->getType().sightRange();
		nearTargetRadius = 50;

		lastTargetPickTime = game->getFrameCount();
		targetPickTimeLimit = 100;
	}

	GroundScoutAgent::~GroundScoutAgent()
	{
	}

	bool GroundScoutAgent::isRunningAway()
	{
		return runningAway;
	}

	bool GroundScoutAgent::isSafeHere()
	{
		UnitGroup nearEnemyUnits = UnitGroup::getUnitGroup(unit->getUnitsInRadius(safeRadius))
		.not(isNeutral)
		.not(unit->getPlayer())
		(canAttack);

		return (nearEnemyUnits.size() != 0);
	}

	bool GroundScoutAgent::hasTarget()
	{
		return (target != BWAPI::Positions::None);
	}

	bool GroundScoutAgent::hasFoundEnemyBuildings()
	{
		UnitGroup nearEnemyBuildings = getEnemyBuildingsInRadius(scoutRadius);

		return (nearEnemyBuildings.size() != 0);
	}

	bool GroundScoutAgent::hasFoundEnemyUnits()
	{
		UnitGroup nearEnemyUnits = UnitGroup::getUnitGroup(unit->getUnitsInRadius(scoutRadius))
		.not(unit->getPlayer())
		.not(isNeutral)
		.not(isBuilding);

		return (nearEnemyUnits.size() != 0);
	}

	bool GroundScoutAgent::isNearTarget()
	{
		Vector vector = Vector::fromPositions(unit->getPosition(), target);

		return (vector.getLength() < nearTargetRadius);
	}

	void GroundScoutAgent::stopRunningAway()
	{
		runningAway = false;
		unit->stop();
	}

	void GroundScoutAgent::goToTarget()
	{
		unit->move(target);
	}

	void GroundScoutAgent::pickTarget()
	{
		if (lastTargetPickTime + targetPickTimeLimit > game->getFrameCount())
		{
			return;
		}

		int random = rand() % 5;

		if (random % 2 == 0)
		{
			UnitGroup enemyBuildings = SelectAllEnemy()
			(isBuilding)
			.not(isNeutral);

			if (enemyBuildings.size() > 0)
			{
				BWAPI::Unit *building = (*enemyBuildings.begin());

				target = building->getPosition();
				randomTarget = false;
			}
		}

		if (random == 0)
		{
			int width = game->mapWidth();
			int height = game->mapHeight();

			int x = width - (rand() % width);
			int y = height - (rand() % height);

			target = BWAPI::Position(BWAPI::TilePosition(x, y));
			randomTarget = true;
		}
		else
		{
			target = mapStateManager->requestTarget();
			randomTarget = false;
		}

		lastTargetPickTime = game->getFrameCount();
	}

	void GroundScoutAgent::runAway()
	{
		pickTarget();

		if (hasTarget())
		{
			goToTarget();
		}
	}

	void GroundScoutAgent::clearTarget()
	{
		target = BWAPI::Positions::None;
	}

	UnitGroup GroundScoutAgent::getEnemyBuildingsInRadius(int radius)
	{
		UnitGroup enemyBuildings = UnitGroup::getUnitGroup(unit->getUnitsInRadius(radius))
		.not(isNeutral)
		.not(unit->getPlayer())
		(isBuilding);

		return enemyBuildings;
	}

	void GroundScoutAgent::update()
	{
		if (isRunningAway())
		{
			if (isSafeHere())
			{
				stopRunningAway();

				if (hasTarget())
				{
					goToTarget();
				}

				return;
			}
		}

		if (!hasTarget())
		{
			pickTarget();

			if (hasTarget())
			{
				goToTarget();
			}

			return;
		}
		else if (BWAPI::Broodwar->getFrameCount() % 50 == 31)
		{
			goToTarget();
		}

		if (hasFoundEnemyBuildings())
		{
			runAway();

			return;
		}
		else
		{
			if (hasFoundEnemyUnits())
			{
				mapStateManager->setFallbackAttackPlace(unit->getPosition());

				runAway();
			}
		}

		if (isNearTarget())
		{
			clearTarget();
		}
		else if (lastTargetPickTime + 3000 < BWAPI::Broodwar->getFrameCount())
		{
			pickTarget();

			if (hasTarget())
			{
				goToTarget();
			}
		}
	}

	void GroundScoutAgent::draw()
	{
		BWAPI::Position position = unit->getPosition();

		game->drawCircleMap(position.x(), position.y(), 40, BWAPI::Colors::Cyan);

		BWAPI::Broodwar->drawTextMap(position.x(), position.y() + 10, "GroundScoutAgent");

		if (hasTarget())
		{
			BWAPI::Broodwar->drawTextMap(position.x(), position.y() + 20, "Random scout target: %s", (randomTarget) ? "yes" : "no");
			game->drawLineMap(position.x(), position.y(), target.x(), target.y(), BWAPI::Colors::Green);
		}
	}
}