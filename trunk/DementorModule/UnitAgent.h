#pragma once

#include <BWAPI.h>

namespace dementor
{
	class UnitAgent
	{
	protected:
		BWAPI::Unit *unit;

		BWAPI::Unit *orderedTarget;
		BWAPI::Position orderedPosition;
	public:
		UnitAgent(BWAPI::Unit *unit);
		~UnitAgent(void);

		virtual void update(void);
		virtual void draw(void);
		virtual void attackMoveTo(BWAPI::Position position);
		virtual void attackUnit(BWAPI::Unit *unit);
	};

}