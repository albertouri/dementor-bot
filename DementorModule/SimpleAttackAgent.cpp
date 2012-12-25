#include "SimpleAttackAgent.h"

#include <BWAPI.h>

#include "UnitGroupManager.h"

#include "Vector.h"

namespace dementor
{
	SimpleAttackAgent::SimpleAttackAgent(BWAPI::Unit *unit)
	:
		UnitAgent(unit)
	{
		attackRadius = unit->getType().groundWeapon().maxRange();

		isAttacking = false;
	}

	SimpleAttackAgent::~SimpleAttackAgent(void)
	{
	}

	void SimpleAttackAgent::update(void)
	{
		UnitGroup targets = UnitGroup::getUnitGroup(unit->getUnitsInRadius(attackRadius))
		.not(unit->getPlayer())
		.not(isNeutral)
		.not(isSpecialBuilding)
		(isVisible);

		if (isAttacking)
		{
			if (targets.size() == 0)
			{
				unit->stop();
				isAttacking = false;
			}
		}
		else
		{
			if (targets.size() > 0)
			{
				unit->stop();
				isAttacking = true;
			}
			else if (BWAPI::Broodwar->getFrameCount() % 20 == 13)
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
		}
	}

	void SimpleAttackAgent::draw(void)
	{
		if (orderedPosition != BWAPI::Positions::None)
		{
			BWAPI::Position unitPosition = unit->getPosition();

			BWAPI::Broodwar->drawLineMap(unitPosition.x(), unitPosition.y(), orderedPosition.x(), orderedPosition.y(), BWAPI::Colors::Red);
			BWAPI::Broodwar->drawTextMap(unitPosition.x(), unitPosition.y() + 10, "SimpleAttackAgent");
		}
	}
}