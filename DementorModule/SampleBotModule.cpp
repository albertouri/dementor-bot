#include "SampleBotModule.h"

#include <BWAPI.h>
#include <BWSAL.h>

#include <EnhancedUI.h>
#include <UnitGroupManager.h>

void SampleBotModule::onStart()
{
	BWAPI::Broodwar->sendText("Hello SampleBotModule!");
	BWAPI::Broodwar->enableFlag(BWAPI::Flag::UserInput);

	enhancedUI = new EnhancedUI();
	unitGroupManager = UnitGroupManager::create();
}

void SampleBotModule::onFrame()
{
	arbitrator.update();

	enhancedUI->update();
}

void SampleBotModule::onUnitDiscover(BWAPI::Unit *unit)
{
	unitGroupManager->onUnitDiscover(unit);
}

void SampleBotModule::onUnitEvade(BWAPI::Unit *unit)
{
	unitGroupManager->onUnitEvade(unit);
}

void SampleBotModule::onUnitMorph(BWAPI::Unit *unit)
{
	unitGroupManager->onUnitMorph(unit);
}

void SampleBotModule::onUnitRenegade(BWAPI::Unit *unit)
{
	unitGroupManager->onUnitRenegade(unit);
} 