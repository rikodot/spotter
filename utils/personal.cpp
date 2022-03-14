#include "personal.h"

#include "../config.h"

/*currently no limit for amount of twitter users and discord users in watchlist - since this is for myself only should not be a concern*/

PersonalDatabase::PersonalDatabase(const std::string& file) : Database(file) {}

bool PersonalDatabase::setup(const std::string& channel_id, const std::string& mention)
{
	if (m_data["data"][0].contains("channel_id"))
	{
		return true;
	}

	nlohmann::json j;
	j["channel_id"] = channel_id;
	j["mention"] = mention;

	m_data["data"][0] = j;
	return save();
}

bool PersonalDatabase::add_discord(const std::string& id)
{
	for (auto j : m_data["data"][0]["discord"])
	{
		if (j["id"] == id)
		{
			return true;
		}
	}

	nlohmann::json j;
	j["id"] = id;

	m_data["data"][0]["discord"].emplace_back(j);
	return save();
}

bool PersonalDatabase::find_discord(const std::string& id)
{
	for (auto j : m_data["data"][0]["discord"])
	{
		if (j["id"] == id)
		{
			return true;
		}
	}
	return false;
}

bool PersonalDatabase::del_discord(const std::string& id)
{
	int i = 0;
	for (auto& j : m_data["data"][0]["discord"])
	{
		if (j["id"] == id)
		{
			m_data["data"][0]["discord"].erase(static_cast<size_t>(i));
			return save();
		}
		++i;
	}
	return false;
}

std::string PersonalDatabase::get_discord()
{
	std::string list;
	for (auto j : m_data["data"][0]["discord"])
	{
		list += j["id"].get<std::string>() + "\n";
	}

	return "USERS IN WATCHLIST (" + std::to_string(m_data["data"][0]["discord"].size()) + "):\n" + list.substr(0, list.length() - 1) /*removes the last line break*/;
}

bool PersonalDatabase::add_twitter(const std::string& name)
{
	for (auto j : m_data["data"][0]["twitter"])
	{
		if (j["name"] == name)
		{
			return true;
		}
	}

	std::string id = twitter_api::get_user_id(name);
	if (id == "-1")
	{
		return false;
	}
	nlohmann::json res = twitter_api::get_tweets(name);
	if (res.empty())
	{
		return false;
	}

	nlohmann::json j;
	j["name"] = name;
	j["id"] = id;
	j["last_tweet"] = res["meta"]["newest_id"];

	m_data["data"][0]["twitter"].emplace_back(j);
	return save();
}

bool PersonalDatabase::del_twitter(const std::string& name)
{
	int i = 0;
	for (auto& j : m_data["data"][0]["twitter"])
	{
		if (j["name"] == name)
		{
			m_data["data"][0]["twitter"].erase(static_cast<size_t>(i));
			return save();
		}
		++i;
	}
	return false;
}

std::string PersonalDatabase::get_twitter()
{
	std::string list;
	for (auto j : m_data["data"][0]["twitter"])
	{
		list += j["name"].get<std::string>() + "\n";
	}

	return "USERS IN WATCHLIST (" + std::to_string(m_data["data"][0]["twitter"].size()) + "):\n" + list.substr(0, list.length() - 1) /*removes the last line break*/;
}

std::string PersonalDatabase::channel()
{
	if (!m_data["data"][0].contains("channel_id"))
	{
		throw std::runtime_error("personal db does not contain channel_id");
	}

	return m_data["data"][0]["channel_id"];
}

std::string PersonalDatabase::mention()
{
	if (!m_data["data"][0].contains("mention"))
	{
		throw std::runtime_error("personal db does not contain mention");
	}

	return m_data["data"][0]["mention"].get<std::string>() != "0" ? "<@" + m_data["data"][0]["mention"].get<std::string>() + ">" : "";
}

Personal::Personal(const std::string& token, char numOfThreads) : SleepyDiscord::DiscordClient{ token, numOfThreads }
{
	m_db = new PersonalDatabase(P_DATABASE_FILE);
	if (!m_db->setup(P_CHANNEL, G_DEV_ID))
	{
		throw std::runtime_error("failed to setup personal db");
	}

	setIntents(0b111111111111111); //hmm use alt account just in case

	std::thread{ &Personal::run, this }.detach();
	std::thread{ &Personal::do_stuff, this }.detach();
	std::thread{ &Personal::commands, this }.detach();
}

Personal::~Personal()
{
	delete m_db;
}

void Personal::commands()
{
	char c[256];
	while (true)
	{
		scanf("%255s", c); //C based superior function takes priority over c++ stl bloatware
		//std::cin >> cmd;
		//std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\0');

		/*TWITTER*/
		if (strcmp(c, "add_twitter") == 0)
		{
			std::cout << "enter username: ";
			scanf("%255s", c);
			std::cout << (m_db->add_twitter(c) ? "successfully added or already in watchlist" : "user not found or failed to save changes or unknown error") << std::endl;
		}
		else if (strcmp(c, "del_twitter") == 0)
		{
			std::cout << "enter username: ";
			scanf("%255s", c);
			std::cout << (m_db->del_twitter(c) ? "successfully deleted" : "user not in watchlist or failed to save changes") << std::endl;
		}
		else if (strcmp(c, "get_twitter") == 0)
		{
			std::cout << m_db->get_twitter() << std::endl;
		}
		/*DISCORD*/
		else if (strcmp(c, "add_discord") == 0)
		{
			std::cout << "enter id: ";
			scanf("%255s", c);
			std::cout << (m_db->add_discord(c) ? "successfully added or already in watchlist" : "failed to save changes or unknown error") << std::endl;
		}
		else if (strcmp(c, "del_discord") == 0)
		{
			std::cout << "enter username: ";
			scanf("%255s", c);
			std::cout << (m_db->del_discord(c) ? "successfully deleted" : "user not in watchlist or failed to save changes") << std::endl;
		}
		else if (strcmp(c, "get_discord") == 0)
		{
			std::cout << m_db->get_discord() << std::endl;
		}
		else
		{
			std::cout << "unknown command (command entered: '" << c << "')" << std::endl;
		}
	}
}

void Personal::do_stuff()
{
	Sleep(1000);
	//twitter has very strict rate limit so I decided to not make this public (at least yet)
start:
	nlohmann::json& data = m_db->get();
	if (!data.empty() && !data[0].empty())
	{
		for (auto& j : data[0]["twitter"])
		{
			auto res = twitter_api::get_tweets(j["name"]);
			if (!res.empty())
			{
				std::string newest = res["meta"]["newest_id"];
				if (newest != j["last_tweet"])
				{
					bool found = false;
					std::for_each(res["data"].rbegin(), res["data"].rend(), [&](auto& i) {
						if (i["id"] == j["last_tweet"]) { found = true; }
						else if (found) { sendMessage(m_db->channel(), "`" + j["name"].get<std::string>() + "` tweeted: " + i["text"].get<std::string>() + " " + m_db->mention()); }
					});

					j["last_tweet"] = newest;
					if (!m_db->save())
					{
						throw std::runtime_error("failed to save last_tweet");
					}
				}
			}
		}
	}

	Sleep(10000);
	goto start;
}

void Personal::onMessage(SleepyDiscord::Message message)
{
	try
	{
		if (message.serverID.string().empty() /*is DM*/)
		{
			sendMessage(m_db->channel(), "`" + message.author.username + "#" + message.author.discriminator + "` sent a DM " + m_db->mention());
		}
		else if (m_db->find_discord(message.author.ID) /*is in watchlist*/)
		{
			sendMessage(m_db->channel(), "`" + message.author.username + "#" + message.author.discriminator + "` sent message in <#" + message.channelID.string() + ">" + " " + m_db->mention());
		}
		else if (utils::am_mentioned(message, this)) /*why check for mention when it gives notification by default? I run this on alt account that I do not login into*/
		{
			sendMessage(m_db->channel(), "`" + message.author.username + "#" + message.author.discriminator + "` mentioned this acount in <#" + message.channelID.string() + ">" + " " + m_db->mention());
		}
	}
	catch (std::runtime_error e)
	{
		std::cout << "runtime error: " << e.what() << std::endl;
	}
	catch (std::exception e)
	{
		std::cout << "exception: " << e.what() << std::endl;
	}
	catch (...)
	{
		std::cout << "unknown exception" << std::endl;
	}
}

std::string twitter_api::request(const std::string& link)
{
	struct curl_slist* headers = nullptr;
	headers = curl_slist_append(headers, G_TWITTER_BEARER);

	std::string response_headers;
	std::string response = api::request(link, headers, &response_headers);
	curl_slist_free_all(headers);

	//since twitter is only for personal use should not have to worry about rate limit (even though it is very low)
	/*std::string before = "\r\nx-rate-limit-remaining: ";
	std::string after = "\r\nstrict-transport-security: ";
	std::string limit = response_headers.substr(response_headers.find(before) + before.length(), response_headers.find(after) - (response_headers.find(before) + before.length()));
	if (std::stoi(limit) == 0)
	{
		before = "\r\nx-rate-limit-reset: ";
		after = "\r\ncontent-disposition: ";
		limit = response_headers.substr(response_headers.find(before) + before.length(), response_headers.find(after) - (response_headers.find(before) + before.length()));
		reset_time = std::stoi(limit);
	}*/

	return response;
}

std::string twitter_api::get_user_id(const std::string& name)
{
	std::string res = request("https://api.twitter.com/2/users/by/username/" + name);
	if (!nlohmann::json::accept(res))
	{
		throw std::runtime_error("api request response is not json");
	}

	auto j = nlohmann::json::parse(res);
	if (!j.contains("data") || !j["data"].contains("id")) //if(con1 || con2) - con1 is validated first
	{
		return "-1";
	}

	return j["data"]["id"].get<std::string>();
}

nlohmann::json twitter_api::get_tweets(const std::string& name)
{
	std::string user_id = get_user_id(name);
	if (user_id == "-1")
	{
		return nlohmann::json{};
	}

	std::string res = request("https://api.twitter.com/2/users/" + user_id + "/tweets");
	if (!nlohmann::json::accept(res))
	{
		throw std::runtime_error("api request response is not json");
	}

	return nlohmann::json::parse(res);
}