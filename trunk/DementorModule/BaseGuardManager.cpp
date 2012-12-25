#include "BaseGuardManager.h"

#include <assert.h>

#include "Vector.h"

namespace dementor
{
	BaseGuardManager::BaseGuardManager(AIModule *bot, BWAPI::Position baseCenter)
	{
		this->bot = bot;
		this->game = bot->getGame();
		this->arbitrator = bot->getArbitrator();

		this->baseCenter = baseCenter;

		guardRadius = 700;

		redAlert = false;
	}

	std::string BaseGuardManager::getName() const
	{
		return "Base Guard Manager";
	}

	std::string BaseGuardManager::getShortName() const
	{
		return "BaseGuard";
	}

	void BaseGuardManager::onOffer(std::set<BWAPI::Unit*> units)
	{
		std::set<BWAPI::Unit*>::iterator it;
		for (it = units.begin(); it != units.end(); it++)
		{
			BWAPI::Unit *unit = (*it);

			arbitrator->accept(this, unit);
			defenseUnits.insert(unit);
		}
	}

	void BaseGuardManager::onRevoke(BWAPI::Unit* unit, double bid)
	{
		UnitGroup::iterator it;

		it = defenseUnits.find(unit);

		if (it != defenseUnits.end())
		{
			defenseUnits.erase(it);
		}
	}

	void BaseGuardManager::update()
	{
		if (game->getFrameCount() < 10)
		{
			return;
		}

		UnitGroup units = UnitGroup::getUnitGroup(game->getUnitsInRadius(baseCenter, guardRadius));
		UnitGroup myBattleUnits = units
		(game->self())
		(canAttack)
		(isCompleted)
		.not(isBuilding)
		.not(isWorker);

		UnitGroup enemyUnits = units
		.not(game->self())
		.not(isNeutral)
		.not(isFlyer);

		if (defenseUnits.size() == 0)
		{
			if (myBattleUnits.size() > enemyUnits.size() / 2)
			{
				return;
			}

			if (game->getFrameCount() > 10000 && myBattleUnits.size() != 0)
			{
				return;
			}
		}

		UnitGroup attackingUnits = enemyUnits;

		if (enemyUnits.size() > 0)
		{
			if (attackingUnits.size() > 0)
			{
				if (!redAlert)
				{
					redAlert = true;

					getUnitsToDefend(enemyUnits);

					game->printf("Base alert!");
				}
				else
				{
					if (game->getFrameCount() % 50 == 0)
					{
						getUnitsToDefend(enemyUnits);

						std::set<BWAPI::Unit*>::iterator it;
						for (it = defenseUnits.begin(); it != defenseUnits.end(); it++)
						{
							BWAPI::Unit *myUnit = (*it);
							BWAPI::Unit *enemyUnit = NULL;
							double distance = 10000;
							
							std::set<BWAPI::Unit*>::iterator enemyIt;
							for (enemyIt = enemyUnits.begin(); enemyIt != enemyUnits.end(); enemyIt++)
							{
								BWAPI::Unit *enemyCandidate = (*enemyIt);
								Vector vector = Vector::fromPositions(enemyCandidate->getPosition(), myUnit->getPosition());

								if (vector.getLength() < distance)
								{
									distance = vector.getLength();
									enemyUnit = enemyCandidate;
								}
							}

							if (enemyUnit != NULL)
							{
								myUnit->attack(enemyUnit);
							}
						}
					}
				}
			}
		}
		else
		{
			if (redAlert)
			{
				redAlert = false;

				freeUnits();

				game->printf("Base is safe.");
			}
		}

		if (redAlert)
		{
			draw();
		}
	}

	void BaseGuardManager::getUnitsToDefend(UnitGroup enemyUnits)
	{
		UnitGroup workers = UnitGroup::getUnitGroup(enemyUnits)
		(isWorker);

		UnitGroup myWorkers = SelectAll()
		(isWorker)
		(isCompleted)
		.not(isConstructing);

		UnitGroup dangerousUnits = enemyUnits(Zergling) + enemyUnits(Zealot);

		if (workers.size() > 0 && dangerousUnits.size() == 0)
		{
			int enemyWorkersCount = workers.size();

			int biddedWorkersCount = 0;
			UnitGroup::iterator it;
			for (it = myWorkers.begin(); it != myWorkers.end(); it++)
			{
				BWAPI::Unit *myWorker = (*it);

				arbitrator->setBid(this, myWorker, 100);

				biddedWorkersCount++;

				if (biddedWorkersCount == enemyWorkersCount)
				{
					break;
				}
			}
		}
		else
		{
			UnitGroup::iterator it;
			for (it = myWorkers.begin(); it != myWorkers.end(); it++)
			{
				BWAPI::Unit *myWorker = (*it);

				arbitrator->setBid(this, myWorkers, 100);
			}
		}
	}

	void BaseGuardManager::freeUnits()
	{
		int resourceRadius = 1000;

		// hack, aby poslal workerov na refinerky
		UnitGroup refineries = UnitGroup::getUnitGroup(game->getUnitsInRadius(baseCenter, resourceRadius))
		(game->self())
		(isRefinery);

		BWAPI::Unit *refinery = (refineries.empty()) ? NULL : (*refineries.begin());

		bool hasRefinery = (refinery != NULL);
		int sentToGas = 0;

		int workersCount = defenseUnits(isWorker).size();

		UnitGroup::iterator it;
		for (it = defenseUnits.begin(); it != defenseUnits.end(); it++)
		{
			BWAPI::Unit *unit = (*it);

			unit->stop();

			// hack, kedze bwsal worker manager ich obcas nechal tak
			if (unit->getType().isWorker())
			{
				if (hasRefinery && sentToGas < 3 && workersCount > 8)
				{
					unit->gather(refinery);
					sentToGas++;
				}
				else
				{
					UnitGroup nearMinerals = UnitGroup::getUnitGroup(unit->getUnitsInRadius(resourceRadius))
					(Mineral_Field);

					if (nearMinerals.begin() != nearMinerals.end())
					{
						unit->gather((*nearMinerals.begin()));
					}
				}
			}

			arbitrator->removeBid(this, unit);
		}

		defenseUnits.clear();
	}

	void BaseGuardManager::draw()
	{
		int x, y;
		BWAPI::Color circleColor;

		x = baseCenter.x();
		y = baseCenter.y();
		circleColor = (redAlert) ? BWAPI::Colors::Red : BWAPI::Colors::Green;

		game->drawCircleMap(x, y, guardRadius, circleColor);

		UnitGroup::iterator it;
		for (it = defenseUnits.begin(); it != defenseUnits.end(); it++)
		{
			BWAPI::Unit *unit = (*it);
			BWAPI::Position position = unit->getPosition();
			
			game->drawCircleMap(position.x(), position.y(), 30, BWAPI::Colors::Green);

			if (unit->getOrder() == BWAPI::Orders::AttackMove)
			{
				BWAPI::Unit *target = unit->getOrderTarget();
				BWAPI::Position targetPosition = target->getPosition();

				game->drawLineMap(position.x(), position.y(), targetPosition.x(), targetPosition.y(), BWAPI::Colors::Red);
			}
		}
	}
}