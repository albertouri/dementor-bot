#include "Commander.h"

#include <cstdlib>
#include <cmath>

#include <BWAPI.h>
#include <BWTA.h>

#include "UnitGroupManager.h"
#include "ScoutManager.h"
#include "BuildOrderManager.h"
#include "WorkerManager.h"
#include "TechManager.h"
#include "UpgradeManager.h"
#include "BaseManager.h"
#include "InformationManager.h"

#include "ManagerTypes.h"
#include "AgentManager.h"
#include "Vector.h"

namespace dementor
{
	Commander::Commander(AIModule *bot)
	:
		bot(bot),
		MAX_WORKERS(100),
		EXPAND_TIMEOUT(10000),
		SMALL_EXPAND_TIMEOUT(3000),
		desiredExpandsCount(6),
		lastScanTime(0),
		SCAN_TIMEOUT(200),
		MAX_SAME_POSITION_ATTACK(150),
		samePositionAttack(0)
	{
		attacking = false;
		defendingBase = false;
		enemyAttackingWithFlyersOnly = false;

		timeToExpand = EXPAND_TIMEOUT;
		lastExpandIssued = 0;
	}

	void Commander::update()
	{
		BuildOrderManager *buildOrderManager = (BuildOrderManager*)bot->getController(BuildOrderManagerType);
		WorkerManager *workerManager = (WorkerManager*)bot->getController(WorkerManagerType);
		TechManager *techManager = (TechManager*)bot->getController(TechManagerType);
		UpgradeManager *upgradeManager = (UpgradeManager*)bot->getController(UpgradeManagerType);
		BaseManager *baseManager = (BaseManager*)bot->getController(BaseManagerType);
		BorderManager *borderManager = (BorderManager*)bot->getController(BorderManagerType);

		AgentManager *agentManager = (AgentManager*)bot->getController(AgentManagerType);
		MapStateManager *mapStateManager = (MapStateManager*)bot->getController(MapStateManagerType);

		BWAPI::Game *game = bot->getGame();

		int frame = bot->getGame()->getFrameCount();

		if (frame < 2)
		{
			return;
		}

		if (frame == 2)
		{
			UnitGroup workers = SelectAll()(isWorker);
			UnitGroup minerals = AllUnits()(Mineral_Field);

			UnitGroup::iterator it;
			UnitGroup::iterator mineralsIt = minerals.begin();
			for (it = workers.begin(); it != workers.end(); it++)
			{
				if (mineralsIt == minerals.end())
				{
					break;
				}

				BWAPI::Unit *worker = (*it);
				BWAPI::Unit *mineralField (*mineralsIt);

				worker->gather(mineralField);

				mineralsIt++;
			}
		}

		int workersCount = SelectAll()(isWorker).size();

		if (defendingBase && frame < 10000 && frame % 10 == 3)
		{
			// we have emergency
			if (workersCount > 9)
			{
				buildOrderManager->build(1, BWAPI::UnitTypes::Terran_Barracks, 99);
				buildOrderManager->build(3, BWAPI::UnitTypes::Terran_Marine, 99);
			}

			if (workersCount > 11)
			{
				workerManager->setAutoBuildPriority(30);
			}
		}
		else
		{
			// normal build
			workerManager->setAutoBuildPriority(100);

			if (workersCount > 10 && frame % 100 == 1)
			{
				buildOrderManager->build(1, BWAPI::UnitTypes::Terran_Barracks,95);
			}

			if (workersCount > 14 && frame % 100 == 2)
			{
				buildOrderManager->build(2, BWAPI::UnitTypes::Terran_Barracks,90);
			}

			if ((frame < 10000 && frame % 100 == 3) || (frame < 5000 && frame % 10 == 0))
			{
				buildOrderManager->build(10, BWAPI::UnitTypes::Terran_Marine, 90);

				if (SelectAll()(Academy).size() > 0)
				{
					buildOrderManager->build(6, BWAPI::UnitTypes::Terran_Firebat, 90);
					buildOrderManager->build(7, BWAPI::UnitTypes::Terran_Medic, 90);
				}
			}
			else
			{
				buildOrderManager->build(5, BWAPI::UnitTypes::Terran_Marine, 90);
				buildOrderManager->build(0, BWAPI::UnitTypes::Terran_Firebat, 90);
				buildOrderManager->build(0, BWAPI::UnitTypes::Terran_Medic, 90);
			}
		}

		if (mapStateManager->hasEnemyAttackFlyers() && frame % 212 == 0)
		{
			buildOrderManager->build(1, BWAPI::UnitTypes::Terran_Engineering_Bay, 90);

			if (game->self()->minerals() > 600)
			{
				buildOrderManager->build(30, BWAPI::UnitTypes::Terran_Marine, 90);
				buildOrderManager->build(10, BWAPI::UnitTypes::Terran_Medic, 90);
				buildOrderManager->build(5, BWAPI::UnitTypes::Terran_Missile_Turret, 90);

				upgradeManager->upgrade(BWAPI::UpgradeTypes::Terran_Infantry_Weapons);
				upgradeManager->upgrade(BWAPI::UpgradeTypes::Terran_Infantry_Armor);

				upgradeManager->upgrade(BWAPI::UpgradeTypes::U_238_Shells);
			}
			else
			{
				buildOrderManager->build(15, BWAPI::UnitTypes::Terran_Marine, 90);
				buildOrderManager->build(2, BWAPI::UnitTypes::Terran_Missile_Turret, 90);
			}
		}

		if (SelectAll()(Barracks)(isCompleted).size() >= 1 && frame % 100 == 5)
		{
			workerManager->setAutoBuildPriority(80);

			if (SelectAll()(Marine).size() > 4)
			{
				buildOrderManager->build(1, BWAPI::UnitTypes::Terran_Academy, 90);
				buildOrderManager->build(1, BWAPI::UnitTypes::Terran_Comsat_Station, 95);
			}

			buildOrderManager->build(1, BWAPI::UnitTypes::Terran_Factory,50);
			buildOrderManager->build(1, BWAPI::UnitTypes::Terran_Machine_Shop,90);

			techManager->research(BWAPI::TechTypes::Spider_Mines);
			techManager->research(BWAPI::TechTypes::Tank_Siege_Mode);
			upgradeManager->upgrade(BWAPI::UpgradeTypes::Ion_Thrusters);

			buildOrderManager->build(5, BWAPI::UnitTypes::Terran_Vulture, 90);
		}

		if (frame % 100 == 7)
		{
			UnitGroup workers = SelectAll()(isWorker);
			if (workers.size() > MAX_WORKERS && workerManager->isAutoBuildEnabled())
			{
				workerManager->disableAutoBuild();

				game->printf("Disabled worker auto build, too many workers: %d/%d", workers.size(), MAX_WORKERS);
			}
			else if (workers.size() < MAX_WORKERS && !workerManager->isAutoBuildEnabled())
			{
				workerManager->enableAutoBuild();

				game->printf("Enabled worker auto build, not enough workers: %d/%d", workers.size(), MAX_WORKERS);
			}
		}

		if (SelectAll()(Factory)(isCompleted).size() == 1 && frame % 100 == 11)
		{
			buildOrderManager->build(1, BWAPI::UnitTypes::Terran_Engineering_Bay, 90);
			buildOrderManager->build(1, BWAPI::UnitTypes::Terran_Missile_Turret, 90);

			buildOrderManager->build(2, BWAPI::UnitTypes::Terran_Factory, 70);
			buildOrderManager->build(2, BWAPI::UnitTypes::Terran_Machine_Shop,90);
		}

		if (SelectAll()(Factory)(isCompleted).size() == 2 && SelectAll()(Command_Center).size() > 1 && frame % 100 == 13)
		{
			buildOrderManager->build(2, BWAPI::UnitTypes::Terran_Missile_Turret, 90);

			buildOrderManager->build(3, BWAPI::UnitTypes::Terran_Factory, 70);
			buildOrderManager->build(2, BWAPI::UnitTypes::Terran_Machine_Shop,90);
			buildOrderManager->build(1, BWAPI::UnitTypes::Terran_Starport, 50);
			buildOrderManager->build(1, BWAPI::UnitTypes::Terran_Armory, 60);

			upgradeManager->upgrade(BWAPI::UpgradeTypes::Charon_Boosters);
		}

		// metal army composition
		if (frame % 100 == 17)
		{
			if (baseManager->getActiveBases().size() < 2 || game->self()->minerals() < 500)
			{
				buildOrderManager->build(8, BWAPI::UnitTypes::Terran_Vulture, 60);
				buildOrderManager->build(4, BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode, 60);
				buildOrderManager->build(0, BWAPI::UnitTypes::Terran_Wraith, 60);
				buildOrderManager->build(4, BWAPI::UnitTypes::Terran_Goliath, 60);
			}
			else if (game->self()->minerals() < 1000)
			{
				buildOrderManager->build(15, BWAPI::UnitTypes::Terran_Vulture, 60);
				buildOrderManager->build(8, BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode, 60);
				buildOrderManager->build(7, BWAPI::UnitTypes::Terran_Wraith, 60);
				buildOrderManager->build(10, BWAPI::UnitTypes::Terran_Goliath, 60);
			}
			else
			{
				if (SelectAll()(Vulture).size() < 10)
				{
					buildOrderManager->build(15, BWAPI::UnitTypes::Terran_Vulture, 70);
				}
				else
				{
					buildOrderManager->build(25, BWAPI::UnitTypes::Terran_Vulture, 60);
				}

				buildOrderManager->build(15, BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode, 60);
				buildOrderManager->build(10, BWAPI::UnitTypes::Terran_Wraith, 60);
				buildOrderManager->build(15, BWAPI::UnitTypes::Terran_Goliath, 60);
			}
		}
		
		if (game->self()->minerals() > 1000 && frame % 100 == 19)
		{
			if (SelectAll()(Factory).size() < 4)
			{
				buildOrderManager->build(6, BWAPI::UnitTypes::Terran_Factory, 60);
			}

			if (SelectAll()(Starport).size() < 2)
			{
				buildOrderManager->build(2, BWAPI::UnitTypes::Terran_Starport, 60);
			}
		}

		if (game->self()->minerals() > 1000 && frame % 100 == 23)
		{
			buildOrderManager->build(6, BWAPI::UnitTypes::Terran_Factory, 50);

			buildOrderManager->build(1, BWAPI::UnitTypes::Terran_Science_Facility, 50);

			upgradeManager->upgrade(BWAPI::UpgradeTypes::Terran_Vehicle_Plating);
			upgradeManager->upgrade(BWAPI::UpgradeTypes::Terran_Vehicle_Weapons);

			upgradeManager->upgrade(BWAPI::UpgradeTypes::Terran_Ship_Plating);
			upgradeManager->upgrade(BWAPI::UpgradeTypes::Terran_Ship_Weapons);

			buildOrderManager->build(3, BWAPI::UnitTypes::Terran_Armory, 50);
		}

		// expanding logic
		if (frame > 8000 && game->self()->minerals() > 600)
		{
			bool isTime = (frame > timeToExpand);
			bool shouldExpand = (desiredExpandsCount != 0 && lastExpandIssued + SMALL_EXPAND_TIMEOUT < frame);

			if (isTime || shouldExpand)
			{
				std::set<Base*> activeBases = baseManager->getActiveBases();

				std::string reason = (isTime) ? "is time" : "should expand";
				game->printf("Expanding, %s.", reason.c_str());

				BWTA::BaseLocation *expandLocation = baseManager->expand(100);

				if (expandLocation != NULL)
				{
					mapStateManager->setPlannedExpandPosition(expandLocation->getPosition());

					timeToExpand = frame + EXPAND_TIMEOUT;
					lastExpandIssued = frame;
					desiredExpandsCount--;

					UnitGroup commandCenters = SelectAll()(Command_Center);
					buildOrderManager->build(commandCenters.size() + 1, BWAPI::UnitTypes::Terran_Comsat_Station, 95);
				}
				else
				{
					game->printf("Expand failed, can't get suitable location.");
				}
			}
		}

		revealInvisibleUnits();

		// maly hack na refinerky
		if (frame > 10000 && frame % 300 == 73)
		{
			std::set<BWTA::BaseLocation*> baseLocations = BWTA::getBaseLocations();
			std::set<BWTA::BaseLocation*>::iterator it;
			for (it = baseLocations.begin(); it != baseLocations.end(); it++)
			{
				BWTA::BaseLocation *baseLocation = (*it);

				if (baseLocation->isMineralOnly())
				{
					continue;
				}

				BWAPI::Position position = baseLocation->getPosition();

				UnitGroup nearCommandCenters = UnitGroup::getUnitGroup(game->getUnitsInRadius(position, 300))
				(game->self())
				(Command_Center);

				if (nearCommandCenters.size() == 0)
				{
					continue;
				}

				UnitGroup nearRefineries = UnitGroup::getUnitGroup(game->getUnitsInRadius(position, 300))
				(game->self())
				(Refinery);

				std::set<BWAPI::Unit*> geysers = baseLocation->getGeysers();

				int refineryDeficit = geysers.size() - nearRefineries.size();
				if (refineryDeficit <= 0)
				{
					continue;
				}

				UnitGroup nearWorkers = UnitGroup::getUnitGroup(game->getUnitsInRadius(position, 300))
				(game->self())
				(isWorker)
				.not(isConstructing);

				if (nearWorkers.size() == 0)
				{
					continue;
				}

				BWAPI::TilePosition buildPosition = BWAPI::TilePositions::None;

				std::set<BWAPI::Unit*>::iterator geysersIt;
				for (geysersIt = geysers.begin(); geysersIt != geysers.end(); geysersIt++)
				{
					BWAPI::Unit *geyser = (*geysersIt);

					BWAPI::TilePosition geyserPosition = geyser->getTilePosition();

					UnitGroup refineriesOnTile = UnitGroup::getUnitGroup(game->getUnitsOnTile(geyserPosition.x(), geyserPosition.y()))
					(isRefinery);

					if (refineriesOnTile.size() == 0)
					{
						buildPosition = geyserPosition;
						break;
					}
				}

				if (buildPosition == BWAPI::TilePositions::None)
				{
					continue;
				}

				BWAPI::Unit *worker = (*nearWorkers.begin());

				worker->build(buildPosition, BWAPI::UnitTypes::Terran_Refinery);

				game->printf("Build backup refinery.");
				game->pingMinimap(BWAPI::Position(buildPosition));
			}
		}

		// simple base defend
		if (frame % 100 == 37 || (frame < 5000 && frame % 10 == 1))
		{
			UnitGroup commandCenters = SelectAll()(Command_Center);

			UnitGroup::iterator it;
			for (it = commandCenters.begin(); it != commandCenters.end(); it++)
			{
				BWAPI::Unit *commandCenter = (*it);

				UnitGroup nearEnemies = UnitGroup::getUnitGroup(commandCenter->getUnitsInRadius(700))
				.not(commandCenter->getPlayer())
				.not(isNeutral)
				.not(isSpecialBuilding)
				.not(isAddon);

				UnitGroup nearEnemyBuildings = UnitGroup::getUnitGroup(commandCenter->getUnitsInRadius(1200))
				.not(commandCenter->getPlayer())
				.not(isNeutral)
				.not(isSpecialBuilding)
				.not(isAddon)
				(isBuilding)
				(canAttack);

				nearEnemies = nearEnemies + nearEnemyBuildings;

				if (nearEnemies.size() > 0)
				{
					defendingBase = true;

					if (nearEnemies(canAttack).size())
					{
						UnitGroup freeBarracks = SelectAll()(Barracks)(isCompleted).not(isTraining);
						UnitGroup::iterator it;
						for (it = freeBarracks.begin(); it != freeBarracks.end(); it++)
						{
							if (game->self()->minerals() < BWAPI::UnitTypes::Terran_Marine.mineralPrice())
							{
								break;
							}

							BWAPI::Unit *barrack = (*it);

							barrack->train(BWAPI::UnitTypes::Terran_Marine);

							game->printf("Training emergency marine.");
						}
					}

					BWAPI::Unit *enemy = (*nearEnemies.begin());

					UnitGroup infantry = getInfantry();
					infantry.attackMove(enemy->getPosition());

					bool enemyAttackingWithFlyersOnly = (nearEnemies.not(isFlyer).size() == 0);

					if (!enemyAttackingWithFlyersOnly)
					{
						agentManager->attackMoveTo(enemy->getPosition());
					}

					break;
				}
				else
				{
					defendingBase = false;
				}
			}
		}

		// infantry logic
		if (frame > 10000 && frame % 100 == 41)
		{
			if (mapStateManager->hasEnemyAttackFlyers())
			{
				// enemy has flyers, so use infantry as guard militia
				UnitGroup enemyFlyers = AllUnits()
				.not(game->self())
				.not(isNeutral)
				(isFlyer)
				(canAttack);

				// aah i see mutalisks! attack it
				if (enemyFlyers.size() > 0)
				{
					UnitGroup infantry = getInfantry();

					if (infantry.size() > enemyFlyers.size() / 3)
					{
						BWAPI::Unit *enemyFlyer = (*enemyFlyers.begin());

						infantry.attackMove(enemyFlyer->getPosition());
					}
				}
				else if (frame % 300 == 41)
				{
					// nothing to see, so patrol around base randomly
					UnitGroup myBuildings = SelectAll()
					(isBuilding)
					(Command_Center);

					if (myBuildings.size() > 0)
					{
						BWAPI::Position position = (*myBuildings.begin())->getPosition();

						int x, y;
						x = rand() % 1000;
						y = rand() % 1000;

						position = BWAPI::Position(position.x() + x, position.y() + y);

						getInfantry().attackMove(position);
					}
				}
			}
		}

		// attacking logic
		if (frame % 100 == 43 && (!defendingBase || enemyAttackingWithFlyersOnly))
		{
			UnitGroup attackUnits = SelectAll()(Vulture) + SelectAll()(Siege_Tank) + SelectAll()(Goliath);
			attackUnits = attackUnits(isCompleted);

			if (attackUnits.size() >= 15 || baseManager->getActiveBases().size() == 1 && attackUnits.size() >= 8 || attacking && attackUnits.size() >= 10)
			{
				if (!attacking)
				{
					attacking = true;

					game->printf("Commencing attack with %d units", attackUnits.size());
				}

				UnitGroup specialBuildings = getSpecialBuildings();
				if (!specialBuildings.empty())
				{
					BWAPI::Unit *specialBuilding = (*specialBuildings.begin());

					agentManager->attackUnit(specialBuilding);

					if (AllUnits().not(game->self()).not(isNeutral)(isFlyer)(canAttack).size() == 0)
					{
						getInfantry().attackUnit(specialBuilding);
					}

					game->pingMinimap(specialBuilding->getPosition());
				}
				else if (mapStateManager->hasEnemyBuilding())
				{
					BWAPI::Position attackPosition = mapStateManager->getEnemyBuildingPosition();
					samePositionAttack++;

					if (samePositionAttack >= MAX_SAME_POSITION_ATTACK)
					{
						attackPosition = mapStateManager->getAnotherEnemyBuildingPosition(attackPosition);
						samePositionAttack = 0;

						game->printf("Attacking to same position for too long; getting another one.");
					}

					if (attackPosition != BWAPI::Positions::None)
					{
						agentManager->attackMoveTo(attackPosition);

						if (AllUnits().not(game->self()).not(isNeutral)(isFlyer)(canAttack).size() == 0)
						{
							getInfantry().attackMove(attackPosition);
						}

						game->pingMinimap(attackPosition);
					}
				}
			}
			else
			{
				bool shouldFallback = false;

				if (attacking == true)
				{
					shouldFallback = true;

					game->printf("Attack failed, fallback.");
				}

				attacking = false;

				UnitGroup commandCenters = SelectAll()(Command_Center);

				// == 2 kvoli tomu, aby si nezasekol vlastny cliff ked chce expandovat a este neutocil
				if (commandCenters.size() == 2 || shouldFallback)
				{
					std::set<BWTA::Chokepoint*> chokePoints = borderManager->getMyBorder();

					if (chokePoints.size() > 0)
					{
						BWTA::Chokepoint *chokepoint = (*chokePoints.begin());
						BWAPI::Position position = chokepoint->getCenter();

						if (!mapStateManager->hasEnemyAttackFlyers())
						{
							UnitGroup infantry = getInfantry();
							infantry.attackMove(position);
						}

						agentManager->attackMoveTo(position);
					}
				}
			}
		}

		// some draws
		game->drawTextScreen(10, 10, "Attacking? %s", (attacking) ? "yes" : "no");
		game->drawTextScreen(10, 20, "Defending base? %s", (defendingBase) ? "yes" : "no");
		// 30 ma obsadeny scout manager
		game->drawTextScreen(10, 40, "Has enemy got flyers? %s", (mapStateManager->hasEnemyAttackFlyers()) ? "yes" : "no");

		game->drawTextScreen(10, 70, "Elapsed time: %d", game->elapsedTime());
		game->drawTextScreen(10, 80, "Elapsed frames: %d", game->getFrameCount());
		game->drawTextScreen(10, 90, "Threshold to change target: %d", samePositionAttack);
		game->drawTextScreen(10, 100, "Expand planned at: %d", timeToExpand);

		UnitGroup sweeps = SelectAll()(Scanner_Sweep);
		UnitGroup::iterator it;
		for(it = sweeps.begin(); it != sweeps.end(); it++)
		{
			BWAPI::Unit *sweep = (*it);
			BWAPI::Position position = sweep->getPosition();

			game->drawCircleMap(position.x(), position.y(), 50, BWAPI::Colors::Blue);
		}
	}

	UnitGroup Commander::getInfantry()
	{
		UnitGroup infantry = SelectAll()(Marine) + SelectAll()(Firebat) + SelectAll()(Medic);

		return infantry;
	}

	UnitGroup Commander::getSpecialBuildings()
	{
		UnitGroup specialBuildings;

		std::set<BWTA::Chokepoint*> chokepoints = BWTA::getChokepoints();
		std::set<BWTA::Chokepoint*>::iterator chokeIt;

		for (chokeIt = chokepoints.begin(); chokeIt != chokepoints.end(); chokeIt++)
		{
			BWTA::Chokepoint *chokepoint = (*chokeIt);
			BWAPI::TilePosition tilePosition = BWAPI::TilePosition(chokepoint->getCenter());

			std::set<BWAPI::Unit*> unitsOnTile = bot->getGame()->getUnitsOnTile(tilePosition.x(), tilePosition.y());
			std::set<BWAPI::Unit*>::iterator unitsIt;
			for (unitsIt = unitsOnTile.begin(); unitsIt != unitsOnTile.end(); unitsIt++)
			{
				BWAPI::Unit *unit = (*unitsIt);
				BWAPI::UnitType unitType = unit->getType();

				if (unit->isInvincible())
				{
					continue;
				}

				if (!unitType.isSpecialBuilding())
				{
					continue;
				}

				UnitGroup nearAttackUnits = UnitGroup::getUnitGroup(unit->getUnitsInRadius(100))
				(bot->getGame()->self())
				.not(isBuilding)
				.not(isWorker)
				(canAttack);

				if (nearAttackUnits.size() < 3)
				{
					continue;
				}

				specialBuildings.insert(unit);
			}
		}

		return specialBuildings;
	}

	void Commander::revealInvisibleUnits()
	{
		BWAPI::Game *game = bot->getGame();
		int frame = game->getFrameCount();

		if (lastScanTime + SCAN_TIMEOUT > frame)
		{
			return;
		}

		UnitGroup enemyInvisibleUnits = AllUnits()(isCloaked) + AllUnits()(isBurrowed) + AllUnits()(hasPermanentCloak);
		enemyInvisibleUnits = enemyInvisibleUnits
		.not(game->self())
		.not(isNeutral);

		if (enemyInvisibleUnits.size() == 0)
		{
			return;
		}

		UnitGroup comsats = SelectAll()(Comsat_Station);
		UnitGroup::iterator it;
		for (it = enemyInvisibleUnits.begin(); it != enemyInvisibleUnits.end(); it++)
		{
			BWAPI::Unit *enemyUnit = (*it);

			UnitGroup scans = UnitGroup::getUnitGroup(enemyUnit->getUnitsInRadius(500))
			(Scanner_Sweep)
			(game->self());

			if (scans.size() > 0)
			{
				continue;
			}

			UnitGroup availableUnits = UnitGroup::getUnitGroup(enemyUnit->getUnitsInRadius(300))
			(game->self())
			(canAttack);

			if (availableUnits.size() == 0)
			{
				continue;
			}

			UnitGroup::iterator comsats_it;
			for (comsats_it = comsats.begin(); comsats_it != comsats.end(); comsats_it++)
			{
				BWAPI::Unit *comsat = (*comsats_it);

				if (comsat->getEnergy() < 50)
				{
					continue;
				}

				comsat->useTech(BWAPI::TechTypes::Scanner_Sweep, enemyUnit->getPosition());
				lastScanTime = frame;

				game->pingMinimap(enemyUnit->getPosition());

				return;
			}
		}
	}
}