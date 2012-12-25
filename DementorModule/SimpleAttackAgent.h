#pragma once

#include <BWAPI.h>

#include "UnitAgent.h"

namespace dementor
{
	class SimpleAttackAgent : public UnitAgent
	{
	private:
		int attackRadius;

		bool isAttacking;

		bool isNear(BWAPI::Position position, int radius);
	public:
		SimpleAttackAgent(BWAPI::Unit *vulture);
		~SimpleAttackAgent(void);

		void update(void);
		void draw(void);
	};
}