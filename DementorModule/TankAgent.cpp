#include "TankAgent.h"
#include "Vector.h"
#include <BWAPI.h>
#include <UnitGroupManager.h>

using namespace BWAPI;

namespace dementor {
	TankAgent::TankAgent(BWAPI::Unit *tank)
	:
		UnitAgent(tank)
	{
		this->siegeChangeMood = 0;
	}

	TankAgent::~TankAgent(void)
	{
	}

	void TankAgent::executeHigherOrder()
	{
		BWAPI::Game *game = BWAPI::Broodwar;

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

	void TankAgent::update(void)
	{
		int frame = BWAPI::Broodwar->getFrameCount();

		int siegeOuterRadius = UnitTypes::Terran_Siege_Tank_Siege_Mode.groundWeapon().maxRange();
		int safeDistance = 3*siegeOuterRadius / 4;
		int friendlyUnitsRadius = siegeOuterRadius;
		int tankRadius = UnitTypes::Terran_Siege_Tank_Tank_Mode.groundWeapon().maxRange();
		int tankOuterRadius = UnitTypes::Terran_Siege_Tank_Tank_Mode.groundWeapon().maxRange();

		int interestRadius = siegeOuterRadius;

		UnitGroup interested = UnitGroup::getUnitGroup(this->unit->getUnitsInRadius(interestRadius));
		
		std::set<BWAPI::Unit*>::iterator i;

		interests = 0;
		dangers = 0;
		attackable = 0;
		attackableBySiege = 0;
		closestDangerDistance = interestRadius;
		closestInterestDistance = interestRadius;

		for (i = interested.begin(); i != interested.end(); i++) {
			BWAPI::Unit *interest = (*i);

			// ak nie je enemy, pokracuj
			if(!interest->getPlayer()->isEnemy(this->unit->getPlayer()))
			{
				continue;
			}

			if(interest->getType().isFlyer())
			{
				continue;
			}

			int distance = interest->getPosition().getApproxDistance(this->unit->getPosition());

			// nebezpecna vziadlenost
			if(distance < safeDistance && interest->getType().canAttack()) {
				++dangers;
				if(closestDangerPosition == 0 || distance < closestDangerDistance) {
					closestDangerDistance = distance;
					closestDangerPosition = interest->getPosition();
				}
			// vzdialenost na dostrel
			} else if(distance < siegeOuterRadius) {
				BWAPI::UnitType interestType = interest->getType();

				if (interestType.isBuilding())
				{
					attackableBySiege += 3;
				}
				else
				{
					attackableBySiege += 1;
				}
				if(distance < tankRadius) {
					++attackable;
				}
			// vzdialenost za dostrelom
			} else {
				++interests;
				if(distance < closestInterestDistance) {
					closestInterestDistance = distance;
					closestInterestPosition = interest->getPosition();
				}
			}
		}

		// test na vzdialenost k targetu, ak mam
		if (orderedTarget != NULL)
		{
			Vector targetVector = Vector::fromPositions(unit->getPosition(), orderedTarget->getPosition());

			if (targetVector.getLength() < siegeOuterRadius)
			{
				attackableBySiege += 3;
			}
		}

		// nemame na co utocit?
		if(attackableBySiege == 0)
		{
			// ak nemam na co utocit a dlho som stal, tak sa odsiegujem
			if(unit->isSieged() && siegeChangeMood > 100)
			{
				unit->unsiege();
				siegeChangeMood = 0;
			}
			else if (frame % 50 == 33)
			{
				executeHigherOrder();
			}

			siegeChangeMood++;
		}
		else
		{
			// je na co utocit :)

			// ak niesom siegnuty, a su na to okolnosti, siegnem
			if(!unit->isSieged())
			{
				if (siegeChangeMood > 100 && attackableBySiege > 2)
				{
					unit->siege();
					siegeChangeMood = 0;
				}
				else
				{
					siegeChangeMood++;
				}
			}

			// ak mam na co utocit, ale interests == 0 (t.j. mal by som utocit na orderedTarget), zautocim nan
			if (interests == 0 && frame % 50 == 33)
			{
				executeHigherOrder();
			}
		}
	}

	void TankAgent::draw(void) {
		BWAPI::Position pos = this->unit->getPosition();

		BWAPI::Broodwar->drawTextMap(pos.x(), pos.y()+10, "%s: %d", "Interests", this->interests);
		BWAPI::Broodwar->drawTextMap(pos.x(), pos.y()+20, "%s: %d", "Attackable", this->attackableBySiege);
		BWAPI::Broodwar->drawTextMap(pos.x(), pos.y()+30, "%s: %d", "Mood", this->siegeChangeMood);

		BWAPI::Broodwar->drawCircleMap(pos.x(), pos.y(), this->closestDangerDistance, BWAPI::Colors::Red);
		BWAPI::Broodwar->drawCircleMap(pos.x(), pos.y(), this->closestInterestDistance, BWAPI::Colors::Green);
	}
}