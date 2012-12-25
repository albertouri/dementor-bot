#include "AgentManager.h"

#include <assert.h>

#include "AIModule.h"
#include "TankAgent.h"
#include "VultureAgent.h"
#include "GroundScoutAgent.h"
#include "SimpleAttackAgent.h"

#include "MapStateManager.h"

namespace dementor {

	AgentManager::AgentManager(AIModule *bot)
	:
		bot(bot),
		arbitrator(bot->getArbitrator())
	{
	}

	AgentManager::~AgentManager(void)
	{
	}

	std::string AgentManager::getName() const
	{
		return "Agent Manager";
	}

	std::string AgentManager::getShortName() const
	{
		return "AgMan";
	}

	void AgentManager::onUnitCreate(BWAPI::Unit *unit)
	{
		BWAPI::UnitType type = unit->getType();

		if (
			type == BWAPI::UnitTypes::Terran_Vulture
			||
			type == BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode
			||
			type == BWAPI::UnitTypes::Terran_Marine
			||
			type == BWAPI::UnitTypes::Terran_Wraith
			||
			type == BWAPI::UnitTypes::Terran_Goliath
			)
		{
			arbitrator->setBid(this, unit, 100);
		}
	}

	void AgentManager::onUnitDestroy(BWAPI::Unit *unit)
	{
		std::map<BWAPI::Unit*, UnitAgent*>::iterator it;

		it = agents.find(unit);
		if (it == agents.end())
		{
			return;
		}

		UnitAgent *agent = (*it).second;

		destroyAgent(unit, agent);
	}

	void AgentManager::onOffer(std::set<BWAPI::Unit*> units)
	{
		std::set<BWAPI::Unit*>::iterator it;
		for (it = units.begin(); it != units.end(); it++)
		{
			BWAPI::Unit *unit = (*it);
			BWAPI::UnitType unitType = unit->getType();

			UnitAgent *agent = NULL;

			if (unitType == BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode)
			{
				agent = new TankAgent(unit);
			}
			else if (unitType == BWAPI::UnitTypes::Terran_Vulture)
			{
				if (scouts.size() < 2)
				{
					MapStateManager *mapStateManager = (MapStateManager*)bot->getController(MapStateManagerType);
					agent = new GroundScoutAgent(mapStateManager, unit);

					scouts.insert(agent);
				}
				else
				{
					agent = new VultureAgent(unit);
				}
			}
			else if (unitType == BWAPI::UnitTypes::Terran_Marine)
			{
				if (scouts.size() < 1 && SelectAll()(Marine).size() > 3)
				{
					MapStateManager *mapStateManager = (MapStateManager*)bot->getController(MapStateManagerType);
					agent = new GroundScoutAgent(mapStateManager, unit);

					scouts.insert(agent);
				}
			}
			else
			{
				if (unitType == BWAPI::UnitTypes::Terran_Wraith && scouts.size() < 3)
				{
					MapStateManager *mapStateManager = (MapStateManager*)bot->getController(MapStateManagerType);
					agent = new GroundScoutAgent(mapStateManager, unit);

					scouts.insert(agent);
				}
				else
				{
					agent = new SimpleAttackAgent(unit);
				}
			}

			if(agent != NULL)
			{
				arbitrator->accept(this, unit);
				agents.insert(std::pair<BWAPI::Unit*, UnitAgent*>(unit, agent));
			}
			else
			{
				arbitrator->decline(this, unit, 50);
			}
		}
	}
	
	void AgentManager::onRevoke(BWAPI::Unit *unit, double bid)
	{
		std::map<BWAPI::Unit*, UnitAgent*>::iterator it;

		it = agents.find(unit);

		if (it == agents.end())
		{
			// I was not owning this unit yet, but I want it before
			return;
		}

		// I have this unit and need to remove it and destroy its agent
		UnitAgent *agent = (*it).second;
		destroyAgent(unit, agent);
	}

	void AgentManager::destroyAgent(BWAPI::Unit *unit, UnitAgent *agent)
	{
		std::set<UnitAgent*>::iterator scoutsIt;
		scoutsIt = scouts.find(agent);
		if (scoutsIt != scouts.end())
		{
			destroyScoutAgent(agent);
		}

		std::map<BWAPI::Unit*, UnitAgent*>::iterator it = agents.find(unit);

		agents.erase(it);
		delete agent;

		arbitrator->removeBid(this, unit);
	}

	void AgentManager::destroyScoutAgent(UnitAgent *agent)
	{
		std::set<UnitAgent*>::iterator it;

		it = scouts.find(agent);

		assert(it != scouts.end());

		scouts.erase(it);

		bot->getGame()->printf("Scout agent destroyed.");
	}

	void AgentManager::update(void) {
		std::map<BWAPI::Unit*, UnitAgent*>::iterator it;

		for(it = this->agents.begin(); it != this->agents.end(); it++) {
			(*it).second->update();
		}

		draw();
	}

	void AgentManager::draw(void) {
		std::map<BWAPI::Unit*, UnitAgent*>::iterator it;

		for(it = this->agents.begin(); it != this->agents.end(); it++) {
			BWAPI::Unit *unit = (*it).first;

			if (unit->isSelected())
			{
				(*it).second->draw();
			}
		}

		BWAPI::Game *game = bot->getGame();

		game->drawTextScreen(10, 30, "Scouting agents: %d", scouts.size());
	}

	void AgentManager::moveTo(BWAPI::Position position)
	{
		std::map<BWAPI::Unit*, UnitAgent*>::iterator it;

		for(it = this->agents.begin(); it != this->agents.end(); it++)
		{
			// TODO
			// neskor spravit naozaj na iba move
			(*it).second->attackMoveTo(position);
		}
	}

	void AgentManager::attackMoveTo(BWAPI::Position position)
	{
		std::map<BWAPI::Unit*, UnitAgent*>::iterator it;

		for(it = this->agents.begin(); it != this->agents.end(); it++)
		{
			(*it).second->attackMoveTo(position);
		}
	}

	void AgentManager::attackUnit(BWAPI::Unit *unit)
	{
		std::map<BWAPI::Unit*, UnitAgent*>::iterator it;

		for(it = this->agents.begin(); it != this->agents.end(); it++)
		{
			(*it).second->attackUnit(unit);
		}
	}
}