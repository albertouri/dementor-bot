#include "AIModule.h"

#include "ManagerTypes.h"

#include "Commander.h"

#include "MatchResult.h"

namespace dementor
{
	AIModule::AIModule(BWAPI::Game *game)
	:
		game(game)
	{
	}

		BWAPI::Game *AIModule::getGame()
	{
		return game;
	}

	void AIModule::registerController(ManagerTypes type, void *controller)
	{
		controllers.insert(std::pair<ManagerTypes, void*>(type, controller));
	}

	void* AIModule::getController(ManagerTypes type)
	{
		std::map<ManagerTypes, void*>::iterator it;

		it = controllers.find(type);

		if (it == controllers.end())
		{
			return NULL;
		}

		return it->second;
	}

	Arbitrator::Arbitrator<BWAPI::Unit*, double> *AIModule::getArbitrator()
	{
		return &arbitrator;
	}

	Commander *AIModule::getCommander()
	{
		return commander;
	}

	void AIModule::camera()
	{
		if (game->getMouseState(BWAPI::M_LEFT))
		{
			return;
		}

		const int CENTER_SCREEN_X = 320;
		const int CENTER_SCREEN_Y = 140;

		if (!autocam)
		{
			return;
		}

		UnitGroup interestingUnits = AllUnits()(isAttacking);

		if (interestingUnits.size() == 0)
		{
			return;
		}

		BWAPI::Unit *unit = (*interestingUnits.begin());

		BWAPI::Position unitPosition = unit->getPosition();

		int x, y;
		x = unitPosition.x() - CENTER_SCREEN_X;
		y = unitPosition.y() - CENTER_SCREEN_Y;

		if (x < 0)
		{
			x = 0;
		}

		if (y < 0)
		{
			y = 0;
		}

		BWAPI::Broodwar->setScreenPosition(x, y);
	}

	void AIModule::drawDebug()
	{
		mapStateManager->draw();
		repairManager->draw();
	}

	void AIModule::onStart()
	{
		// some game, debug and info settings
		debug = false;
		autocam = true;
		gui = true;

		saveMatchResult = false;

		// some informations on start
		game->printf("Dementor bot is alive");

		std::string mapName = game->mapName();
		std::set<BWAPI::TilePosition> startLocations = game->getStartLocations();
		game->printf("The map is %s, a %d player map", mapName.c_str(), startLocations.size());

		game->printf("Enabled UserInput flag.");
		game->enableFlag(BWAPI::Flag::UserInput);

		onSendText("gsfs");

		// reading and analyzing map
		BWTA::readMap();
		BWTA::analyze();

		// managers creation and registering
		workerManager = new WorkerManager(&arbitrator);
		registerController(WorkerManagerType, workerManager);

		buildManager = new BuildManager(&arbitrator);
		registerController(BuildManagerType, buildManager);

		techManager = new TechManager(&arbitrator);
		registerController(TechManagerType, techManager);

		upgradeManager = new UpgradeManager(&arbitrator);
		registerController(UpgradeManagerType, upgradeManager);

		supplyManager = new SupplyManager();
		registerController(SupplyManagerType, supplyManager);

		baseManager = new BaseManager();
		registerController(BaseManagerType, baseManager);

		buildOrderManager = new BuildOrderManager(buildManager, techManager, upgradeManager, workerManager, supplyManager);
		registerController(BuildOrderManagerType, buildOrderManager);

		agentManager = new AgentManager(this);
		registerController(AgentManagerType, agentManager);

		repairManager = new RepairManager(this);
		registerController(RepairManagerType, repairManager);

		BWAPI::Position startPosition = BWAPI::Position(game->self()->getStartLocation());
		baseGuardManager = new BaseGuardManager(this, startPosition);
		registerController(BaseGuardManagerType, baseGuardManager);

		mapStateManager = new MapStateManager(this);
		registerController(MapStateManagerType, mapStateManager);

		unitGroupManager = UnitGroupManager::create();
		registerController(UnitGroupManagerType, unitGroupManager);

		borderManager = BorderManager::create();
		registerController(BorderManagerType, borderManager);

		InformationManager::create();
		
		// setting managers parameters
		workerManager->enableAutoBuild();
		workerManager->setBaseManager(baseManager);
		workerManager->setBuildOrderManager(buildOrderManager);

		supplyManager->setBuildManager(buildManager);
		supplyManager->setBuildOrderManager(buildOrderManager);

		techManager->setBuildingPlacer(buildManager->getBuildingPlacer());
		upgradeManager->setBuildingPlacer(buildManager->getBuildingPlacer());

		baseManager->setBuildOrderManager(buildOrderManager);
		baseManager->setBorderManager(borderManager);

		buildOrderManager->setDebugMode(debug);

		baseManager->setRefineryBuildPriority(50);

		// creating commander
		commander = new Commander(this);
	}

	void AIModule::onEnd(bool isWinner)
	{
		if (saveMatchResult)
		{
			saveGameResult(isWinner);
		}
	}

	void AIModule::saveGameResult(bool isBotWinner)
	{
		MatchResult result = MatchResult(game, isBotWinner);
		result.save("result.txt");
	}

	void AIModule::onFrame()
	{
		workerManager->update();
		buildManager->update();
		buildOrderManager->update();
		baseManager->update();
		techManager->update();
		upgradeManager->update();
		supplyManager->update();
		borderManager->update();

		agentManager->update();
		baseGuardManager->update();
		mapStateManager->update();
		repairManager->update();

		commander->update();

		arbitrator.update();

		camera();

		drawDebug();
	}

	void AIModule::onUnitCreate(BWAPI::Unit *unit)
	{
		agentManager->onUnitCreate(unit);
	}

	void AIModule::onUnitDestroy(BWAPI::Unit *unit)
	{
		arbitrator.onRemoveObject(unit);

		workerManager->onRemoveUnit(unit);
		buildManager->onRemoveUnit(unit);
		techManager->onRemoveUnit(unit);
		upgradeManager->onRemoveUnit(unit);
		baseManager->onRemoveUnit(unit);

		agentManager->onUnitDestroy(unit);
		repairManager->onUnitDestroy(unit);
	}

	void AIModule::onUnitDiscover(BWAPI::Unit* unit)
	{
		unitGroupManager->onUnitDiscover(unit);
		mapStateManager->onUnitDiscover(unit);
	}

	void AIModule::onUnitEvade(BWAPI::Unit* unit)
	{
		unitGroupManager->onUnitEvade(unit);
	}

	void AIModule::onUnitMorph(BWAPI::Unit* unit)
	{
		unitGroupManager->onUnitMorph(unit);
	}
	void AIModule::onUnitRenegade(BWAPI::Unit* unit)
	{
		unitGroupManager->onUnitRenegade(unit);
	}

	void AIModule::onSendText(std::string text)
	{
		game->printf(text.c_str());

		if (text == "dbg")
		{
			debug = !debug;

			buildOrderManager->setDebugMode(debug);

			game->printf("Debug turned %s.", (debug) ? "on" : "off");
		}

		if (text == "atc")
		{
			autocam = !autocam;

			game->printf("Autocam turned %s.", (autocam) ? "on" : "off");
		}

		if (text == "gsn")
		{
			localSpeed = 30;
			frameSkip = 0;

			updateGameSpeed();
		}

		if (text == "gsf")
		{
			localSpeed = 0;
			frameSkip = 0;

			updateGameSpeed();
		}

		if (text == "gsfs")
		{
			localSpeed = 0;
			frameSkip = 8;

			updateGameSpeed();
		}

		if (text == "atk")
		{
			BWAPI::Position mousePosition = game->getMousePosition();
			BWAPI::Position screenPosition = game->getScreenPosition();

			BWAPI::Position mapPosition = BWAPI::Position(mousePosition.x() + screenPosition.x(), mousePosition.y() + screenPosition.y());
			agentManager->attackMoveTo(mapPosition);

			game->printf("Issued manual attack at %d %d.", mapPosition.x(), mapPosition.y());
		}
	}

	void AIModule::updateGameSpeed()
	{
		game->printf("GUI %s", (gui) ? "enabled" : "disabled");
		game->setGUI(gui);

		game->printf("Local speed set to %d", localSpeed);
		game->setLocalSpeed(localSpeed);

		game->printf("Frameskip set to %d", frameSkip);
		game->setFrameSkip(frameSkip);
	}
}