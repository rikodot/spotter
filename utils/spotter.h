#ifndef SPOTTER_H
#define SPOTTER_H

#include "sleepy_discord/sleepy_discord.h"
#include "../config.h"

class Commands;
class Database;

class Spotter : public SleepyDiscord::DiscordClient
{
	Database* m_db = nullptr;
	Commands* m_cmd = nullptr;
	std::string m_prefix = S_CMD_PREFIX;
public:
	Spotter() = delete;
	Spotter(Database* d, Commands* c, const std::string& token, char numOfThreads);

	void onMessage(SleepyDiscord::Message message) override;

	std::string getPrefix();
};

#endif
