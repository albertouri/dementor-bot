#pragma once

#include <BWAPI.h>
#include "UnitAgent.h"

namespace dementor {

	class TankAgent : public UnitAgent
	{
	private:
		int siegeChangeMood;
		int interests;
		int dangers;
		int attackable;
		int attackableBySiege;
		BWAPI::Position closestDangerPosition;
		BWAPI::Position closestInterestPosition;
		int closestDangerDistance;
		int closestInterestDistance;

		void executeHigherOrder();
	public:
		TankAgent(BWAPI::Unit *tank);
		~TankAgent(void);

		void update(void);
		void draw(void);
	};

};