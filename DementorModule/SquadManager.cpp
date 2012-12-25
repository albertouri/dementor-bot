#include "SquadManager.h"

#include <UnitGroupManager.h>

const double SquadManager::BID = 80.0;
int SquadManager::freeId = 1;

int SquadManager::generateId()
{
	return freeId++;
}

SquadManager::SquadManager(Arbitrator::Arbitrator<BWAPI::Unit*, double> *arbitrator)
:
	arbitrator(arbitrator),
	debug(true)
{
}

void SquadManager::onOffer(std::set<BWAPI::Unit*> units)
{
	for (std::set<BWAPI::Unit*>::iterator i = units.begin(); i != units.end(); i++)
	{
		BWAPI::Unit *unit = (*i);
		BWAPI::Broodwar->printf(unit->getType().getName().c_str());

		arbitrator->accept(this, unit);
		this->freeUnits.insert(unit);
	}
}

void SquadManager::onRevoke(BWAPI::Unit *unit, double bid)
{
	removeUnit(unit);
}

void SquadManager::onRemoveUnit(BWAPI::Unit *unit)
{
	removeUnit(unit);
}

std::string SquadManager::getName() const
{
	return "Squad Manager";
}

std::string SquadManager::getShortName() const
{
	return "Squad";
}

void SquadManager::update()
{
	if (debug)
	{
		draw();
	}

	int frameCount = BWAPI::Broodwar->getFrameCount();

	if (frameCount % 30 == 0)
	{
		findAvailableUnits();
	}

	for (std::set<Squad*>::iterator i = squads.begin(); i != squads.end(); i++)
	{
		(*i)->update();
	}
}

BWAPI::Unit *SquadManager::requestUnit(BWAPI::UnitType unitType)
{
	for (std::set<BWAPI::Unit*>::iterator i = freeUnits.begin(); i != freeUnits.end(); i++)
	{
		BWAPI::Unit *unit = *i;

		if (unit->getType() == unitType)
		{
			freeUnits.erase(unit);
			return unit;
		}
	}

	return NULL;
}

void SquadManager::findAvailableUnits()
{
	std::set<BWAPI::Unit*> availableUnits = SelectAll()(isCompleted).not(isWorker).not(isBuilding);

	for (std::set<BWAPI::Unit*>::iterator i = availableUnits.begin(); i != availableUnits.end(); i++)
	{
		BWAPI::Unit *unit = (*i);

		if (!arbitrator->hasBid(unit))
		{
			arbitrator->setBid(this, unit, BID);
		}
	}
}

void SquadManager::removeUnit(BWAPI::Unit *unit)
{
	freeUnits.erase(unit);

	for (std::set<Squad*>::iterator i = squads.begin(); i != squads.end(); i++)
	{
		(*i)->removeUnit(unit);
	}
}

void SquadManager::draw()
{
	BWAPI::Broodwar->drawTextScreen(200, 10, "Squads: %d", squads.size());
	BWAPI::Broodwar->drawTextScreen(200, 20, "Free units: %d", freeUnits.size());
}

void SquadManager::addSquad(Squad *squad)
{
	squads.insert(squad);
}

void SquadManager::removeSquad(Squad *squad)
{
	squads.erase(squad);
}

std::set<Squad*> SquadManager::getSquads()
{
	return squads;
}

void SquadManager::issueAttackAll(BWAPI::Position position)
{
	for (std::set<Squad*>::iterator i = squads.begin(); i != squads.end(); i++)
	{
		(*i)->attackTo(position);
	}
}

void SquadManager::issueMoveAll(BWAPI::Position position)
{
	for (std::set<Squad*>::iterator i = squads.begin(); i != squads.end(); i++)
	{
		(*i)->moveTo(position);
	}
}

void SquadManager::setDebug(bool debug)
{
	this->debug = debug;

	std::set<Squad*>::iterator i;
	for (i = squads.begin(); i != squads.end(); i++)
	{
		Squad *squad = (*i);

		squad->setDebug(debug);
	}
}