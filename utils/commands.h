#ifndef COMMANDS_H
#define COMMANDS_H

#include "sleepy_discord/sleepy_discord.h"
#include "../nlohmann/json.hpp"
#include <map>

class Database;
class Spotter;

/*
{
	"data": [
		{
			"command": "help",
			"reply": "idk",
			"admin_only": "false",
			"call_func": "no"
		},
		{
			"command": "test",
			"reply": "reply_none",
			"admin_only": "true",
			"call_func": "yes"
		},
		...
	]
}
*/

class Commands
{
	nlohmann::json m_data;
	typedef void (*FnPtr)(SleepyDiscord::Message&, SleepyDiscord::DiscordClient*, Database*, size_t);
	std::map<std::string, FnPtr> m_funcs;
	Database* m_db = nullptr;

public:
	std::string m_dev_id;

	Commands() = delete;
	Commands(Database* d, const std::string& dev_id);

	bool find(const std::string& name);

	nlohmann::json get(const std::string& name);

	bool add(const std::string& name, const std::string& reply, FnPtr func = nullptr, bool dev = false);

	bool execute(const std::string& name, SleepyDiscord::Message& message, Spotter* client);
};

namespace cmd_funcs
{
	void is_live_twitch(SleepyDiscord::Message& msg, SleepyDiscord::DiscordClient* client, Database* db, size_t cmd_length);

	void add_twitch(SleepyDiscord::Message& msg, SleepyDiscord::DiscordClient* client, Database* db, size_t cmd_length);

	void del_twitch(SleepyDiscord::Message& msg, SleepyDiscord::DiscordClient* client, Database* db, size_t cmd_length);

	void get_twitch(SleepyDiscord::Message& msg, SleepyDiscord::DiscordClient* client, Database* db, size_t cmd_length);

	void get_channel(SleepyDiscord::Message& msg, SleepyDiscord::DiscordClient* client, Database* db, size_t cmd_length);

	void change_channel(SleepyDiscord::Message& msg, SleepyDiscord::DiscordClient* client, Database* db, size_t cmd_length);

	void get_mention_role(SleepyDiscord::Message& msg, SleepyDiscord::DiscordClient* client, Database* db, size_t cmd_length);

	void change_mention_role(SleepyDiscord::Message& msg, SleepyDiscord::DiscordClient* client, Database* db, size_t cmd_length);

	void lookup(SleepyDiscord::Message& msg, SleepyDiscord::DiscordClient* client, Database* db, size_t cmd_length);

	void limit(SleepyDiscord::Message& msg, SleepyDiscord::DiscordClient* client, Database* db, size_t cmd_length);

	void test(SleepyDiscord::Message& msg, SleepyDiscord::DiscordClient* client, Database* db, size_t cmd_length);

	void add_custom(SleepyDiscord::Message& msg, SleepyDiscord::DiscordClient* client, Database* db, size_t cmd_length);

	void del_custom(SleepyDiscord::Message& msg, SleepyDiscord::DiscordClient* client, Database* db, size_t cmd_length);

	void get_custom(SleepyDiscord::Message& msg, SleepyDiscord::DiscordClient* client, Database* db, size_t cmd_length);

	void check_custom(SleepyDiscord::Message& msg, SleepyDiscord::DiscordClient* client, Database* db, size_t cmd_length);
}

#endif
