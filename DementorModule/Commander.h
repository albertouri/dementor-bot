#pragma once

#include "AIModule.h"

namespace dementor
{
	class AIModule;

	class Commander
	{
	private:
		AIModule *bot;

		const unsigned int MAX_WORKERS;
		const unsigned int EXPAND_TIMEOUT;
		const unsigned int SMALL_EXPAND_TIMEOUT;
		const unsigned int SCAN_TIMEOUT;
		const unsigned int MAX_SAME_POSITION_ATTACK;

		bool attacking;
		bool defendingBase;
		bool enemyAttackingWithFlyersOnly;

		int lastScanTime;

		int desiredExpandsCount;
		int timeToExpand;
		int lastExpandIssued;

		int samePositionAttack;

		void revealInvisibleUnits();
		UnitGroup getInfantry();
		UnitGroup getSpecialBuildings();
	public:
		Commander(AIModule *bot);
		void update();
	};
}