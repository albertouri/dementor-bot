#pragma once

#include <BWAPI.h>
#include <BWSAL.h>

#include <EnhancedUI.h>
#include <UnitGroupManager.h>

class SampleBotModule : public BWAPI::AIModule
{
private:
	Arbitrator::Arbitrator<BWAPI::Unit*, double> arbitrator;

	EnhancedUI *enhancedUI;
	UnitGroupManager *unitGroupManager;
public:
	virtual void onStart();
	virtual void onFrame();
	virtual void onUnitDiscover(BWAPI::Unit* unit);
    virtual void onUnitEvade(BWAPI::Unit* unit);
    virtual void onUnitMorph(BWAPI::Unit* unit);
    virtual void onUnitRenegade(BWAPI::Unit* unit);
};