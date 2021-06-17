#include "schedulerbot.hpp"

#include "message.hpp"

#include "channel.hpp"

//Constructor
SchedulerBot::SchedulerBot() : _connected(false)
{
	//Create new TCP socket
	_socket = std::shared_ptr<sf::TcpSocket>(new sf::TcpSocket());

	//Load all information from files
	LoadBotDetails();

	LoadAdmin();

	LoadChannels();

	LoadSchedule();
}

//Load bot details from file (twitch name and Oauth code)
//Two lines to load
void SchedulerBot::LoadBotDetails()
{
	std::ifstream botFile;

	//Open bot file
	botFile.open("botdetails.txt");

	//Read the bot channel name
	std::getline(botFile, _botName);

	//Read the bot Oauth code
	std::getline(botFile, _botOauth);

	//Close bot file
	botFile.close();
}

//Load admin channel (string) from file
//The admin.txt file should just contain a single line, so just one line read needed
void SchedulerBot::LoadAdmin()
{
	std::ifstream adminFile;

	//Open admin file
	adminFile.open("admin.txt");

	//Read the line, and set it to _admin
	std::getline(adminFile, _admin);

	//Close the file
	adminFile.close();
}

//Load all channels to connect to from file
//Multiple channels possible, so multiple lines to read
void SchedulerBot::LoadChannels()
{
	std::ifstream channelFile;

	//Open channel file
	channelFile.open("channels.txt");

	std::string channel;

	//Read every available line and add the read line to the channels map, with the channel as t
	while (std::getline(channelFile, channel))
	{
		_channels[channel] = std::shared_ptr<Channel>(new Channel(channel));
	}

	//Close channel file
	channelFile.close();
}

//Load schedules from schedule file
//Schedule file contains games for all channels
//Is line is formatted as: channel, time, day, month, year, matchinfo 
void SchedulerBot::LoadSchedule()
{
	std::ifstream scheduleFile;

	//Open schedule file
	scheduleFile.open("schedule.txt");

	std::string match;

	//Read in line by line and extract required information from the line
	while (std::getline(scheduleFile, match))
	{

		int i = 0;

		std::string channel;

		bool channelFound = false;

		//Iterate through each character in the line until ',' is found, all characters up to this point are added to channel string
		while (i < match.size() && !channelFound)
		{
			if (match[i] == ',')
				channelFound = true;

			else
				channel.push_back(match[i]);

			i++;
		}


		std::string time;

		bool timeFound = false;

		//From previous ',' iterate through each character in the line until ',' is found, all characters up to this point are added to time string
		while (i < match.size() && !timeFound)
		{
			if (match[i] == ',')
				timeFound = true;

			else
				time.push_back(match[i]);

			i++;
		}


		std::string day;

		bool dayFound = false;

		//From previous ',' iterate through each character in the line until ',' is found, all characters up to this point are added to day string
		while (i < match.size() && !dayFound)
		{
			if (match[i] == ',')
				dayFound = true;

			else
				day.push_back(match[i]);

			i++;
		}


		std::string month;

		bool monthFound = false;

		//From previous ',' iterate through each character in the line until ',' is found, all characters up to this point are added to month string
		while (i < match.size() && !monthFound)
		{
			if (match[i] == ',')
				monthFound = true;

			else
				month.push_back(match[i]);

			i++;
		}


		std::string year;

		bool yearFound = false;

		//From previous ',' iterate through each character in the line until ',' is found, all characters up to this point are added to year string
		while (i < match.size() && !yearFound)
		{
			if (match[i] == ',')
				yearFound = true;

			else
				year.push_back(match[i]);

			i++;
		}


		std::string info;

		//From previous ',' add all remaining characters in the line to the info string
		while (i < match.size())
		{
			info.push_back(match[i]);

			i++;
		}

		//Check that the channel exists in the map, and add match if it does
		if (_channels.count(channel))
		{
			_channels[channel]->AddMatch(std::stoi(time), std::stoi(day), std::stoi(month), std::stoi(year), info);
		}
	}

	//Close the schedule file
	scheduleFile.close();
}

//Save all channel names to file
void SchedulerBot::SaveChannels()
{
	std::ofstream channelFile;

	//Open channel file and clear it
	channelFile.open("channels.txt", std::ofstream::out | std::ofstream::trunc);

	//Write each channel name
	for (auto channel = _channels.begin(); channel != _channels.end(); channel++)
		channelFile << channel->first << std::endl;

	//Close channel name
	channelFile.close();
}

//Save channel schedules
//All schedules go in the same file
void SchedulerBot::SaveSchedule()
{
	std::ofstream scheduleFile;

	//Open schedule file and clear it
	scheduleFile.open("schedule.txt", std::ofstream::out | std::ofstream::trunc);

	//Iterate through each channel, and get the map of matches
	for (auto channel = _channels.begin(); channel != _channels.end(); channel++)
	{
		std::map<std::chrono::system_clock::time_point, std::shared_ptr<Match>> matches = _channels[channel->first]->GetMatches();

		//Iterate through each match and write it to the file with the format: channel, time, day, month, year, matchinfo 
		for (auto match = matches.begin(); match != matches.end(); match++)
		{
			scheduleFile << channel->first << "," << match->second->GetTime() << "," << match->second->GetDay() << "," << match->second->GetMonth() << "," << match->second->GetYear() << "," << match->second->GetInfo() << std::endl;
		}
	}

	//Close schedule file
	scheduleFile.close();
}

//Connect to twitch IRC
bool SchedulerBot::Connect()
{
	//Attempt to connect with the socket to the twitch address
	if (_socket->connect("irc.chat.twitch.tv", 6667) != sf::Socket::Done)
		std::cout << "[Client] Failed to connect to server!\n";


	//Connect to twitch IRC using the bot oauth and nick

	std::stringstream passSS;

	passSS << "PASS " << _botOauth << "\r\n";

	Send(passSS.str());

	std::stringstream nickSS;

	nickSS << "NICK " << _botName << "\r\n";

	Send(nickSS.str());	//NOTE: The NICK IRC command is no-op, however needs to be sent

	//Make the appropriate requests for premissions

	Send("CAP REQ :twitch.tv/membership\r\n");

	Send("CAP REQ :twitch.tv/tags\r\n");

	Send("CAP REQ :twitch.tv/commands\r\n");


	sf::Clock timeoutClock;

	//Wait for confirmation of connection for up to three seconds
	while (!_connected && timeoutClock.getElapsedTime().asMilliseconds() < 3000)
	{
		_connected = InitialReceive();
	}

	//If connection is not made, return false
	if (!_connected)
	{
		std::cout << "No connection confirmation" << std::endl;

		return false;
	}


	//Send request to join each appropriate channel
	for (auto channel = _channels.begin(); channel != _channels.end(); channel++)
	{
		std::stringstream joinSS;

		joinSS << "JOIN #" << channel->first << "\r\n";

		Send(joinSS.str());
	}

	//Set socket to non-blocking in advance of main loop receiving
	_socket->setBlocking(false);

	std::cout << "Starting up" << std::endl;


	//Send startup message to each channel

	for (auto channel = _channels.begin(); channel != _channels.end(); channel++)
	{
		Send("PRIVMSG #" + channel->first + " :SchedulerBot starting up\r\n");
	}

	return true;
}

//Twitch has a minimum period allowed between sending messages
//Instead of sending messages directly, they go into a buffer to be sent out later with a sufficient interval between them
void SchedulerBot::AddToSendBuffer(std::string message, std::string channel)
{
	_sendBuffer.push_back(std::shared_ptr<Message>(new Message(message, channel)));
}

//Send a message to twitch IRC and check if it succeeds
void SchedulerBot::Send(std::string message)
{
	std::cout << "Sending " << message << std::endl;

	std::size_t sent;

	if (_socket->send(message.data(), message.length(), sent) != sf::Socket::Done)
		std::cout << "Message send fail" << std::endl;
}

//After initial connection request is made, check whether any response is forthcoming
bool SchedulerBot::InitialReceive()
{
	char data[1024];
	std::size_t received;

	//Check if there is any message received
	if (_socket->receive(data, sizeof(data), received) == sf::Socket::Done)
	{
		const std::string response(data, received);

		std::cout << std::endl << response << std::endl;

		//Connection cannot be authenticated (oauth code wrong)
		if (response.find("Login authentication failed") != std::string::npos)
		{
			std::cout << "FAIL" << std::endl;

			return false;
		}

		//Connection succesful
		else if (response.find("twitch.tv/commands") != std::string::npos)
			return true;
	}

	return false;
}

//Main loop receive command
void SchedulerBot::Receive()
{
	char data[1024];
	std::size_t received;

	//Check if any message has been received
	if (_socket->receive(data, sizeof(data), received) == sf::Socket::Done)
	{
		//Append the message to the read buffer to be handled during Update()
		_readBuffer.append(data, received);

		const std::string response(data, received);

		std::cout << std::endl << response << std::endl;

		//twitch IRC will occasionally PING the connected bot, this needs to be replied to with PONG
		if (response == "PING :tmi.twitch.tv\r\n")
		{
			std::cout << "PING" << std::endl;

			Send("PONG :tmi.twitch.tv\r\n");
		}
	}
}

///Main loop update function
void SchedulerBot::Update()
{
	//Parse the buffer of recieved messages, then handle the individual messages
	ParseBuffer();

	//Check if alerts need sending for approaching matches, check if match needs deleting from list
	std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();

	for (auto channel = _channels.begin(); channel != _channels.end(); channel++)
	{
		//Get matches for this channel
		std::map<std::chrono::system_clock::time_point, std::shared_ptr<Match>> matches = channel->second->GetMatches();

		for (auto match = matches.begin(); match != matches.end(); match ++)
		{
			//Check if the hour warning has been sent
			if (!match->second->GetHourWarningSent())
			{
				//If not, check if time is within one hour of match. 
				if (match->first - std::chrono::hours(1) < timeNow)
				{
					//Send the one hour time warning
					AddToSendBuffer(match->second->GetInfo() + " !timealertonehour  ONE HOUR WARNING", channel->first);

					//Set the hour warning as sent
					match->second->SetHourWarningSent(true);
				}
			}

			//Check if the 5 min warning has been sent
			if (!match->second->Get5MinWarningSent())
			{
				//If not, check if time is within 5 min of match
				if (match->first - std::chrono::minutes(5) < timeNow)
				{
					//Send the 5 min time warning
					AddToSendBuffer(match->second->GetInfo() + " !timealertfiveminutes FIVE MINUTE WARNING", channel->first);

					//Set the 5 min warning as sent
					match->second->Set5MinWarningSent(true);
				}
			}

			//If the match is due to happen now
			if (match->first < timeNow)
			{
				//Erase the match from the schedule
				channel->second->EraseMatch(match->first);
				
				//Save the new schedule
				SaveSchedule();
			}

		}
	}

	//If sufficient time (1500 ms) has elapsed since the last message, send the next message in the send buffer
	if (_sendBuffer.size() && _sendClock.getElapsedTime().asMilliseconds() >= 1500)
	{
		std::stringstream messageSS; 
	
		//Set up the PRIVMSG format
		messageSS << "PRIVMSG #" << _sendBuffer[0]->_channel << " :" << _sendBuffer[0]->_message << "\r\n";

		//Send the message
		Send(messageSS.str());

		//Clear the sent message from the buffer
		_sendBuffer.erase(_sendBuffer.begin());

		//Restart the timer
		_sendClock.restart();
	}
}

//Prase the received message buffer into individual messages, that can then be handeld
void SchedulerBot::ParseBuffer()
{
	///////////////////////
	//	Go through readBuffer, pull out one message at a time and deal with it, then delete message from readBuffer
	///////////////////////

	bool allMessagesHandled = false;

	//Keep going while there are still messages to handle
	while (!allMessagesHandled)
	{
		auto lastMessageChar = _readBuffer.begin();

		//Find the linefeed to signify end of message
		for (auto c = _readBuffer.begin(); c != _readBuffer.end(); c++)
		{
			if (int(*c == 10))	//Line feed
			{
				lastMessageChar = c;

				break;
			}
		}

		///////////////////////
		//	If no LINE FEED is found, or there are no chars left in the message, messsage handling is complete
		///////////////////////

		if (lastMessageChar == _readBuffer.end() || lastMessageChar == _readBuffer.begin())
			allMessagesHandled = true;


		///////////////////////
		//	Else, process the message
		///////////////////////

		else
		{

			///////////////////////
			//	Create the message string
			///////////////////////

			std::string message;

			for (auto c = _readBuffer.begin(); c != lastMessageChar + 1; c++)
			{
				message.push_back(*c);
			}

			///////////////////////
			//	Work out what kind of message it is and handle appropriately
			///////////////////////

			if (message.find("PRIVMSG") != std::string::npos)
			{
				///////////////////////
				//	Private message - find the username
				///////////////////////

				//username start
				size_t nameLastChar = message.find(".tmi.twitch.tv") -1;

				size_t nameFirstChar = nameLastChar;

				//find username start - char after '@'
				while (message[nameFirstChar - 1] != '@')
					nameFirstChar--;


				std::string username;

				//create the username string
				for (size_t i = nameFirstChar; i <= nameLastChar; i++)
					username.push_back(message[i]);

				std::cout << username << ": ";

				///////////////////////
				//	Now handle the content of the message
				///////////////////////

				//Find comment start 
				size_t commentStart = message.find("PRIVMSG");


				//Get channel

				size_t channelFirstChar = commentStart + 9;

				size_t channelLastChar = channelFirstChar;

				//find channel end, last char before ' '
				while (message[channelLastChar + 1] != ' ')
					channelLastChar++;


				std::string channel;

				//set up the channel string
				for (size_t i = channelFirstChar; i <= channelLastChar; i++)
					channel.push_back(message[i]);

				std::cout << channel << ": ";


				//find start char of the comment, first char after ':'
				while (message[commentStart] != ':')
				{
					commentStart++;
				}

				commentStart++;

				std::string comment;

				//Form the comment from all remaining chars in the message
				for (size_t c = commentStart; c != message.size(); c++)
				{
					comment.push_back(message[c]);

					std::cout << message[c];
				}

				///////////////////////
				//	Handle comment
				///////////////////////

				HandleComment(comment, channel, username);

				std::cout << std::endl << std::endl;
			}

			else
			{
				//std::cout << std::endl << "OTHER COMMAND" << std::endl << message << std::endl;
			}

			//Delete the message up to the line feed from the readBuffer
			_readBuffer.erase(_readBuffer.begin(), lastMessageChar + 1);
		}
	}
}

//Handle a comment from a user in a channel that has been parsed by the bufferparser
void SchedulerBot::HandleComment(std::string comment, std::string channel, std::string username)
{
	//General commands - anyone in any listed channel can use these

	//Show list of possible commands for general usres and streamers
	if (comment.find("!schedulehelp") != std::string::npos)
	{
		//Check this channel is in map of channels
		if (_channels.count(channel))
		{
			AddToSendBuffer("!schedule --- list full schedule for this stream", channel);
			AddToSendBuffer("!schedulegame <time> <DD/MM/YY> <Match Info> --- add game to schedule", channel);
			AddToSendBuffer("!scheduleremove <time> <DD/MM/YY> --- remove game from schedule", channel);
		}
	}

	//Write the curent schedule to chat
	else if (comment.find("!schedule\r") != std::string::npos)
	{
		//Check this channel is in map of channels
		if (_channels.count(channel))
		{
			//Get map of matches from the channel
			std::map<std::chrono::system_clock::time_point, std::shared_ptr<Match>> matches = _channels[channel]->GetMatches();

			if (matches.size())
			{
				//Iterate through each match and add to sendbuffer
				for (auto match = matches.begin(); match != matches.end(); match++)
				{
					std::stringstream matchSS;

					matchSS << match->second->GetTime() << "UTC " << match->second->GetDay() << "/" << match->second->GetMonth() << "/" << match->second->GetYear() << " " << match->second->GetInfo();

					AddToSendBuffer(matchSS.str(), channel);
				}
			}

			//If there are no matches, reply as such
			else
			{
				AddToSendBuffer("No matches scheduled", channel);
			}
		}
	}









	//Streamer commands - usable by those that own a listed channel, in their own channel

	//Check that the above conditions are met
	if (username == channel && _channels.count(channel))
	{
		//Add a new game to the schedule - the syntax has to be rather precise: !schedulegame <TIME> <DD/MM/YY> <MATCHINFO>
		if (comment.find("!schedulegame") != std::string::npos)
		{
			bool correctSyntax = true;

			std::string time;

			std::string day;

			std::string month;

			std::string year;

			std::string matchInfo;

			//GET TIME

			//Check the comment is big enough
			if (comment.size() >= 15)
			{
				size_t timeFirstChar = 14;

				size_t timeLastChar = 14;

				//Step through the comment until ' ' is found, to find the time substring
				while (comment[timeLastChar + 1] != ' ' && comment.size() > timeLastChar + 1)
					timeLastChar++;

				//Check the time has the correct size, either 3 or 4 charcters
				if (timeLastChar - timeFirstChar == 2 || timeLastChar - timeFirstChar == 3)
				{
					for (size_t i = timeFirstChar; i <= timeLastChar; i++)
						time.push_back(comment[i]);
				}

				//If it doesn't the syntax is incorrect
				else
					correctSyntax = false;



				//GET DATE

				//Check the comment is big enough
				if (comment.size() >= timeLastChar + 3)
				{
					size_t dateFirstChar = timeLastChar + 2;

					size_t dateLastChar = dateFirstChar;

					//Step through the comment until ' ' is found, to find the time substring
					while (comment[dateLastChar + 1] != ' ' && comment.size() > dateLastChar + 1)
						dateLastChar++;

					//Check the date has the correct size, 8 characters
					if (dateLastChar - dateFirstChar == 7)
					{
						day.push_back(comment[dateFirstChar]);
						day.push_back(comment[dateFirstChar + 1]);

						month.push_back(comment[dateFirstChar + 3]);
						month.push_back(comment[dateFirstChar + 4]);

						year.push_back(comment[dateFirstChar + 6]);
						year.push_back(comment[dateFirstChar + 7]);
					}

					//If it doesn't the syntax is incorrect
					else
						correctSyntax = false;


					//GET INFO

					//Check the comment is big enough
					if (comment.size() >= dateLastChar + 5)
					{
						size_t infoFirstChar = dateLastChar + 2;

						size_t infoLastChar = infoFirstChar;

						//Step through the comment until '\' is found, to find the time substring
						while (comment[infoLastChar + 1] != '\\' && comment.size() > infoLastChar + 3)
							infoLastChar++;

						//No limits on matchinfo size
						for (int i = infoFirstChar; i <= infoLastChar; i++)
							matchInfo.push_back(comment[i]);

					}

					else
						correctSyntax = false;
				}

				else
					correctSyntax = false;
			}

			else
				correctSyntax = false;

			std::cout << "Schedule: " << std::endl;

			std::cout << time << " " << day << " " << month << " " << year << " " << matchInfo << std::endl;

			//If the syntax is correct, go ahead and try to add the match
			if (correctSyntax)
			{
				std::cout << "Correct syntax" << std::endl;

				//If the match can be scheduled succesfully, save the schedule and send a message confirming the match is scheduled
				if (_channels[channel]->AddMatch(std::stoi(time), std::stoi(day), std::stoi(month), std::stoi(year), matchInfo))
				{
					SaveSchedule();

					AddToSendBuffer("Match scheduled", channel);
				}

				//Otherwise, send a message informing that a atch is already scheduled at that time
				else
					AddToSendBuffer("Match already scheduled for this time", channel);
			}

			//If the syntax is incorrect, send a message informing that the syntax is incorrect
			else
			{
				std::cout << "Incorrect syntax" << std::endl;

				AddToSendBuffer("Incorrect syntax --- !schedulerhelp", channel);
			}

		}

		//Remove a game from he schedule, the syntax is as adding a game, only without the match info: !scheduleremove <TIME> <DD/MM/YY>
		else if (comment.find("!scheduleremove ") != std::string::npos)
		{
			bool correctSyntax = true;

			std::string time;

			std::string day;

			std::string month;

			std::string year;

			//GET TIME

			//Check comment is large enough
			if (comment.size() >= 17)
			{
				size_t timeFirstChar = 16;

				size_t timeLastChar = 16;

				//Step through the comment until ' ' and the end of the time string is found
				while (comment[timeLastChar + 1] != ' ' && comment.size() > timeLastChar + 1)
					timeLastChar++;

				//Check the time has the correct size, either 3 or 4 charcters
				if (timeLastChar - timeFirstChar == 2 || timeLastChar - timeFirstChar == 3)
				{
					for (size_t i = timeFirstChar; i <= timeLastChar; i++)
						time.push_back(comment[i]);
				}

				else
					correctSyntax = false;

				//GET DATE

				//Check comment is large enough
				if (comment.size() >= timeLastChar + 3)
				{
					//Date must have a specific size, so the index of the last char is predetermined
					size_t dateFirstChar = timeLastChar + 2;

					size_t dateLastChar = dateFirstChar + 7;

					//Check comment is large enough
					if (comment.size() >= dateLastChar + 3)
					{
						//Check the date has the correct size, 8 characters
						if (dateLastChar - dateFirstChar == 7)
						{

							//Pick out the correct chars
							day.push_back(comment[dateFirstChar]);
							day.push_back(comment[dateFirstChar + 1]);

							month.push_back(comment[dateFirstChar + 3]);
							month.push_back(comment[dateFirstChar + 4]);

							year.push_back(comment[dateFirstChar + 6]);
							year.push_back(comment[dateFirstChar + 7]);
						}

						else
							correctSyntax = false;
					}

					else
						correctSyntax = false;
				}

				else 
					correctSyntax = false;
			}

			else
				correctSyntax = false;

			//If the syntax is correct, erase the match from the corresponding channel, save the schedule, and send a message confirming that the match was removed
			if (correctSyntax)
			{
				_channels[channel]->EraseMatch(std::stoi(time), std::stoi(day), std::stoi(month), std::stoi(year));

				SaveSchedule();

				AddToSendBuffer("Match removed", channel);
			}

			//Otherwise, send a message informing that the syntax was inccorect
			else
				AddToSendBuffer("Incorrect syntax --- !schedulerhelp", channel);


		}

		//Command to check the time that the schedulerbot is running on (and therefore the timezone to schedule in) = recommended UTC
		else if (comment.find("!scheduletime") != std::string::npos)
		{
			//Get the system time now
			std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

			//Convert to time_t
			std::time_t now_c = std::chrono::system_clock::to_time_t(now);
			std::tm now_tm;
	
			//convert to tm
			::localtime_r(&now_c, &now_tm);

			std::stringstream timeStream;

			//Add the time to a string stream
			timeStream << std::put_time(&now_tm, "%a %b %d %H:%M:%S %Y");

			//Add to send buffer
			AddToSendBuffer(timeStream.str(), channel);
		}

	}


















	//Commands from admin - the twitch user specified in the admin.txt file. Can be used in any channel

	if (username == _admin)
	{
		//Add a new channel to the channel map
		if (comment.find("!schedulenewchannel") != std::string::npos)
		{
			//Find channel name 
			if (comment.size() >= 20)
			{
				//Start of channel name will be just after '!schedulenewchannel '
				size_t nameFirstChar = 20;

				size_t nameLastChar = 20;

				//Step through the comment until end of comment is found
				while (comment[nameLastChar + 1] != '\\' && comment.size() > nameLastChar + 3)
					nameLastChar++;

				std::string channelToAdd;

				//Form channel string
				for (size_t i = nameFirstChar; i <= nameLastChar; i++)
					channelToAdd.push_back(comment[i]);

				//If the channel is already held, send message informing of this
				if (_channels.count(channelToAdd))
					AddToSendBuffer("Channel already listed", _admin);

				//Otherwise, add the channel, save te channel list, and send confirmation message
				else
				{
					_channels[channelToAdd] = std::shared_ptr<Channel>(new Channel(channelToAdd));

					SaveChannels();

					AddToSendBuffer(channelToAdd + " added to schedulebot", _admin);
				}
			}
		}

		//Remove channel from the channel map
		else if (comment.find("!scheduleremovechannel") != std::string::npos)
		{
			//Find channel name 
			if (comment.size() >= 23)
			{
				//Start of channel name will be just after '!scheduleremovechannel '
				size_t nameFirstChar = 23;

				size_t nameLastChar = 23;

				//Step through the comment until end of comment is found
				while (comment[nameLastChar + 1] != '\\' && comment.size() > nameLastChar + 3)
					nameLastChar++;

				std::string channelToRemove;

				//Form channel string
				for (size_t i = nameFirstChar; i <= nameLastChar; i++)
					channelToRemove.push_back(comment[i]);

				//If channel exists in map, remove it, save the channels, save the schedule, and send reply that channel was removed
				if (_channels.count(channelToRemove))
				{
					_channels.erase(channelToRemove);

					SaveChannels();

					SaveSchedule();

					AddToSendBuffer("Channel removed", _admin);
				}

				//Otherwise, send reply informing that there is no such channel listed
				else
				{
					AddToSendBuffer("Channel not listed", _admin);
				}
			}
		}
	}
}
