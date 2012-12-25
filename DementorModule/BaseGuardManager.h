#pragma once

#include <BWAPI.h>

#include <Arbitrator.h>

#include "AIModule.h"

namespace dementor
{
	class AIModule;

	class BaseGuardManager : public Arbitrator::Controller<BWAPI::Unit*, double>
	{
	private:
		AIModule *bot;
		BWAPI::Game *game;
		Arbitrator::Arbitrator<BWAPI::Unit*, double> *arbitrator;

		BWAPI::Position baseCenter;

		int guardRadius;

		bool redAlert;

		UnitGroup defenseUnits;

		void freeUnits();
		void getUnitsToDefend(UnitGroup enemyUnits);
	public:
		BaseGuardManager(AIModule *bot, BWAPI::Position baseCenter);

		virtual std::string getName() const;
		virtual std::string getShortName() const;

		virtual void onOffer(std::set<BWAPI::Unit*> units);
		virtual void onRevoke(BWAPI::Unit* unit, double bid);

		void update();
		void draw();
	};
}