#include "Squad.h"

#include <algorithm>

const int Squad::drawSquadRadius = 30;

Squad::Squad(SquadManager *manager, std::map<BWAPI::UnitType, UnitCount*> *composition)
:
	manager(manager),
	id(manager->generateId()),
	debug(false),
	composition(composition),
	optimalUnits(false),
	center(BWAPI::Positions::None),
	target(BWAPI::Positions::None)
{
	manager->addSquad(this);
}

void Squad::setDebug(bool debug)
{
	this->debug = debug;
}

void Squad::requestUnits()
{
	for (std::map<BWAPI::UnitType, UnitCount*>::iterator i = composition->begin(); i != composition->end(); i++)
	{
		for (int j = (*i).second->getActual(); j < (*i).second->getMax(); j++)
		{
			BWAPI::Unit *unit = manager->requestUnit((*i).first);

			if (unit == NULL)
			{
				break;
			}

			units.insert(unit);
			(*i).second->addOne();
		}
	}
}

bool Squad::hasEnoughUnits()
{
	for (std::map<BWAPI::UnitType, UnitCount*>::iterator i = composition->begin(); i != composition->end(); i++)
	{
		UnitCount *unitCount = (*i).second;

		if (!unitCount->hasEnough())
		{
			return false;
		}
	}

	return true;
}

bool Squad::hasOptimalUnits()
{
	for (std::map<BWAPI::UnitType, UnitCount*>::iterator i = composition->begin(); i != composition->end(); i++)
	{
		UnitCount *unitCount = (*i).second;

		if (!unitCount->hasMax())
		{
			return false;
		}
	}

	return true;
}

UnitGroup Squad::getUnits()
{
	return units;
}

int Squad::getId()
{
	return id;
}

void Squad::update()
{
	calculateCenter();

	if (debug)
	{
		draw();
	}

	if (BWAPI::Broodwar->getFrameCount() % 30 == 0)
	{
		optimalUnits = hasOptimalUnits();

		if (!optimalUnits)
		{
			requestUnits();
		}

		enoughUnits = hasEnoughUnits();
	}
}

void Squad::draw()
{
	if (center == BWAPI::Positions::None)
	{
		return;
	}

	BWAPI::Color squadState;

	if (optimalUnits)
	{
		squadState = BWAPI::Colors::Green;
	}
	else if (enoughUnits)
	{
		squadState = BWAPI::Colors::Orange;
	}
	else
	{
		squadState = BWAPI::Colors::Red;
	}

	BWAPI::Broodwar->drawCircleMap(center.x(), center.y(), drawSquadRadius, squadState);
	BWAPI::Broodwar->drawTextMap(center.x(), center.y() + 30, "Commanded units: %d", units.size());

	BWAPI::Broodwar->drawLineMap(center.x(), center.y(), target.x(), target.y(), BWAPI::Colors::Green);

	int offset = 40;
	std::map<BWAPI::UnitType, UnitCount*>::iterator i;

	for (i = composition->begin(); i != composition->end(); i++)
	{
		BWAPI::UnitType type = (*i).first;
		UnitCount *count = (*i).second;
		BWAPI::Broodwar->drawTextMap(center.x(), center.y() + offset, "%s\t(a%d\to%d\tm%d)", type.getName().c_str(), count->getActual(), count->getMax(), count->getMin());

		offset += 10;
	}
}

void Squad::moveTo(BWAPI::Position target)
{
	this->target = target;
	order = BWAPI::Orders::AttackMove;
}

void Squad::attackTo(BWAPI::Position target)
{
	this->target = target;
	order = BWAPI::Orders::Move;
}

void Squad::calculateCenter()
{
	if (units.size() == 0)
	{
		center = BWAPI::Positions::None;
		return;
	}

	std::vector<int> horizontals;
	std::vector<int> verticals;

	for (std::set<BWAPI::Unit*>::iterator i = units.begin(); i != units.end(); i++)
	{
		BWAPI::Position unitPosition = (*i)->getPosition();

		horizontals.push_back(unitPosition.x());
		verticals.push_back(unitPosition.y());
	}

	sort(horizontals.begin(), horizontals.end());
	sort(verticals.begin(), verticals.end());

	int centerIndex = horizontals.size() / 2;

	center = BWAPI::Position(horizontals.at(centerIndex), verticals.at(centerIndex));
}

void Squad::removeUnit(BWAPI::Unit *unit)
{
	if (units.find(unit) != units.end())
	{
		std::map<BWAPI::UnitType, UnitCount*>::iterator i = composition->find(unit->getType());

		if (i != composition->end())
		{
			UnitCount *count = (*i).second;
			
			count->substractOne();
		}

		units.erase(unit);
	}
}