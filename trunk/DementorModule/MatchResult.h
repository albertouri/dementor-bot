#pragma once

#include <BWAPI.h>

namespace dementor
{
	class MatchResult
	{
	private:
		BWAPI::Game *game;
		bool isBotWinner;
	public:
		MatchResult(BWAPI::Game *game, bool isBotWinner);
		void save(std::string destination);
	};
}