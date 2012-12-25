#pragma once

#include <map>

#include <BWAPI.h>
#include <BWSAL.h>

#include <Arbitrator/Controller.h>
#include <WorkerManager.h>
#include <UnitGroupManager.h>
#include <BaseManager.h>
#include <BuildOrderManager.h>
#include <TechManager.h>
#include <UpgradeManager.h>
#include <BuildManager.h>
#include <SupplyManager.h>
#include <BorderManager.h>
#include <InformationManager.h>
#include <EnhancedUI.h>

#include "ManagerTypes.h"

#include "Commander.h"
#include "AgentManager.h"
#include "MapStateManager.h"
#include "BaseGuardManager.h"
#include "RepairManager.h"

namespace dementor
{
	class Commander;
	class AgentManager;
	class MapStateManager;
	class BaseGuardManager;
	class RepairManager;

	class AIModule : public BWAPI::AIModule
	{
	private:
		BWAPI::Game *game;

		bool debug;
		bool autocam;
		bool gui;

		int localSpeed;
		int frameSkip;

		bool saveMatchResult;

		Arbitrator::Arbitrator<BWAPI::Unit*, double> arbitrator;

		std::map<ManagerTypes, void*> controllers;

		WorkerManager *workerManager;
		BaseManager *baseManager;
		BuildManager *buildManager;
		TechManager *techManager;
		UpgradeManager *upgradeManager;
		SupplyManager *supplyManager;
		BuildOrderManager *buildOrderManager;

		UnitGroupManager *unitGroupManager;
		BorderManager *borderManager;

		AgentManager *agentManager;
		MapStateManager *mapStateManager;
		BaseGuardManager *baseGuardManager;
		RepairManager *repairManager;

		Commander *commander;

		void executeStrategy();
		void camera();

		void drawDebug();

		void registerController(ManagerTypes type, void *controller);

		void saveGameResult(bool isBotWinner);

		void updateGameSpeed();
	public:
		AIModule(BWAPI::Game *game);

		BWAPI::Game* getGame();

		void* getController(ManagerTypes type);
		Arbitrator::Arbitrator<BWAPI::Unit*, double> *getArbitrator();
		Commander *getCommander();

		virtual void onStart();
		virtual void onEnd(bool isWinner);
		virtual void onFrame();
		virtual void onSendText(std::string text);
		//virtual void onReceiveText(BWAPI::Player* player, std::string text);
		//virtual void onPlayerLeft(BWAPI::Player* player);
		//virtual void onNukeDetect(BWAPI::Position target);
		virtual void onUnitDiscover(BWAPI::Unit* unit);
		virtual void onUnitEvade(BWAPI::Unit* unit);
		//virtual void onUnitShow(BWAPI::Unit* unit);
		//virtual void onUnitHide(BWAPI::Unit* unit);
		virtual void onUnitCreate(BWAPI::Unit* unit);
		virtual void onUnitDestroy(BWAPI::Unit* unit);
		virtual void onUnitMorph(BWAPI::Unit* unit);
		virtual void onUnitRenegade(BWAPI::Unit* unit);
		//virtual void onSaveGame(std::string gameName);
	};
}