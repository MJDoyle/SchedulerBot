LIBS = -lsfml-system -lsfml-network

all: SchedulerBot

main.o: main.cpp
	g++ -c -std=c++0x  "main.cpp" -o main.o

schedulerbot.o: schedulerbot.cpp
	g++ -c -std=c++0x  "schedulerbot.cpp" -o schedulerbot.o

SchedulerBot: main.o schedulerbot.o
	@echo "** Building the bot..."
	g++ -o SchedulerBot main.o schedulerbot.o $(LIBS)

clean:
	@echo "** Removing object files and executable..."
	rm -f SchedulerBot *.o

install:
	@echo "** Installing..."
	cp SchedulerBot /usr/bin

uninstall:
	@echo "** Uninstalling..."
	rm -f /usr/bin/SchedulerBot
