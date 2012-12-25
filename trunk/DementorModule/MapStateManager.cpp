#include "MapStateManager.h"

#include "BWAPI.h"

#include "Vector.h"

namespace dementor
{
	MapStateManager::MapStateManager(AIModule *bot)
	:
		bot(bot),
		plannedExpandPosition(BWAPI::Positions::None),
		MINIMAL_DISTANCE_TO_ANOTHER_TARGET(600)
	{
		std::set<BWTA::BaseLocation*> bases = BWTA::getBaseLocations();

		std::set<BWTA::BaseLocation*>::iterator it;
		for (it = bases.begin(); it != bases.end(); it++)
		{
			BWTA::BaseLocation *baseLocation = (*it);

			baseLocations.push_back(baseLocation);
		}

		fallbackAttackPlace = BWAPI::Positions::None;

		enemyAttackFlyerSeen = false;

		startLocations = BWTA::getStartLocations();
	}

	void MapStateManager::update()
	{
		BWAPI::Game *game = bot->getGame();

		if (game->getFrameCount() % 100 != 63)
		{
			return;
		}

		// find invalid positions...
		std::list<BWAPI::Position>::iterator it;
		std::list<BWAPI::Position> invalidPositions;
		for (it = enemyBuildings.begin(); it != enemyBuildings.end(); it++)
		{
			BWAPI::Position position = (*it);
			BWAPI::TilePosition tilePosition = BWAPI::TilePosition(position);

			if (game->isVisible(tilePosition))
			{
				UnitGroup unitsOnTile = UnitGroup::getUnitGroup(game->getUnitsOnTile(tilePosition.x(), tilePosition.y()))
				(isBuilding)
				.not(isNeutral)
				.not(game->self());

				if (unitsOnTile.size() == 0)
				{
					invalidPositions.push_back(position);
				}
			}
		}

		// ... and remove it
		std::list<BWAPI::Position>::iterator invalidIt;
		for (invalidIt = invalidPositions.begin(); invalidIt != invalidPositions.end(); invalidIt++)
		{
			BWAPI::Position invalidPosition = (*invalidIt);

			// kedze vyberam udaje z listu, mohla byt pozicia duplicitne
			if (hasSavedPosition(invalidPosition))
			{
				removePosition(invalidPosition);
			}
		}
	}

	BWAPI::Position MapStateManager::requestTarget()
	{
		BWAPI::Game *game = bot->getGame();

		if (game->getFrameCount() < 10000 && enemyBuildingsSet.empty())
		{
			while (startLocations.size() > 0)
			{
				std::set<BWTA::BaseLocation*>::iterator it = startLocations.begin();

				BWTA::BaseLocation *baseLocation = (*it);
				BWAPI::TilePosition tilePosition = baseLocation->getTilePosition();

				if (!game->isExplored(tilePosition.x(), tilePosition.y()))
				{
					return baseLocation->getPosition();
				}
				else
				{
					startLocations.erase(it);
				}
			}
		}

		if (baseLocations.empty())
		{
			return BWAPI::Positions::None;
		}

		BWTA::BaseLocation *baseLocation = baseLocations.front();

		baseLocations.pop_front();
		baseLocations.push_back(baseLocation);

		if (game->isVisible(baseLocation->getTilePosition()))
		{
			// v tomto pripade si scout po chvilke vyziada znova target
			// hack ale zatial funguje
			return BWAPI::Positions::None;
		}
		else
		{
			return baseLocation->getPosition();
		}
	}

	void MapStateManager::saveEnemyBuilding(BWAPI::Unit *enemyBuilding)
	{
		savePosition(enemyBuilding->getPosition());

		// hodim si to do fallback place
		fallbackAttackPlace = enemyBuilding->getPosition();
	}

	bool MapStateManager::hasEnemyBuilding()
	{
		return (enemyBuildings.size() > 0) || fallbackAttackPlace != BWAPI::Positions::None;
	}

	void MapStateManager::setFallbackAttackPlace(BWAPI::Position position)
	{
		this->fallbackAttackPlace = position;
	}

	BWAPI::Position MapStateManager::getEnemyBuildingPosition()
	{
		return getAnotherEnemyBuildingPosition(BWAPI::Positions::None);
	}

	BWAPI::Position MapStateManager::getAnotherEnemyBuildingPosition(BWAPI::Position anotherPosition)
	{
		BWAPI::Game *game = bot->getGame();

		// hack, ked idem expandovat
		// t.j. snazi sa vycistit nepriatelsky expand
		if (plannedExpandPosition != BWAPI::Positions::None)
		{
			UnitGroup nearEnemies = UnitGroup::getUnitGroup(game->getUnitsInRadius(plannedExpandPosition, 200))
			.not(game->self())
			.not(isNeutral);

			nearEnemies = nearEnemies(canAttack) + nearEnemies(isBuilding);

			if (nearEnemies.size() > 0)
			{
				return plannedExpandPosition;
			}
		}

		// remove problematic position
		if (anotherPosition != BWAPI::Positions::None && hasSavedPosition(anotherPosition))
		{
			removePosition(anotherPosition);

			game->printf("Removed previous enemy building position.");
		}

		// get valid position
		BWAPI::Position position = BWAPI::Positions::None;
		BWAPI::Position farPosition = BWAPI::Positions::None;
		std::list<BWAPI::Position>::iterator it;
		for (it = enemyBuildings.begin(); it != enemyBuildings.end(); it++)
		{
			BWAPI::Position positionCandidate = (*it);

			if (anotherPosition != BWAPI::Positions::None)
			{
				Vector vector = Vector::fromPositions(positionCandidate, anotherPosition);

				if (vector.getLength() > MINIMAL_DISTANCE_TO_ANOTHER_TARGET)
				{
					farPosition = positionCandidate;
					break;
				}
			}
			else
			{
				position = positionCandidate;
				break;
			}
		}

		if (anotherPosition != BWAPI::Positions::None)
		{
			if (farPosition != BWAPI::Positions::None)
			{
				return farPosition;
			}
		}

		// check position validity and return valid position
		if (position == BWAPI::Positions::None)
		{
			return fallbackAttackPlace;
		}
		else
		{
			return position;
		}
	}

	void MapStateManager::onUnitDiscover(BWAPI::Unit *unit)
	{
		BWAPI::UnitType unitType = unit->getType();

		checkIfItsAttackFlyer(unit);

		if (!unitType.isBuilding())
		{
			return;		
		}

		// ak ju uz mam evidovanu, ignorovat
		if (hasSavedPosition(unit->getPosition()))
		{
			// hodim si ju do fallback place
			fallbackAttackPlace = unit->getPosition();
			return;
		}

		BWAPI::Game *game = bot->getGame();

		BWAPI::Player *unitPlayer = unit->getPlayer();
		BWAPI::Player *self = bot->getGame()->self();
		if (!unitPlayer->isEnemy(self))
		{
			return;
		}

		// hack pre istotu
		if (unitType.isSpecialBuilding())
		{
			return;
		}

		// tiez pre istotu
		BWAPI::PlayerType playerType = unitPlayer->getType();
		if (playerType != BWAPI::PlayerTypes::Player && playerType != BWAPI::PlayerTypes::Computer)
		{
			return;
		}

		saveEnemyBuilding(unit);
	}

	void MapStateManager::setPlannedExpandPosition(BWAPI::Position position)
	{
		plannedExpandPosition = position;
	}

	void MapStateManager::checkIfItsAttackFlyer(BWAPI::Unit *unit)
	{
		BWAPI::UnitType unitType = unit->getType();

		if (!unitType.isFlyer())
		{
			return;
		}

		if (!unitType.canAttack())
		{
			return;
		}

		BWAPI::Game *game = bot->getGame();
		BWAPI::Player *player = unit->getPlayer();
		if (!game->self()->isEnemy(player))
		{
			return;
		}

		// tiez pre istotu
		BWAPI::PlayerType playerType = player->getType();
		if (playerType != BWAPI::PlayerTypes::Player && playerType != BWAPI::PlayerTypes::Computer)
		{
			return;
		}

		enemyAttackFlyerSeen = true;
	}

	bool MapStateManager::hasEnemyAttackFlyers()
	{
		return enemyAttackFlyerSeen;
	}

	bool MapStateManager::hasSavedPosition(BWAPI::Position position)
	{
		return (enemyBuildingsSet.find(position) != enemyBuildingsSet.end());
	}

	void MapStateManager::savePosition(BWAPI::Position position)
	{
		enemyBuildingsSet.insert(position);
		enemyBuildings.push_front(position);
	}

	void MapStateManager::removePosition(BWAPI::Position position)
	{
		enemyBuildingsSet.erase(enemyBuildingsSet.find(position));
		enemyBuildings.remove(position);
	}

	void MapStateManager::draw()
	{
		BWAPI::Game *game = bot->getGame();

		std::list<BWTA::BaseLocation*>::iterator basesIt;
		for (basesIt = baseLocations.begin(); basesIt != baseLocations.end(); basesIt++)
		{
			BWTA::BaseLocation *base = (*basesIt);
			BWAPI::Position position = base->getPosition();

			game->drawCircleMap(position.x(), position.y(), 300, BWAPI::Colors::Red);
		}

		std::list<BWAPI::Position>::iterator buildingsIt;
		for (buildingsIt = enemyBuildings.begin(); buildingsIt != enemyBuildings.end(); buildingsIt++)
		{
			BWAPI::Position position = (*buildingsIt);

			game->drawCircleMap(position.x(), position.y(), 50, BWAPI::Colors::Red);
		}

		BWAPI::Position attackPosition = BWAPI::Positions::None;

		buildingsIt = enemyBuildings.begin();
		if (buildingsIt != enemyBuildings.end())
		{
			attackPosition = (*buildingsIt);
		}
		else
		{
			attackPosition = fallbackAttackPlace;
		}

		if (attackPosition != BWAPI::Positions::None)
		{
			game->drawCircleMap(attackPosition.x(), attackPosition.y(), 150, BWAPI::Colors::Orange);
		}
	}
}