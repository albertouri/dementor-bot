#pragma once

#include <map>

#include <BWAPI.h>

#include "VultureAgent.h"

class VulturesManager
{
private:
	std::map<BWAPI::Unit*, VultureAgent*>::iterator agentsIterator;
	std::map<BWAPI::Unit*, VultureAgent*> agents;

	std::set<BWAPI::Position> beacons;
	std::set<BWAPI::Position>::iterator beaconsIterator;
public:
	void update();
	void addVulture(BWAPI::Unit *vulture);
	void removeVulture(BWAPI::Unit *vulture);

	BWAPI::Position getNearestBeacon(BWAPI::Unit *vulture);
	void addBeacon(BWAPI::Position beacon);
};