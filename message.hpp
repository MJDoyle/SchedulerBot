//Very simple message class (more of a struct) that is used to store a message and the channel that it is to be sent to, while waiting in the send buffer

#pragma once

#include "config.hpp"

class Message
{
public:

	Message(std::string message, std::string channel)
	{
		_message = message;
		_channel = channel;
	}

	std::string _message;

	std::string _channel;
};