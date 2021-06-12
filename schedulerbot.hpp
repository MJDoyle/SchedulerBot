//Main class for the bot, everything happens in here

#pragma once

#include "config.hpp"
#include "channel.hpp"

class Message;

class SchedulerBot
{
public:

	//Constructor
	SchedulerBot();

	//Connect to twitch IRC
	bool Connect();

	//Send message
	void Send(std::string message);

	//After sending initial connection request, wait for response
	bool InitialReceive();

	//Receive commands from channels
	void Receive();

	//Parse messages, check if alerts need sending, send buffered messages
	void Update();

private:

	//TCP socked to connect to twitch IRC with
	std::shared_ptr<sf::TcpSocket> _socket;

	//Connection state
	bool _connected;

	//All read messages get appended here, to be read later
	std::string _readBuffer;

	//Messages to be sent are stored here until they get sent
	std::vector<std::shared_ptr<Message>> _sendBuffer;

	//Add a message to the send buffer to be sent later
	void AddToSendBuffer(std::string message, std::string channel);

	//Timer for sending messages to avoid twitch limit on message send frequency
	sf::Clock _sendClock;

	//Parse the read buffer into a series of complete messages
	void ParseBuffer();

	//Handle each complete message and deal with it
	void HandleComment(std::string comment, std::string channel, std::string username);

	//Map (by channel name) of all channels that the bot handles
	std::map<std::string, std::shared_ptr<Channel>> _channels;

	//The admin channel
	std::string _admin;

	//Twitch account name for the bot
	std::string _botName;

	//OAUTH code for the bot
	std::string _botOauth;

	//Load the bot details from file
	void LoadBotDetails();

	//Load the admin channel from file
	void LoadAdmin();

	//Load channels from file
	void LoadChannels();

	//Load schedule for all channels from file
	void LoadSchedule();

	//Whenever a new channel is added save it to file
	void SaveChannels();

	//Whenever a new match is added save it to file
	void SaveSchedule();
};