#include "VultureAgent.h"

#include <map>
#include <vector>

#include <cstdlib>

#include "UnitGroupManager.h"

#include "Vector.h"

namespace dementor
{
	VultureAgent::VultureAgent(BWAPI::Unit *vulture)
	:
		UnitAgent(vulture)
	{
		potentialAttack = vulture->getType().sightRange();
		potentialMine = potentialAttack + 200;
		potentialRun = 130;
		potentialRunBuildings = 300;
		potentialMineOffset = 50;
		potentialMineTank = 75;

		runLenght = 150;
		tooBigDistance = 300;

		placeMineStart = 0;

		isRunningAway = false;
		isPlantingMine = false;

		safePlace = unit->getPosition();
	}

	VultureAgent::~VultureAgent(void)
	{
	}

	void VultureAgent::update(void)
	{
		if (isPlantingMine)
		{
			if (unit->getOrder() != BWAPI::Orders::PlaceMine || placeMineStart + 100 < BWAPI::Broodwar->getFrameCount())
			{
				isPlantingMine = false;
			}
			else
			{
				return;
			}
		}

		UnitGroup dangerousUnits = getEnemyUnitsInRadius(potentialRun) + getEnemyUnitsInRadius(potentialRunBuildings)(isBuilding);

		if (isRunningAway)
		{
			if ((!unit->isUnderAttack() && dangerousUnits.size() == 0) || !canRunAway())
			{
				isRunningAway = false;
				unit->stop();
			}
			else
			{
				if (BWAPI::Broodwar->getFrameCount() % 15 == 7)
				{
					if (dangerousUnits.size() > 0)
					{
						runAwayFromUnits(dangerousUnits);
					}
					else
					{
						runAway();
					}
				}
				return;
			}
		}
		else if (unit->isUnderAttack() || dangerousUnits.size() > 0)
		{
			if (canRunAway())
			{
				if (dangerousUnits.size() > 0)
				{
					runAwayFromUnits(dangerousUnits);
				}
				else
				{
					runAway();
				}

				return;
			}
		}

		if (canPlantMine() && shouldPlantMine())
		{
			plantMine();
			return;
		}
		
		pickTarget();

		if (target != NULL && !unit->isAttacking())
		{	
			unit->attack(target);
		}
		else
		{
			executeHigherOrder();
		}
	}

	void VultureAgent::pickTarget()
	{
		UnitGroup enemyUnits = UnitGroup::getUnitGroup(unit->getUnitsInRadius(potentialAttack))
		.not(isFlyer)
		.not(isCloaked)
		.not(isBurrowed);

		UnitGroup friendlyUnits = UnitGroup::getUnitGroup(unit->getUnitsInRadius(potentialAttack))
		(Vulture_Spider_Mine);

		interests = enemyUnits + friendlyUnits;

		std::map<BWAPI::Unit*, double> scores;

		std::set<BWAPI::Unit*>::iterator it;
		for (it = interests.begin(); it != interests.end(); it++)
		{
			BWAPI::Unit *unit = (*it);
			BWAPI::UnitType unitType = unit->getType();

			double score = 0.0;

			if (unit->getPlayer()->isEnemy(BWAPI::Broodwar->self()))
			{
				score = 0.7;

				if (unitType.canAttack())
				{
					score = 0.8;
				}

				if (unitType.isBuilding())
				{
					score = 0.2;

					if (unitType.canAttack())
					{
						score = 0.1;
					}
				}
			}
			else if (unit->getPlayer() == unit->getPlayer())
			{
				if (unit->getType() == BWAPI::UnitTypes::Terran_Vulture_Spider_Mine)
				{
					UnitGroup nearbyTanks = UnitGroup::getUnitGroup(unit->getUnitsInRadius(potentialMineTank))
					(Siege_Tank)
					(isSieged);

					if (nearbyTanks.size() > 0)
					{
						score = 1.0;
					}
				}
			}

			scores.insert(std::pair<BWAPI::Unit*, double>(unit, score));
		}

		target = NULL;
		double bestScore = 0.0;
		
		std::map<BWAPI::Unit*, double>::iterator scores_it;
		for (scores_it = scores.begin(); scores_it != scores.end(); scores_it++)
		{
			BWAPI::Unit *unit = (*scores_it).first;
			double score = (*scores_it).second;

			if (score > bestScore)
			{
				bestScore = score;
				target = unit;
			}
		}
	}

	UnitGroup VultureAgent::getEnemyUnitsInRadius(int radius)
	{
		UnitGroup units = UnitGroup::getUnitGroup(unit->getUnitsInRadius(radius))
		.not(unit->getPlayer())
		.not(isNeutral)
		(canAttack);

		return units;
	}

	bool VultureAgent::canRunAway()
	{
		Vector vector = Vector::fromPositions(safePlace, unit->getPosition());

		return (vector.getLength() > 100);
	}

	void VultureAgent::runAway()
	{
		isRunningAway = true;

		unit->move(safePlace);
	}

	void VultureAgent::runAwayFromUnits(UnitGroup units)
	{
		isRunningAway = true;

		Vector escapeVector(0, 0);
		std::set<BWAPI::Unit*>::iterator it;
		for (it = units.begin(); it != units.end(); it++)
		{
			BWAPI::Unit *enemyUnit = (*it);

			Vector vector = Vector::fromPositions(enemyUnit->getPosition(), unit->getPosition());
			escapeVector = escapeVector + vector;
		}

		escapeVector = escapeVector.normalize();
		escapeVector = escapeVector * 200;

		BWAPI::Position runPosition = unit->getPosition() + escapeVector;

		BWAPI::TilePosition start = BWAPI::TilePosition(unit->getPosition());
		BWAPI::TilePosition end = BWAPI::TilePosition(runPosition);
		
		if (BWTA::getGroundDistance(start, end) > tooBigDistance)
		{
			runAway();
		}
		else
		{
			unit->move(runPosition);
		}
	}

	bool VultureAgent::canPlantMine()
	{
		if (unit->getSpiderMineCount() == 0)
		{
			return false;
		}

		UnitGroup nearbyMines = UnitGroup::getUnitGroup(unit->getUnitsInRadius(potentialMineOffset))
		(Vulture_Spider_Mine);

		if (nearbyMines.size() > 0)
		{
			return false;
		}

		UnitGroup nearbyFriendlyVulnerableUnits = UnitGroup::getUnitGroup(unit->getUnitsInRadius(potentialMineTank))
		(Siege_Tank)
		(isSieged);

		if (nearbyFriendlyVulnerableUnits.size() > 0)
		{
			return false;
		}

		return true;
	}

	bool VultureAgent::shouldPlantMine()
	{
		// hack, niekedy je dobre minovat len tak
		UnitGroup nearFriendlyBuildings = UnitGroup::getUnitGroup(unit->getUnitsInRadius(potentialMine))
		(isBuilding)
		(unit->getPlayer());

		if (rand() % 201 == 0 && unit->isMoving() && unit->getSpiderMineCount() > 1 && nearFriendlyBuildings.size() == 0)
		{
			return true;
		}

		UnitGroup unitsToBlowUp = UnitGroup::getUnitGroup(unit->getUnitsInRadius(potentialMine))
		.not(unit->getPlayer()).not(isNeutral).not(isBuilding).not(isFlyer)(canAttack);

		return (unitsToBlowUp.size() > 0);
	}

	void VultureAgent::plantMine()
	{
		isPlantingMine = true;
		placeMineStart = BWAPI::Broodwar->getFrameCount();

		unit->useTech(BWAPI::TechTypes::Spider_Mines, unit->getPosition());
	}

	void VultureAgent::executeHigherOrder()
	{
		if (orderedTarget != NULL)
		{
			unit->attack(orderedTarget);
			return;
		}

		if (orderedPosition != BWAPI::Positions::None)
		{
			unit->attack(orderedPosition);
			return;
		}
	}

	void VultureAgent::draw(void)
	{
		std::set<BWAPI::Unit*>::iterator it;

		BWAPI::Position position = unit->getPosition();

		BWAPI::Broodwar->drawCircleMap(position.x(), position.y(), potentialAttack, BWAPI::Colors::Blue);
		BWAPI::Broodwar->drawCircleMap(position.x(), position.y(), potentialMine, BWAPI::Colors::Teal);
		BWAPI::Broodwar->drawCircleMap(position.x(), position.y(), potentialRun, BWAPI::Colors::Red);
		BWAPI::Broodwar->drawCircleMap(position.x(), position.y(), potentialRunBuildings, BWAPI::Colors::Orange);

		for (it = interests.begin(); it != interests.end(); it++)
		{
			BWAPI::Unit *unit = (*it);
			BWAPI::Position position = unit->getPosition();
			BWAPI::Color color = BWAPI::Colors::Red;

			bool isEnemy = unit->getPlayer()->isEnemy(BWAPI::Broodwar->self());
			bool isDangerous = unit->getType().canAttack();

			if (isEnemy)
			{
				color = (isDangerous) ? BWAPI::Colors::Red : BWAPI::Colors::Orange;
			}
			else
			{
				color = BWAPI::Colors::Blue;
			}

			BWAPI::Broodwar->drawCircleMap(position.x(), position.y(), 30, color);
		}

		BWAPI::Position targetPosition = unit->getTargetPosition();
		if (targetPosition != BWAPI::Positions::None)
		{
			BWAPI::Broodwar->drawLineMap(position.x(), position.y(), targetPosition.x(), targetPosition.y(), BWAPI::Colors::Red);			
		}

		BWAPI::Broodwar->drawTextMap(position.x(), position.y() + 10, "VultureAgent");

		if (target != NULL)
		{
			BWAPI::Broodwar->drawTextMap(position.x(), position.y() + 20, "Target: %s", target->getType().getName().c_str());
		}

		BWAPI::Broodwar->drawTextMap(position.x(), position.y() + 30, "isRunningAway: %s", (isRunningAway) ? "yes" : "no");
		BWAPI::Broodwar->drawTextMap(position.x(), position.y() + 40, "Mines count: %d", unit->getSpiderMineCount());

		BWAPI::Broodwar->drawTextMap(position.x(), position.y() + 50, "%s", unit->getOrder().getName().c_str());
	}
}