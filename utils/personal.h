#ifndef PERSONAL_H
#define PERSONAL_H

#include "sleepy_discord/sleepy_discord.h"
#include "database.h"

class Personal;

/*
{
	"data": [
		{
			"channel_id": "00",
			"mention": "0",
			"twitter": [
				{
					"name": "idk",
					"user_id": "00",
					"last_tweet": "00"
				},
				{
					"name": "idk",
					"user_id": "00",
					"last_tweet": "00"
				},
				...
			]
			"discord": [
				{ "id": "00" },
				{ "id": "00" },
				...
			]
		}
	]
}
*/

class PersonalDatabase : public Database
{
public:
	PersonalDatabase() = delete;
	PersonalDatabase(const std::string& file);

	bool setup(const std::string& channel_id, const std::string& mention = "0");

	bool add_discord(const std::string& id);

	bool del_discord(const std::string&);

	std::string get_discord();

	bool find_discord(const std::string& id);

	std::string channel();

	std::string mention();

	bool add_twitter(const std::string& name);

	bool del_twitter(const std::string& name);

	std::string get_twitter();
};

class Personal : public SleepyDiscord::DiscordClient
{
	PersonalDatabase* m_db = nullptr;

public:
	Personal() = delete;
	Personal(const std::string& token, char numOfThreads);

	void do_stuff();

	~Personal();

	void onMessage(SleepyDiscord::Message message) override;

	void commands(); //since we cant use discord messages...
};

/*
https://api.twitter.com/2/users/by/username/name/
https://api.twitter.com/2/users/id/tweets
*/

namespace twitter_api
{
	std::string request(const std::string& link);

	std::string get_user_id(const std::string& name);

	nlohmann::json get_tweets(const std::string& name);
}

#endif
