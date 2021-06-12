//SchedulerBot
//
//A simple Twitch bot that allows channels to schedule games.

#include "schedulerbot.hpp"

int main()
{
	//Set up new bot
	std::shared_ptr<SchedulerBot> bot = std::shared_ptr<SchedulerBot>(new SchedulerBot());

	//Connect to twich
	if (!bot->Connect())
		return 1;

	//Main loop
	while (true)
	{
		bot->Receive();

		bot->Update();
	}

	return 1;
}