#include <ConstructionManager.h>
#include <UnitGroupManager.h>

ConstructionManager::ConstructionManager(Arbitrator::Arbitrator<BWAPI::Unit*, double>* arbitrator, BuildingPlacer* placer)
{
	this->arbitrator = arbitrator;
	this->placer = placer;

	const std::set<BWAPI::UnitType> unitTypes = BWAPI::UnitTypes::allUnitTypes();
	std::set<BWAPI::UnitType>::const_iterator it;
	for(it = unitTypes.begin(); it != unitTypes.end(); it++)
	{
		BWAPI::UnitType unitType = (*it);

		plannedCount[unitType] = 0;
		startedCount[unitType] = 0;
	}
}

void ConstructionManager::onOffer(std::set<BWAPI::Unit*> units)
{
  //we are being offered some units (hopefully builders).
  //accept want we need and decline the rest

  //go through all the buildings that are asking for builders
  for(std::map<BWAPI::UnitType,std::set<Building*> >::iterator i=this->buildingsNeedingBuilders.begin();i!=this->buildingsNeedingBuilders.end();i++)
  {
    std::set<Building*>::iterator b_next;
    //go through all the buildings that are asking for builders
    for(std::set<Building*>::iterator b = i->second.begin(); b != i->second.end(); b=b_next)
    {
      b_next=b;
      b_next++;
      double min_dist = 1000000;
      BWAPI::Unit* builder = NULL;
      //find the best builder based on distance
      for(std::set<BWAPI::Unit*>::iterator u = units.begin(); u != units.end(); u++)
      {
        //only consider this builder if it can build this type of building
        if (((*b)->type.whatBuilds().first)==(*u)->getType() && (!(*b)->type.isAddon() || (*u)->getAddon()==NULL))
        {
          double dist = (*u)->getPosition().getDistance((*b)->position);
          if (dist < min_dist)
          {
            min_dist = dist;
            builder  = *u;
          }
        }
      }

      //if we found a builder for this building
      if (builder != NULL)
      {
        //tell the arbitrator we accept the unit, and raise the bid to hopefully prevent other managers from using it
        arbitrator->accept(this, builder);
        arbitrator->setBid(this, builder, 100.0);

        //add builder to builders map
        builders.insert(std::make_pair(builder, *b));

        //also tell the building it has this builder
        (*b)->builderUnit = builder;

        //if this is the addon, we can also set the tile position and reserve the build site
        if ((*b)->type.isAddon())
        {
          (*b)->tilePosition = builder->getTilePosition();
          if (builder->isLifted())
          {
            if (!this->placer->canBuildHereWithSpace((*b)->tilePosition,(*b)->type))
              (*b)->tilePosition=placer->getBuildLocationNear((*b)->tilePosition,(*b)->type.whatBuilds().first);
          }
          else
          {
            bool buildable=true;
            for(int x=(*b)->tilePosition.x()+4;x<(*b)->tilePosition.x()+6;x++)
              for(int y=(*b)->tilePosition.y()+1;y<(*b)->tilePosition.y()+3;y++)
                if (!this->placer->buildable(x,y))
                  buildable=false;
            if (!buildable)
            {
              this->placer->freeTiles((*b)->tilePosition, 4,3);
              (*b)->tilePosition=placer->getBuildLocationNear((*b)->tilePosition,(*b)->type.whatBuilds().first);
            }
          }
          this->placer->reserveTiles((*b)->tilePosition, 4,3);
          this->placer->reserveTiles((*b)->tilePosition+BWAPI::TilePosition(4,1), 2,2);
        }

        //remove builder from the units set
        units.erase(builder);

        //erase this building from the set of buildings that need builders
        i->second.erase(b);
      }
    }
  }
  //decline whatever is left
  for(std::set<BWAPI::Unit*>::iterator u = units.begin(); u != units.end(); u++)
  {
    arbitrator->decline(this, *u, 0);
    arbitrator->removeBid(this, *u);
  }
}

void ConstructionManager::onRevoke(BWAPI::Unit* unit, double bid)
{
  this->onRemoveUnit(unit);
}

void ConstructionManager::update()
{
  std::set<BWAPI::Unit*> myPlayerUnits = SelectAll()(isCompleted)(GetAddon,(BWAPI::Unit*)NULL).not(isCarryingMinerals,isCarryingGas,isGatheringGas);

  //iterate through all the builder types
  for(std::map<BWAPI::UnitType,std::set<Building*> >::iterator i=this->buildingsNeedingBuilders.begin();i!=this->buildingsNeedingBuilders.end();i++)
  {
    if (!i->second.empty()) //if its empty, we don't really need builders of this type
    {
      for(std::set<BWAPI::Unit*>::iterator u = myPlayerUnits.begin(); u != myPlayerUnits.end(); u++)
      {
        //if this unit is completed and the right type, and doesn't have an addon, and we aren't already using it
        if ((*u)->getType()==i->first && this->builders.find(*u)==this->builders.end())
        {
          //bid value depends on distance - we like closer builders better
          double min_dist=1000000;
          for(std::set<Building*>::iterator b = i->second.begin(); b != i->second.end(); b++)
          {
            double dist = (*u)->getPosition().getDistance((*b)->position);
            if (dist < min_dist)
              min_dist = dist;
          }
          if (min_dist < 10)
            min_dist = 10;
          if (min_dist > 256*32 + 10)
            min_dist = 256*32 + 10;
          double bid = 80 - (min_dist - 10)/(256*32)*60;
          //if this isn't a worker, we don't care about distance
          if (!(*u)->getType().isWorker())
            arbitrator->setBid(this, *u, 80);
          else
            arbitrator->setBid(this, *u, bid);
        }
      }
    }
  }
  std::list<Building>::iterator i_next;
  int index=0;

  //iterate through all the incomplete buildings
  for(std::list<Building>::iterator i=this->incompleteBuildings.begin();i!=this->incompleteBuildings.end();i=i_next)
  {
    index++;
    i_next=i;
    i_next++;
    Building* b = &(*i);
    if (!b->started && b->buildingUnit!=NULL) //buildingUnit exists, so we've started actual construction
    {
      startedCount[b->type]++;
      b->started=true;
    }
    if (b->type.isAddon()) //if the type is an addon
    {
      if (b->builderUnit!=NULL)
        b->buildingUnit=b->builderUnit->getAddon(); //set buildingUnit to the addon (whether the addon exists or not)

      BWAPI::Unit* u = b->builderUnit;
      BWAPI::Unit* s = b->buildingUnit;
      if (s!=NULL && s->isCompleted()) //if the building exists and is completed
      {
        startedCount[b->type]--;
        plannedCount[b->type]--;
        if (u != NULL)
        {
          this->builders.erase(u);
          arbitrator->removeBid(this,u);
        }
        this->placer->freeTiles(b->tilePosition, 4,3);
        this->placer->freeTiles(b->tilePosition+BWAPI::TilePosition(4,1), 2,2);

        //If the building is complete, we can forget about it.
        this->incompleteBuildings.erase(i);
      }
      else //building either does not exist, or is incomplete
      {
        if (BWAPI::Broodwar->canMake(NULL,b->type))
        {
          if (u == NULL) //if we don't have a builder, ask for one and wait for it to be offered
            buildingsNeedingBuilders[b->type.whatBuilds().first].insert(b);
          else
          {
            if (u->getAddon() == NULL) //if the addon does not exist yet
            {
              if (u->isLifted()) //if the builder is lifted, find somehwere to land
              {
                if (BWAPI::Broodwar->getFrameCount()>b->lastOrderFrame+BWAPI::Broodwar->getLatency()*2)
                {
                  if (!this->placer->canBuildHereWithSpace(b->tilePosition,b->type))
                  {
                    this->placer->freeTiles(b->tilePosition, 4,3);
                    this->placer->freeTiles(b->tilePosition+BWAPI::TilePosition(4,1), 2,2);
                    b->tilePosition=placer->getBuildLocationNear(b->tilePosition,b->type.whatBuilds().first);
                    if (b->tilePosition == BWAPI::TilePositions::None)
                      b->tilePosition=placer->getBuildLocationNear(b->tilePosition,b->type.whatBuilds().first,0);
                    this->placer->reserveTiles(b->tilePosition, 4,3);
                    this->placer->reserveTiles(b->tilePosition+BWAPI::TilePosition(4,1), 2,2);
                  }
                  u->land(b->tilePosition);
                  b->lastOrderFrame = BWAPI::Broodwar->getFrameCount();
                }
              }
              else
              {
                if (u->isTraining()) //if the builder is training, cancel it - we have control over the builder
                {
                  u->cancelTrain();
                }
                else
                {
                  if (u->getTilePosition()!=b->tilePosition) //we may need to lift to find a build site with enough space for an add-on
                  { //b->tilePosition is set in onOffer()
                    //u->lift(); // no lifting
                    b->lastOrderFrame = BWAPI::Broodwar->getFrameCount();
                  }
                  else //otherwise, we may be in a good spot to make the add-on
                  {
                    //check to see if we can build here
                    bool buildable=true;
                    for(int x=b->tilePosition.x()+4;x<b->tilePosition.x()+6;x++)
                      for(int y=b->tilePosition.y()+1;y<b->tilePosition.y()+3;y++)
                        if (!this->placer->buildable(x,y) || BWAPI::Broodwar->hasCreep(x,y))
                          buildable=false;
                    if (buildable) //if so, start making the add-on
                    {
                      u->buildAddon(b->type);
                    }
                    else
                    {
                      //otherwise, lift up (once we are lifted we will look for a valid spot to land and build)
                      //u->lift(); // no lifting
                      b->lastOrderFrame = BWAPI::Broodwar->getFrameCount();
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
    else //if the type is a normal building (not an addon)
    {
      if (b->tilePosition==BWAPI::TilePositions::None) //if we don't have a build location yet
      {
        if ((BWAPI::Broodwar->getFrameCount()+index)%25==0)
        {
          //get a build location near the goal position
          b->tilePosition = this->placer->getBuildLocationNear(b->goalPosition, b->type);
          if (b->tilePosition!=BWAPI::TilePositions::None)
          {
            b->position = BWAPI::Position(b->tilePosition.x()*32 + b->type.tileWidth()*16, b->tilePosition.y()*32 + b->type.tileHeight()*16);
            this->placer->reserveTiles(b->tilePosition, b->type.tileWidth(), b->type.tileHeight());
          }
        }
        //if we didn't find a build location yet, or cannot make the building, skip this building for now
        if (b->tilePosition==BWAPI::TilePositions::None)
          continue;
      }

      //if the builder dies, set it to null
      if (b->builderUnit!=NULL && !b->builderUnit->exists())
        b->builderUnit=NULL;

      //if the building dies, or isn't the right type, set it to null
      if (b->buildingUnit!=NULL && (!b->buildingUnit->exists() || b->buildingUnit->getType()!=b->type))
        b->buildingUnit=NULL;

      if (b->buildingUnit == NULL) //if we don't have a building yet, look for it
      {
        //look at the units on the tile to see if it exists yet
        std::set<BWAPI::Unit*> unitsOnTile = BWAPI::Broodwar->getUnitsOnTile(b->tilePosition.x(), b->tilePosition.y());
        for(std::set<BWAPI::Unit*>::iterator t = unitsOnTile.begin(); t != unitsOnTile.end(); t++)
          if ((*t)->getType() == b->type && !(*t)->isLifted())
          {
            //we found the building
            b->buildingUnit = *t;
            break;
          }
        //maybe the builder _is_ the building! (Zerg)
        if (b->buildingUnit == NULL && b->builderUnit!=NULL && b->builderUnit->getType().isBuilding())
        {
          //we found the building
          b->buildingUnit = b->builderUnit;
        }
      }
      BWAPI::Unit* u = b->builderUnit;
      BWAPI::Unit* s = b->buildingUnit;
      if (s != NULL && s->isCompleted()) //if the building is completed, we're done
      {
        startedCount[b->type]--;
        plannedCount[b->type]--;
        if (u != NULL)
        {
          this->builders.erase(u);
          arbitrator->removeBid(this,u);
        }
        this->placer->freeTiles(b->tilePosition, b->type.tileWidth(), b->type.tileHeight());

        //If the building is complete, we can forget about it.
        this->incompleteBuildings.erase(i);
      }
      else //otherwise, it doesn't exist or is incomplete
      {
        if (s == NULL) //if the building doesn't even exist
        {
          if (u == NULL) //ask for a builder if we don't have one yet
            buildingsNeedingBuilders[b->type.whatBuilds().first].insert(b);
          else //if we have a worker
          {
            if (!u->isConstructing()) //if the worker isn't constructing
            {
              double distance = u->getPosition().getDistance(b->position);
              if (distance > 100 && u->getLastCommandFrame() + 4 < BWAPI::Broodwar->getFrameCount() && u->getOrder() != BWAPI::Orders::Move ) //if its too far away, tell it to go to the build site
                u->rightClick(b->position);
              else //if its close enough, tell it to build
                if (BWAPI::Broodwar->canBuildHere(u, b->tilePosition, b->type)) //if we can build here, tell the worker to build
                {
                  if (BWAPI::Broodwar->canMake(u, b->type))
                    u->build(b->tilePosition, b->type);
                }
                else //if we cannot build here, we need to find another build site (reset the tilePosition)
                {
                  this->placer->freeTiles(b->tilePosition, b->type.tileWidth(), b->type.tileHeight());
                  b->tilePosition = BWAPI::TilePositions::None;
                  b->position = BWAPI::Positions::None;
                }              
            }
          }
        }
        else //if the building does exist
        {
          if (b->type.getRace() != BWAPI::Races::Terran) //if its not terran, we can forget about the builder (i.e. probe is not needed to finish construction)
          {
            if (u != NULL)
            {
              this->builders.erase(u);
              arbitrator->removeBid(this, u);
              b->builderUnit = NULL;  
              u = b->builderUnit;
            }
          }
          else
          {
            //if the buildind is terran, the worker may have been killed
            if (u == NULL) //looks like the worker was killed, or revoked. In either case we need to ask for another worker to finish our building
              buildingsNeedingBuilders[b->type.whatBuilds().first].insert(b);
            else
            {
              //we have a worker, so lets rightClick it on the incomplete building so it can resume construction
              if (BWAPI::Broodwar->getFrameCount()%(4*BWAPI::Broodwar->getLatency())==0)
              {
                if ( !u->isConstructing() || !s->isBeingConstructed() )
                {
                  //right click builder on building
                  u->rightClick(s);
                }
              }
            }
          }
        }
      }
    }
  }
}

std::string ConstructionManager::getName() const
{
  return "Construction Manager";
}

std::string ConstructionManager::getShortName() const
{
  return "Con";
}

void ConstructionManager::onRemoveUnit(BWAPI::Unit* unit)
{
  //remove the builder if needed - incomplete buildings will ask for a new builder during update()
  if (builders.find(unit) != builders.end())
  {
    Building* building = builders.find(unit)->second;
    building->builderUnit = NULL;
    builders.erase(unit);
  }
  else
  {
    for(std::list<Building>::iterator i=incompleteBuildings.begin();i!=incompleteBuildings.end();i++)
    {
      if (unit==i->buildingUnit)
      {
        i->buildingUnit=NULL;
      }
    }
  }
}

bool ConstructionManager::build(BWAPI::UnitType type, BWAPI::TilePosition goalPosition)
{
  //build order starts here
  if (!type.isBuilding()) return false; //we only accept buildings
  Building newBuilding;
  newBuilding.type           = type;
  newBuilding.goalPosition   = goalPosition;
  newBuilding.tilePosition   = BWAPI::TilePositions::None;
  newBuilding.builderUnit    = NULL; //builder not picked yet
  newBuilding.buildingUnit   = NULL; //building does not exist yet
  newBuilding.position       = BWAPI::Positions::None; //no position yet
  newBuilding.lastOrderFrame = 0;
  newBuilding.started        = false; //not started yet
  plannedCount[type]++; //increment planned count for this type of unit
  this->incompleteBuildings.push_back(newBuilding);
  return true;
}

int ConstructionManager::getPlannedCount(BWAPI::UnitType type) const
{
  std::map<BWAPI::UnitType,int>::const_iterator i=plannedCount.find(type);
  if (i!=plannedCount.end())
    return i->second;
  return 0;
}

int ConstructionManager::getStartedCount(BWAPI::UnitType type) const
{
  std::map<BWAPI::UnitType,int>::const_iterator i=startedCount.find(type);
  if (i!=startedCount.end())
    return i->second;
  return 0;
}