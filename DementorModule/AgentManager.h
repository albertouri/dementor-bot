#pragma once

#include <set>
#include <map>

#include <BWAPI.h>

#include "AIModule.h"
#include "UnitAgent.h"

namespace dementor {
	class AIModule;

	class AgentManager : public Arbitrator::Controller<BWAPI::Unit*,double>
	{
	private:
		AIModule *bot;
		Arbitrator::Arbitrator<BWAPI::Unit*, double> *arbitrator;
		//std::set<UnitAgent*> agents;
		std::map<BWAPI::Unit*, UnitAgent*> agents;
		std::set<UnitAgent*> scouts;

		void destroyAgent(BWAPI::Unit *unit, UnitAgent *agent);
		void destroyScoutAgent(UnitAgent *agent);
	public:
		AgentManager(AIModule *bot);
		~AgentManager(void);

		virtual std::string getName() const;
		virtual std::string getShortName() const;

		void onUnitCreate(BWAPI::Unit *unit);
		void onUnitDestroy(BWAPI::Unit *unit);

		void update(void);
		void draw(void);
		void moveTo(BWAPI::Position position);
		void attackMoveTo(BWAPI::Position position);
		void attackUnit(BWAPI::Unit *unit);

		virtual void onOffer(std::set<BWAPI::Unit*> units);
		virtual void onRevoke(BWAPI::Unit *unit, double bid);
	};
}