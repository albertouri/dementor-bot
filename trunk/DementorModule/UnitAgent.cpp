#include "UnitAgent.h"

namespace dementor {

	UnitAgent::UnitAgent(BWAPI::Unit *unit)
	:
		unit(unit)
	{
		orderedPosition = BWAPI::Positions::None;
		orderedTarget = NULL;
	}

	UnitAgent::~UnitAgent()
	{
	}

	void UnitAgent::update()
	{
	}

	void UnitAgent::draw()
	{
		BWAPI::Position position = unit->getPosition();
		BWAPI::Position target = BWAPI::Positions::None;
		if (orderedTarget != NULL)
		{
			target = orderedTarget->getPosition();
		}
		else if (orderedPosition != BWAPI::Positions::None)
		{
			target = orderedPosition;
		}

		if (target != BWAPI::Positions::None)
		{
			BWAPI::Broodwar->drawLineMap(position.x(), position.y(), target.x(), target.y(), BWAPI::Colors::Red);
		}
	}

	void UnitAgent::attackMoveTo(BWAPI::Position position)
	{
		this->orderedPosition = position;
		this->orderedTarget = NULL;
	}

	void UnitAgent::attackUnit(BWAPI::Unit *unit)
	{
		this->orderedTarget = unit;
		this->orderedPosition = BWAPI::Positions::None;
	}
}