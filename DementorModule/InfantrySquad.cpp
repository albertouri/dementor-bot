#include "InfantrySquad.h"

InfantrySquad::InfantrySquad(SquadManager *manager, std::map<BWAPI::UnitType, UnitCount*> *composition)
:
	potentialEnemy(150),
	potentialHelp(100),
	Squad(manager, composition)
{
}

void InfantrySquad::update()
{
	Squad::update();

	std::set<BWAPI::Unit*>::iterator i;

	for (i = units.begin(); i != units.end(); i++)
	{
		BWAPI::Unit *unit = (*i);

		update(unit);

		if (debug)
		{
			draw(unit);
		}
	}
}

void InfantrySquad::update(BWAPI::Unit *unit)
{
	if (unit->getLastCommandFrame() + 30 > BWAPI::Broodwar->getFrameCount())
	{
		return;
	}

	if (unit->getType() == BWAPI::UnitTypes::Terran_Medic)
	{
		updateMedic(unit);
	}
	else
	{
		updateInfantry(unit);
	}
}

void InfantrySquad::updateMedic(BWAPI::Unit *unit)
{
	std::set<BWAPI::Unit*>::iterator i;

	BWAPI::Unit *target = NULL;
	double distance = 0;

	for (i = units.begin(); i != units.end(); i++)
	{
		BWAPI::Unit *friendlyUnit = (*i);

		if (distance == 0 || friendlyUnit->getPosition().getDistance(unit->getPosition()) < distance)
		{
			distance = friendlyUnit->getPosition().getDistance(unit->getPosition());
			target = friendlyUnit;
		}
	}

	unit->attack(target->getPosition());
}

void InfantrySquad::updateInfantry(BWAPI::Unit *unit)
{
	UnitGroup nearEnemyUnits = UnitGroup::getUnitGroup(unit->getUnitsInRadius(potentialEnemy));
	nearEnemyUnits = nearEnemyUnits.not(BWAPI::Broodwar->self()).not(isNeutral);

	if (nearEnemyUnits.size() > 0)
	{
		BWAPI::Position position = unit->getPosition();

		std::set<BWAPI::Unit*>::iterator i;

		BWAPI::Unit *target;
		double distance = 0;

		for (i = nearEnemyUnits.begin(); i != nearEnemyUnits.end(); i++)
		{
			BWAPI::Unit *enemy = (*i);

			if (distance == 0 || enemy->getPosition().getDistance(position) < distance)
			{
				distance = enemy->getPosition().getDistance(position);
				target = enemy;
			}
		}

		unit->attack(target);
	}
	else
	{
		if (order == BWAPI::Orders::Move)
		{
			unit->move(target);
		}
		else if (order == BWAPI::Orders::AttackMove)
		{
			unit->attack(target);
		}
	}
}

void InfantrySquad::draw(BWAPI::Unit *unit)
{
	BWAPI::Position position = unit->getPosition();

	BWAPI::Broodwar->drawTextMap(position.x(), position.y() + 20, "%s", unit->getOrder().getName().c_str());
	
	BWAPI::Position target = unit->getTargetPosition();
	BWAPI::Order order = unit->getOrder();
	BWAPI::Color color;

	if (order == BWAPI::Orders::Move)
	{
		color = BWAPI::Colors::Green;
	}
	else if (order == BWAPI::Orders::AttackUnit)
	{
		color = BWAPI::Colors::Red;
	}
	else
	{
		color = BWAPI::Colors::Blue;
	}

	BWAPI::Broodwar->drawLineMap(position.x(), position.y(), target.x(), target.y(), color);
}

void InfantrySquad::moveTo(BWAPI::Position position)
{
	Squad::moveTo(position);
}

void InfantrySquad::attackTo(BWAPI::Position position)
{
	Squad::attackTo(position);
}