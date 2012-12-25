#pragma once

#include <map>

#include <BWAPI.h>
#include <RectangleArray.h>

void log(const char* text, ...);
std::map<int, int> computeAssignments(Util::RectangleArray< double> &cost);

namespace utils
{
	bool mineralCompare(const std::pair<BWAPI::Unit*, int> i, const std::pair<BWAPI::Unit*, int> j);
}