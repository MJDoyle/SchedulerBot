//Channel class contains the full schedule for the channel, which is loaded from file when the bot starts up or is added via twitch command

#pragma once

#include "config.hpp"
#include "match.hpp"

class Channel
{
public:

	//Constructor
	Channel(std::string name)
	{
		_name = name;
	}

	//Name getter
	std::string GetName() { return _name; }

	//Return list of matches
	std::map<std::chrono::system_clock::time_point, std::shared_ptr<Match>> GetMatches() { return _matches; }

	//Add a new match directly from numbers taken from twitch chat command
	bool AddMatch(int time, int day, int month, int year, std::string info)
	{
		//Convert the command to a std::tm
		std::tm tm = { /* .tm_sec  = */ 0,
			/* .tm_min  = */ time % 100,
			/* .tm_hour = */ time / 100,
			/* .tm_mday = */ day,
			/* .tm_mon  = */ month - 1,
			/* .tm_year = */ year + 100,
		};

		//Set daylight saving time to not be used
		tm.tm_isdst = -1;

		//Make a time_point from the tm
		std::chrono::system_clock::time_point tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));

		//Check if there is already a match taking place at that time. If not, add match to map
		if (!_matches.count(tp))
		{
			_matches[tp] = std::shared_ptr<Match>(new Match(time, day, month, year, info));

			return true;
		}

		//If there is already a match at this time, return false
		return false;
	}

	//Erase match if it has already started (via time_point)
	void EraseMatch(std::chrono::system_clock::time_point timePoint)
	{
		_matches.erase(timePoint);
	}

	//Overload for erase match via numbers directly taken from twitch message
	void EraseMatch(int time, int day, int month, int year)
	{

		//Convert the command to a std::tm
		std::tm tm = { /* .tm_sec  = */ 0,
			/* .tm_min  = */ time % 100,
			/* .tm_hour = */ time / 100,
			/* .tm_mday = */ day,
			/* .tm_mon  = */ month - 1,
			/* .tm_year = */ year + 100,
		};

		//Set daylight saving time to not be used
		tm.tm_isdst = -1;

		//Make a time_point from the tm
		std::chrono::system_clock::time_point tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));

		//If there is a match at that time, remove it
		if (_matches.count(tp))
		{
			_matches.erase(tp);
		}

	}

private:

	//Channel name
	std::string _name;

	//Matches are stored in map, keyed by time_point
	std::map<std::chrono::system_clock::time_point, std::shared_ptr<Match>> _matches;
};