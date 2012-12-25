#include <BaseManager.h>
#include <BuildOrderManager.h>
#include <BorderManager.h>
#include "UnitGroupManager.h"

#include "../DementorModule/Vector.h"

BaseManager::BaseManager()
{
  this->builder = NULL;
  this->RefineryNeeded  = 1;
  this->refineryBuildPriority = 0;
}
void BaseManager::setBuildOrderManager(BuildOrderManager* builder)
{
  this->builder = builder;
}
void BaseManager::setBorderManager(BorderManager* borderManager)
{
  this->borderManager = borderManager;
}

void BaseManager::update()
{
	for(std::set<Base*>::iterator b = this->allBases.begin(); b != this->allBases.end(); b++)
	{
		Base *base = (*b);

		// periodicky check, ci baza pouziva najblizsi resource depot
		if (BWAPI::Broodwar->getFrameCount() % 300 == 231)
		{
			BWAPI::TilePosition tilePosition = base->getBaseLocation()->getTilePosition();
			BWAPI::Position position = BWAPI::Position(tilePosition);

			UnitGroup centers = UnitGroup::getUnitGroup(BWAPI::Broodwar->getUnitsInRadius(position, 600))
			(BWAPI::Broodwar->self())
			(isResourceDepot);
			UnitGroup::iterator it;

			BWAPI::Unit *nearestCenter = NULL;
			double distance = 10000;

			for (it = centers.begin(); it != centers.end(); it++)
			{
				BWAPI::Unit *centerCandidate = (*it);
				dementor::Vector vector = dementor::Vector::fromPositions(position, centerCandidate->getPosition());

				if (vector.getLength() < distance)
				{
					nearestCenter = centerCandidate;
					distance = vector.getLength();
				}
			}

			if (nearestCenter != NULL)
			{
				base->setResourceDepot(nearestCenter);
			}

			if (base->getResourceDepot() != NULL)
			{
				if (!base->getResourceDepot()->exists())
				{
					base->setResourceDepot(NULL);
				}
				else if (base->getResourceDepot()->isCompleted() || base->getResourceDepot()->getRemainingBuildTime() < 250)
				{
					base->setActive(true);
				}
			}
		}

		//Set Refinerys
		if (!base->isActiveGas() && base->hasGas())
		{
			if (base->getRefinery() == NULL)
			{
				std::set<BWAPI::Unit*> baseGeysers = (*b)->getBaseLocation()->getGeysers();

				BWAPI::TilePosition geyserLocation;

				//cycle through geysers & get tile location
				for(std::set<BWAPI::Unit*>::iterator bg = baseGeysers.begin(); bg != baseGeysers.end(); bg++)
				{
					geyserLocation = (*bg)->getTilePosition();
				}

				//check for refinery already on geyser
				std::set<BWAPI::Unit*> unitsOnGeyser = BWAPI::Broodwar->getUnitsOnTile(geyserLocation.x(),geyserLocation.y());

				for(std::set<BWAPI::Unit*>::iterator u = unitsOnGeyser.begin(); u != unitsOnGeyser.end(); u++)
				{
					BWAPI::Unit *unit = (*u);
					if (unit->getPlayer() == BWAPI::Broodwar->self() && unit->getType().isRefinery())
					{
						base->setRefinery(*u);
						break;
					}
				}
			}

			if (base->getRefinery() != NULL)
			{
				if (!base->getRefinery()->exists())
				{
					base->setResourceDepot(NULL);
				}
				else
				{
					if (base->getRefinery()->isCompleted() || base->getRefinery()->getRemainingBuildTime() < 250)
					{
						base->setActiveGas(true);
					}
				}
			}
		}
	}

	//check to see if any new base locations need to be added
	for(std::set<BWTA::BaseLocation*>::const_iterator bl = BWTA::getBaseLocations().begin(); bl != BWTA::getBaseLocations().end(); bl++)
	{
		if (location2base.find(*bl) == location2base.end())
		{
			BWAPI::TilePosition tile = (*bl)->getTilePosition();
			std::set<BWAPI::Unit*> units = BWAPI::Broodwar->getUnitsOnTile(tile.x(), tile.y());

			for(std::set<BWAPI::Unit*>::iterator u = units.begin(); u != units.end(); u++)
			{
				if ((*u)->getPlayer() == BWAPI::Broodwar->self() && (*u)->getType().isResourceDepot())
				{
					addBase(*bl);
				}
			}
		}
	}

	if(BWAPI::Broodwar->getFrameCount() % 500 == 0 && BWAPI::Broodwar->getFrameCount() >= 3000 && refineryBuildPriority > 0)
	{
		updateRefineries();
	}
}


void BaseManager::updateRefineries()
{
	//if refinerys needed, build ONE refinery.
	if (this->isRefineryNeeded())
	{
		std::set<Base*> gasBases = this->getAllBasesWithGas();

		for(std::set<Base*>::iterator b = gasBases.begin(); b != gasBases.end(); b++)
		{
			BWTA::BaseLocation* location = (*b)->getBaseLocation();
			if (!this->hasRefinery(location))
			{
				this->builder->buildAdditional(1,BWAPI::Broodwar->self()->getRace().getRefinery(),refineryBuildPriority,(*b)->getBaseLocation()->getTilePosition());
				break;
			}
		}
	}
}

bool BaseManager::isRefineryNeeded()
{
  return (this->RefineryNeeded > this->builder->getPlannedCount(BWAPI::Broodwar->self()->getRace().getRefinery()));
}

void BaseManager::setRefineryBuildPriority(int priority)
{
  this->refineryBuildPriority = priority;
}

int BaseManager::getRefineryBuildPriority()
{
  return this->refineryBuildPriority;
}

void BaseManager::addBase(BWTA::BaseLocation* location)
{
	Base* newBase = new Base(location);

	allBases.insert(newBase);

	this->location2base[location] = newBase;
	this->borderManager->addMyBase(location);
}
void BaseManager::removeBase(BWTA::BaseLocation* location)
{
  std::map<BWTA::BaseLocation*,Base*>::iterator removebase;

  removebase = this->location2base.find(location);
  this->location2base.erase(removebase);
}
Base* BaseManager::getBase(BWTA::BaseLocation* location)
{
  std::map<BWTA::BaseLocation*,Base*>::iterator i=location2base.find(location);
  if (i==location2base.end())
    return NULL;
  return i->second;
}


BWTA::BaseLocation* BaseManager::expand(int priority)
{
	BWTA::BaseLocation* location = NULL;
	BWTA::BaseLocation* locationWithoutGas = NULL;

	double minDist=-1;
	BWTA::BaseLocation* home = BWTA::getStartLocation(BWAPI::Broodwar->self());

	std::set<BWTA::BaseLocation*>::const_iterator i;
	for(i = BWTA::getBaseLocations().begin(); i != BWTA::getBaseLocations().end(); i++)
	{
		BWTA::BaseLocation *baseLocation = (*i);

		if (baseLocation->isIsland())
		{
			continue;
		}

		double dist = home->getGroundDistance(baseLocation);
		if (dist > 0 && getBase(baseLocation) == NULL)
		{
			if (minDist == -1 || dist < minDist)
			{
				if (baseLocation->isMineralOnly())
				{
					locationWithoutGas = baseLocation;
				}
				else
				{
					location = baseLocation;
					minDist = dist;
				}
			}
		}
	}

	if (location == NULL && locationWithoutGas != NULL)
	{
		BWAPI::Broodwar->printf("Expanding to location without gas.");

		location = locationWithoutGas;
	}

	return expand(location, priority);
}
BWTA::BaseLocation* BaseManager::expand(BWTA::BaseLocation* location, int priority)
{
	if (location == NULL)
	{
		return NULL;
	}

	addBase(location);

	this->builder->buildAdditional(1, BWAPI::Broodwar->self()->getRace().getCenter(), priority, location->getTilePosition());

	if(!(location->isMineralOnly()))  
	{
		int geysersCount = location->getGeysers().size();
		this->RefineryNeeded += geysersCount;

		if (!(this->hasRefinery(location)))
		{
			this->builder->buildAdditional(geysersCount, BWAPI::Broodwar->self()->getRace().getRefinery(), priority, location->getTilePosition());
		}
	}

	return location;
}


std::set<Base*> BaseManager::getActiveBases() const
{
  std::set<Base*> activeBases;
  for(std::set<Base*>::const_iterator b = this->allBases.begin(); b != this->allBases.end(); b++)
    if ((*b)->isActive())
      activeBases.insert(*b);
  return activeBases;
}
std::set<Base*> BaseManager::getAllBases() const
{
  std::set<Base*> allBases;
  for(std::set<Base*>::const_iterator b = this->allBases.begin(); b != this->allBases.end(); b++)
    allBases.insert(*b);
  return allBases;
}

std::set<Base*> BaseManager::getAllBasesWithGas()
{
  std::set<Base*> allBasesWithGas;

  for(std::set<Base*>::iterator b = this->allBases.begin(); b != this->allBases.end(); b++)
    if ((*b)->hasGas())
    {
      allBasesWithGas.insert(*b);
    }
  return allBasesWithGas;
}

std::string BaseManager::getName()
{
  return "Base Manager";
}
void BaseManager::onRemoveUnit(BWAPI::Unit* unit)
{
  for(std::set<Base*>::const_iterator b = this->allBases.begin(); b != this->allBases.end(); b++)
  {
    if((*b)->getResourceDepot() == unit)
    {
      if (unit->isCompleted())
      {
        this->borderManager->removeMyBase((*b)->getBaseLocation());
        
        BWTA::BaseLocation* blocation  = (*b)->getBaseLocation();
        removeBase(blocation);
        allBases.erase(b);
      }
      else
        (*b)->setResourceDepot(NULL);
      break;
    }
    else if((*b)->getRefinery() == unit)
    {
      (*b)->setRefinery(NULL);
      (*b)->setActiveGas(false);
      break;
    }
  }
}


bool BaseManager::hasRefinery(BWTA::BaseLocation* location)
{
	bool refinery = false;

	//if base has gas
	if(location->isMineralOnly())
	{
		return false;
	}

	std::set<BWAPI::Unit*> basegeysers = location->getGeysers();

	BWAPI::TilePosition geyserlocation;

	//cycle through geysers & get tile location
	for(std::set<BWAPI::Unit*>::iterator bg = basegeysers.begin(); bg != basegeysers.end(); bg++)
	{
		geyserlocation = (*bg)->getInitialTilePosition();
	}

	//check for refinery already on geyser

	//get units on geyser
	std::set<BWAPI::Unit*> unitsOnGeyser = BWAPI::Broodwar->getUnitsOnTile(geyserlocation.x(),geyserlocation.y());

	//cycle through units on geyser
	for(std::set<BWAPI::Unit*>::iterator u = unitsOnGeyser.begin(); u != unitsOnGeyser.end(); u++)
	{
		//if unit is a refinery
		if ((*u)->getType().isRefinery())
		{
			refinery = true;
		}
	}

	return refinery;
}
