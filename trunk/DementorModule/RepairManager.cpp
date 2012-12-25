#include "RepairManager.h"

#include "assert.h"

namespace dementor
{
	RepairManager::RepairManager(AIModule *bot)
	:
		bot(bot)
	{
		this->game = bot->getGame();
		this->arbitrator = bot->getArbitrator();	
	}

	void RepairManager::checkUnitsToRepair()
	{
		UnitGroup::iterator it;

		// zatial iba budovy
		UnitGroup units = SelectAll()
		(isBuilding)
		.not(isConstructing);

		for (it = units.begin(); it != units.end(); it++)
		{
			BWAPI::Unit *unit = (*it);

			if (unit->getHitPoints() != unit->getType().maxHitPoints())
			{
				unitsToRepair.insert(unit);
			}
		}

		// vyhodim jednotky, ktore maju uz plne zivoty
		std::set<BWAPI::Unit*> fullyRepairedUnits;
		for (it = unitsToRepair.begin(); it != unitsToRepair.end(); it++)
		{
			BWAPI::Unit *unitToRepair = (*it);

			if (unitToRepair->getHitPoints() == unitToRepair->getType().maxHitPoints())
			{
				fullyRepairedUnits.insert(unitToRepair);
			}
		}

		for (it = fullyRepairedUnits.begin(); it != fullyRepairedUnits.end(); it++)
		{
			BWAPI::Unit *unit = (*it);

			std::set<BWAPI::Unit*>::iterator unitToRepairIt = unitsToRepair.find(unit);

			assert(unitToRepairIt != unitsToRepair.end());

			unitsToRepair.erase(unitToRepairIt);
		}
	}

	std::string RepairManager::getName() const
	{
		return "Repair Manager";
	}

	std::string RepairManager::getShortName() const
	{
		return "Repair";
	}

	void RepairManager::onOffer(std::set<BWAPI::Unit*> units)
	{
		std::set<BWAPI::Unit*>::iterator it;
		for (it = units.begin(); it != units.end(); it++)
		{
			BWAPI::Unit *unit = (*it);

			if (unit->getType().isWorker() && workers.size() < 3)
			{
				arbitrator->accept(this, unit);
				workers.insert(unit);

				unit->stop();

				game->printf("Repair man acquired :)");
			}
			else
			{
				arbitrator->decline(this, unit, 0);
			}
		}
	}
	void RepairManager::onRevoke(BWAPI::Unit* unit, double bid)
	{
		std::set<BWAPI::Unit*>::iterator it;
		it = workers.find(unit);

		if (it != workers.end())
		{
			workers.erase(it);
		}
	}

	void RepairManager::onUnitDestroy(BWAPI::Unit *unit)
	{
		std::set<BWAPI::Unit*>::iterator it;

		it = workers.find(unit);
		if (it != workers.end())
		{
			arbitrator->removeBid(this, unit);
			workers.erase(it);

			game->printf("Repair man killed :(");

			return;
		}

		it = unitsToRepair.find(unit);
		if (it != unitsToRepair.end())
		{
			unitsToRepair.erase(it);
			return;
		}
	}

	void RepairManager::update()
	{
		if (game->getFrameCount() < 10 || game->getFrameCount() % 30 != 13)
		{
			return;
		}

		checkUnitsToRepair();

		// checknem ci mam co opravovat, ak nie, prepustim opravarov
		if (unitsToRepair.empty())
		{
			std::set<BWAPI::Unit*>::iterator it;
			for (it = workers.begin(); it != workers.end(); it++)
			{
				BWAPI::Unit *worker = (*it);

				arbitrator->removeBid(this, worker);

				game->printf("Firing repair man.");

				// hack aby zacal worker hned tazit
				UnitGroup nearMinerals = UnitGroup::getUnitGroup(worker->getUnitsInRadius(1000))
				(Mineral_Field);

				if (nearMinerals.begin() != nearMinerals.end())
				{
					worker->gather((*nearMinerals.begin()));
				}
			}

			return;
		}

		//repair logic
		if (bot->getGame()->self()->minerals() > 100)
		{
			// get repair men if needed
			if (workers.size() < 3)
			{
				UnitGroup workerRecruits = SelectAll()
				(isWorker)
				(isCompleted)
				.not(isConstructing);

				UnitGroup::iterator it;
				
				for (it = workerRecruits.begin(); it != workerRecruits.end(); it++)
				{
					BWAPI::Unit *workerRecruit = (*it);

					arbitrator->setBid(this, workerRecruit, 90);
				}
			}

			// assign repair mens to units
			if (!unitsToRepair.empty())
			{
				BWAPI::Unit *unitToRepair = (*unitsToRepair.begin());

				std::set<BWAPI::Unit*>::iterator it;
				for (it = workers.begin(); it != workers.end(); it++)
				{
					BWAPI::Unit *worker = (*it);

					if (!worker->isRepairing())
					{
						worker->repair(unitToRepair);	
					}
				}
			}
		}
	}

	void RepairManager::draw()
	{
		std::set<BWAPI::Unit*>::iterator it;

		for (it = unitsToRepair.begin(); it != unitsToRepair.end(); it++)
		{
			BWAPI::Unit *unit = (*it);
			BWAPI::Position position = unit->getPosition();

			game->drawCircleMap(position.x(), position.y(), 50, BWAPI::Colors::Cyan);
		}

		for (it = workers.begin(); it != workers.end(); it++)
		{
			BWAPI::Unit *unit = (*it);
			BWAPI::Position position = unit->getPosition();

			game->drawCircleMap(position.x(), position.y(), 50, BWAPI::Colors::Red);
		}
	}
}