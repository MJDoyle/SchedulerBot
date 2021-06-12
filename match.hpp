//Contains all the information about a match

#pragma once

#include "config.hpp"

class Match
{
public:

	Match(int time, int day, int month, int year, std::string info) 
	{
		_time = time;
		_day = day;
		_month = month;
		_year = year;
		_info = info;

		_hourWarningSent = false;

		_5minWarningSent = false;
	}

	int GetTime() { return _time; }
	int GetDay() { return _day; }
	int GetMonth() { return _month; }
	int GetYear() { return _year; }
	std::string GetInfo() { return _info; }

	bool GetHourWarningSent() { return _hourWarningSent; }
	bool Get5MinWarningSent() { return _5minWarningSent; }

	void SetHourWarningSent(bool hourWarningSent) { _hourWarningSent = hourWarningSent; }
	void Set5MinWarningSent(bool fiveMinWarningSent) { _5minWarningSent = fiveMinWarningSent; }


private:

	bool _hourWarningSent;

	bool _5minWarningSent;

	int _time;

	int _day;

	int _month;

	int _year;

	std::string _info;

};