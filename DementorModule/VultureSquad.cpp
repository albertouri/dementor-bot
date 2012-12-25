#include "VultureSquad.h"

VultureSquad::VultureSquad(SquadManager *manager, std::map<BWAPI::UnitType, UnitCount*> *composition)
:
	potentialAttack(300),
	potentialRun(130),
	potentialMine(50),
	Squad(manager, composition)
{
}

void VultureSquad::update()
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

void VultureSquad::update(BWAPI::Unit *unit)
{
	UnitGroup nearEnemyUnits = UnitGroup::getUnitGroup(unit->getUnitsInRadius(potentialRun));
	nearEnemyUnits = nearEnemyUnits.not(BWAPI::Broodwar->self())(canAttack);

	if (nearEnemyUnits.size() > 0)
	{
		std::set<BWAPI::Unit*>::iterator i;

		BWAPI::Position unitPosition = unit->getPosition();
		BWAPI::Position vector(unitPosition);

		int vecX = 0;
		int vecY = 0;

		for (i = nearEnemyUnits.begin(); i != nearEnemyUnits.end(); i++)
		{
			BWAPI::Position enemyPosition = (*i)->getPosition();
			
			vecX += enemyPosition.x() - unitPosition.x();
			vecY += enemyPosition.y() - unitPosition.y();
		}

		int targetX = (vecX * (-1)) + unitPosition.x();
		int targetY = (vecY * (-1)) + unitPosition.y();

		vector = BWAPI::Position(targetX, targetY);
		vector.makeValid();

		unit->move(vector);
	}
	else
	{
		UnitGroup unitGroup = UnitGroup::getUnitGroup(unit->getUnitsInRadius(potentialAttack));
		unitGroup = unitGroup.not(BWAPI::Broodwar->self()).not(isNeutral)(isVisible);

		if (unitGroup.size() > 0 && unit->getOrder() != BWAPI::Orders::AttackUnit)
		{
			if (!isMineNearby(unit) && unit->getSpiderMineCount() > 0)
			{
				layMine(unit);
				return;
			}

			BWAPI::Position unitPosition = unit->getPosition();

			double distance = 0;
			BWAPI::Unit *targetUnit;

			std::set<BWAPI::Unit*>::iterator i;
			UnitGroup interestingTargets = unitGroup(canAttack) + unitGroup(isAttacking);

			if (interestingTargets.size() > 0)
			{
				unitGroup = interestingTargets;
			}

			for (i = unitGroup.begin(); i != unitGroup.end(); i++)
			{
				BWAPI::Unit *enemyUnit = (*i);

				if (distance == 0 || enemyUnit->getPosition().getDistance(unitPosition) < distance)
				{
					distance = enemyUnit->getPosition().getDistance(unitPosition);
					targetUnit = enemyUnit;
				}
			}

			unit->attack(targetUnit);
		}
		else if (unit->getOrder() == BWAPI::Orders::PlayerGuard)
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
}

void VultureSquad::draw(BWAPI::Unit *unit)
{
	BWAPI::Position position = unit->getPosition();

	BWAPI::Broodwar->drawCircleMap(position.x(), position.y(), potentialAttack, BWAPI::Colors::Green);
	BWAPI::Broodwar->drawCircleMap(position.x(), position.y(), potentialRun, BWAPI::Colors::Red);

	BWAPI::Broodwar->drawTextMap(position.x(), position.y() + 20, "%s", unit->getOrder().getName().c_str());
	
	BWAPI::Position target = unit->getTargetPosition();
	BWAPI::Order order = unit->getOrder();
	BWAPI::Color color;

	if (order == BWAPI::Orders::Move)
	{
		color = BWAPI::Colors::Green;
	}
	else if (order == BWAPI::Orders::AttackUnit || order == BWAPI::Orders::AttackMove)
	{
		color = BWAPI::Colors::Red;
	}
	else
	{
		color = BWAPI::Colors::Blue;
	}

	BWAPI::Broodwar->drawLineMap(position.x(), position.y(), target.x(), target.y(), color);
}

void VultureSquad::moveTo(BWAPI::Position position)
{
	Squad::moveTo(position);
}

void VultureSquad::attackTo(BWAPI::Position position)
{
	Squad::attackTo(position);
}

bool VultureSquad::isMineNearby(BWAPI::Unit *unit)
{
	UnitGroup units = UnitGroup::getUnitGroup(unit->getUnitsInRadius(potentialMine))(Vulture_Spider_Mine);

	return (units.size() != 0);
}

void VultureSquad::layMine(BWAPI::Unit *unit)
{
	if (unit->getOrder() == BWAPI::Orders::PlaceMine)
	{
		return;
	}

	unit->useTech(BWAPI::TechTypes::Spider_Mines, unit->getPosition());
}