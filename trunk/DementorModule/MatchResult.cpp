#include "MatchResult.h"

#include <sstream>
#include <fstream>

#include <ctime>
#include <cstdlib>

#include <BWAPI.h>

namespace dementor
{
	MatchResult::MatchResult(BWAPI::Game *game, bool isBotWinner)
	:
		game(game),
		isBotWinner(isBotWinner)
	{
	}

	void MatchResult::save(std::string destination)
	{
		std::fstream filestr;

		filestr.open(destination.c_str(), std::fstream::in | std::fstream::out | std::fstream::app);

		time_t rawtime;
		struct tm *timeinfo;

		time (&rawtime);
		timeinfo = localtime(&rawtime);

		filestr << "Match ended at: " << asctime(timeinfo) << std::endl;

		std::set<BWAPI::Player*>::iterator i;
		std::set<BWAPI::Player*> players = game->getPlayers();

		for (i = players.begin(); i != players.end(); i++)
		{
			BWAPI::Player *player = (*i);

			BWAPI::PlayerType type = player->getType();

			if (type == BWAPI::PlayerTypes::Player || type == BWAPI::PlayerTypes::Computer)
			{
				continue;
			}

			filestr << "Player: " <<  player->getName() << "(" << player->getRace().getName() << ")" << std::endl;
		}

		filestr << "Wins? " << ((isBotWinner) ? "Yes" : "No") << std::endl;
		filestr << std::endl;

		filestr.close();
	}
}