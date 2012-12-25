#include "BuildOrderManager.h"
BuildOrderManager::MetaUnit::MetaUnit(BWAPI::Unit* unit)
{
  this->unit=unit;
}
BuildOrderManager::MetaUnit::MetaUnit(int larvaSpawnTime)
{
  this->unit=NULL;
  this->larvaSpawnTime=larvaSpawnTime;
}
int BuildOrderManager::MetaUnit::nextFreeTime() const
{
  return 0;
}
int BuildOrderManager::MetaUnit::nextFreeTime(BWAPI::UnitType t) const
{
  return 0;
}
int BuildOrderManager::MetaUnit::nextFreeTime(BWAPI::TechType t) const
{
  return 0;
}
int BuildOrderManager::MetaUnit::nextFreeTime(BWAPI::UpgradeType t) const
{
  return 0;
}
BWAPI::UnitType BuildOrderManager::MetaUnit::getType() const
{
  if (unit!=NULL)
    return unit->getType();
  return BWAPI::UnitTypes::Zerg_Larva;
}

int BuildOrderManager::MetaUnit::getRemainingBuildTime() const
{
  if (unit!=NULL)
    return unit->getRemainingBuildTime();
  int t=larvaSpawnTime-BWAPI::Broodwar->getFrameCount();
  if (t<0)
    return 0;
  return t;
}
int BuildOrderManager::MetaUnit::getRemainingTrainTime() const
{
  if (unit!=NULL)
    return unit->getRemainingTrainTime();
  int t=larvaSpawnTime-BWAPI::Broodwar->getFrameCount();
  if (t<0)
    return 0;
  return t;
}
int BuildOrderManager::MetaUnit::getRemainingResearchTime() const
{
  if (unit!=NULL)
    return unit->getRemainingResearchTime();
  return 0;
}
int BuildOrderManager::MetaUnit::getRemainingUpgradeTime() const
{
  if (unit!=NULL)
    return unit->getRemainingUpgradeTime();
  return 0;
}
BWAPI::UpgradeType BuildOrderManager::MetaUnit::getUpgrade() const
{
  if (unit!=NULL)
    return unit->getUpgrade();
  return BWAPI::UpgradeTypes::None;
}

bool BuildOrderManager::MetaUnit::hasBuildUnit() const
{
  if (unit!=NULL)
    return unit->getBuildUnit()!=NULL;
  return false;
}
bool BuildOrderManager::MetaUnit::hasAddon() const
{
  if (unit!=NULL)
    return unit->getAddon()!=NULL;
  return false;
}
bool BuildOrderManager::MetaUnit::isBeingConstructed() const
{
  if (unit!=NULL)
    return unit->isBeingConstructed();
  return true;
}
bool BuildOrderManager::MetaUnit::isCompleted() const
{
  if (unit!=NULL)
    return unit->isCompleted();
  return false;
}
bool BuildOrderManager::MetaUnit::isMorphing() const
{
  if (unit!=NULL)
    return unit->isMorphing();
  return false;
}
bool BuildOrderManager::MetaUnit::isTraining() const
{
  if (unit!=NULL)
    return unit->isTraining();
  return false;
}
bool BuildOrderManager::MetaUnit::isUpgrading() const
{
  if (unit!=NULL)
    return unit->isUpgrading();
  return false;
}