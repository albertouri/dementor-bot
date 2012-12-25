#pragma once
#include <Arbitrator.h>
#include <BWAPI.h>

#include "Squad.h"

class Squad;

class SquadManager : public Arbitrator::Controller<BWAPI::Unit*,double>
{
private:
	const static double BID;	

	static int freeId;

	std::set<Squad*> squads;

	bool debug;

	std::map<BWAPI::UnitType, int> composition;

	Arbitrator::Arbitrator<BWAPI::Unit*, double> *arbitrator;
	std::set<BWAPI::Unit*> freeUnits;

	void findAvailableUnits();
	void removeUnit(BWAPI::Unit *unit);

	void draw();
public:
	static int generateId();

	SquadManager(Arbitrator::Arbitrator<BWAPI::Unit*, double> *arbitrator);

	virtual void onOffer(std::set<BWAPI::Unit*> units);
	virtual void onRevoke(BWAPI::Unit *unit, double bid);

	void onRemoveUnit(BWAPI::Unit *unit);

	virtual std::string getName() const;
	virtual std::string getShortName() const;

	virtual void update();

	void addSquad(Squad *squad);
	void removeSquad(Squad *squad);
	std::set<Squad*> getSquads();

	BWAPI::Unit *requestUnit(BWAPI::UnitType unitType);

	void issueAttackAll(BWAPI::Position position);
	void issueMoveAll(BWAPI::Position position);

	void setDebug(bool debug);
};