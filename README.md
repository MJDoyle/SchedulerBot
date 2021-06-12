# SchedulerBot
A simple twitch bot for scheduling. Primarily used to schedule Blood Bowl matches but can be repurposed.

###########
COMMANDS

Admin commands:

!schedulenewchannel <channel_name>

!scheduleremovechannel <channel_name>



Streamer (of listed channels) commands:

!schedulegame

!scheduleremove

!scheduletime



General user commands:

!schedulehelp

!schedule



#############
COMPILATION

Requires System and Network modules of SFML 2.0 (https://www.sfml-dev.org/)

Example makefile for gcc on ubuntu included


############
SETUP

An account for the bot must be registered at https://www.twitch.tv/

Then generate an OAUTH code at https://twitchapps.com/tmi/

botdetails.txt must contain the bot twitch username on the first line, and the bot OAUTH code on the second line

In admin.txt should be written the twitch username of the bot admin (who can add and remove channels from the bot)

channels.txt and schedule.txt can be edited manually but more easily done using twitch chat commands as above
