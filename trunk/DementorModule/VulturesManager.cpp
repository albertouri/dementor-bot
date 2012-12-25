#include "VulturesManager.h"

#include "VultureAgent.h"

void VulturesManager::update()
{
	for (agentsIterator = agents.begin(); agentsIterator != agents.end(); agentsIterator++)
	{
		VultureAgent *agent = (*agentsIterator).second;

		agent->update();
	}
}

void VulturesManager::addVulture(BWAPI::Unit *vulture)
{
	agents.insert(std::pair<BWAPI::Unit*, VultureAgent*>(vulture, new VultureAgent(this, vulture)));
}

void VulturesManager::removeVulture(BWAPI::Unit *vulture)
{
	agentsIterator = agents.find(vulture);

	if (agentsIterator != agents.end())
	{
		agents.erase(vulture);
		delete agentsIterator->second;
	}
}


BWAPI::Position VulturesManager::getNearestBeacon(BWAPI::Unit* vulture)
{
	if (agents.size() < 10)
	{
		return BWAPI::Positions::None;
	}

	BWAPI::Position nearestBeacon = BWAPI::Positions::None;
	int shortestDistance = -1;

	for (beaconsIterator = beacons.begin(); beaconsIterator != beacons.end(); beaconsIterator++)
	{
		BWAPI::Position point = *beaconsIterator;

		int distance = vulture->getDistance(point);

		if (shortestDistance == -1 || distance < shortestDistance)
		{
			shortestDistance = distance;
			nearestBeacon = point;
		}
	}

	return nearestBeacon;
}

void VulturesManager::addBeacon(BWAPI::Position beacon)
{
	beacons.insert(beacon);
}