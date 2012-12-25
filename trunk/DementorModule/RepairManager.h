#pragma once

#include <BWAPI.h>

#include <Arbitrator.h>

#include "AIModule.h"

namespace dementor
{
	class AIModule;

	class RepairManager : public Arbitrator::Controller<BWAPI::Unit*, double>
	{
	private:
		AIModule *bot;
		BWAPI::Game *game;
		Arbitrator::Arbitrator<BWAPI::Unit*, double> *arbitrator;

		std::set<BWAPI::Unit*> workers;
		std::set<BWAPI::Unit*> unitsToRepair;
		
		void checkUnitsToRepair();
	public:
		RepairManager(AIModule *bot);

		virtual std::string getName() const;
		virtual std::string getShortName() const;

		virtual void onOffer(std::set<BWAPI::Unit*> units);
		virtual void onRevoke(BWAPI::Unit* unit, double bid);

		void onUnitDestroy(BWAPI::Unit *unit);

		void update();
		void draw();
	};
}