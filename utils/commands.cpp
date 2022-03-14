#include "commands.h"

#include "spotter.h"
#include "database.h"

Commands::Commands(Database* d, const std::string& dev_id) : m_db{ d }, m_dev_id{ dev_id } {}

bool Commands::find(const std::string& name)
{
	for (auto j : m_data["data"])
	{
		if (!j.contains("command"))
		{
			throw std::runtime_error("commands.json is incomplete");
		}
		if (j["command"].get<std::string>() == name)
		{
			return true;
		}
	}
	return false;
}

nlohmann::json Commands::get(const std::string& name)
{
	for (auto j : m_data["data"])
	{
		if (!j.contains("command"))
		{
			throw std::runtime_error("commands.json is incomplete");
		}
		if (j["command"].get<std::string>() == name)
		{
			return j;
		}
	}
	return nlohmann::json{};
}

bool Commands::add(const std::string& name, const std::string& reply, FnPtr func, bool dev)
{
	if (find(name))
	{
		return false;
	}

	nlohmann::json j;
	j["command"] = name;
	j["reply"] = reply;
	j["admin_only"] = dev;
	j["call_func"] = func == nullptr ? "no" : "yes";

	if (func != nullptr)
	{
		m_funcs[name] = func;
	}

	m_data["data"].emplace_back(j);

	return true;
}

bool Commands::execute(const std::string& name, SleepyDiscord::Message& message, Spotter* client)
{
	if (!find(name))
	{
		return false;
	}

	nlohmann::json j = get(name);
	if (j.empty())
	{
		return false;
	}

	if (j["admin_only"] && message.author.ID.string() != m_dev_id)
	{
		return false;
	}

	if (j["call_func"].get<std::string>() == "yes")
	{
		m_funcs[name](message, client, m_db, static_cast<size_t>(name.length() + client->getPrefix().length()));
	}

	if (j["reply"].get<std::string>() != "reply_none")
	{
		message.reply(client, j["reply"].get<std::string>());
	}

	return true;
}

void cmd_funcs::is_live_twitch(SleepyDiscord::Message& msg, SleepyDiscord::DiscordClient* client, Database* db, size_t cmd_length)
{
	if (static_cast<int>(msg.content.find(' ')) != cmd_length)
	{
		msg.reply(client, "could not find argument");
		return;
	}

	std::string name = msg.content.substr(msg.content.find(' ') + 1, msg.content.find('\0'));

	if (name.length() <= 0)
	{
		msg.reply(client, "invalid argument");
		return;
	}

	msg.reply(client, api::twitch::is_live(name) ? name + " is currently live" : name + " is offline or user does not exist");
}

void cmd_funcs::add_twitch(SleepyDiscord::Message& msg, SleepyDiscord::DiscordClient* client, Database* db, size_t cmd_length)
{
	if (static_cast<int>(msg.content.find(' ')) != cmd_length)
	{
		msg.reply(client, "could not find argument");
		return;
	}

	std::string name = msg.content.substr(msg.content.find(' ') + 1, msg.content.find('\0'));

	if (name.length() <= 0)
	{
		msg.reply(client, "invalid argument");
		return;
	}

	msg.reply(client, db->add_twitch(name, msg.serverID, msg.channelID) ? name + " has been added" : name + " could not been added, is already added or maximum of streamers has been reached");
}

void cmd_funcs::del_twitch(SleepyDiscord::Message& msg, SleepyDiscord::DiscordClient* client, Database* db, size_t cmd_length)
{
	if (static_cast<int>(msg.content.find(' ')) != cmd_length)
	{
		msg.reply(client, "could not find argument");
		return;
	}

	std::string name = msg.content.substr(msg.content.find(' ') + 1, msg.content.find('\0'));

	if (name.length() <= 0)
	{
		msg.reply(client, "invalid argument");
		return;
	}

	msg.reply(client, db->_delete(name, "name", msg.serverID, "twitch") ? name + " has been deleted" : name + " could not been found");
}

void cmd_funcs::get_twitch(SleepyDiscord::Message& msg, SleepyDiscord::DiscordClient* client, Database* db, size_t cmd_length)
{
	nlohmann::json twitch = db->get(msg.serverID)["twitch"];

	std::string list;
	for (auto j : twitch)
	{
		list += j["name"].get<std::string>() + "\n";
	}

	msg.reply(client, "STREAMERS IN WATCHLIST (" + std::to_string(twitch.size()) + "):\n" + list.substr(0, list.length() - 1) /*removes the last line break*/);
}

void cmd_funcs::get_channel(SleepyDiscord::Message& msg, SleepyDiscord::DiscordClient* client, Database* db, size_t cmd_length)
{
	nlohmann::json channel = db->get(msg.serverID);
	msg.reply(client, channel.empty() ? "no record found for this server, you may first setup notifier" : ("<#" + channel["channel_id"].get<std::string>() + ">"));
}

void cmd_funcs::change_channel(SleepyDiscord::Message& msg, SleepyDiscord::DiscordClient* client, Database* db, size_t cmd_length)
{
	if (static_cast<int>(msg.content.find(' ')) != cmd_length)
	{
		msg.reply(client, "could not find argument");
		return;
	}

	std::string channel = msg.content.substr(msg.content.find(' ') + 1, msg.content.find('\0'));

	if (channel.length() <= 0)
	{
		msg.reply(client, "invalid argument");
		return;
	}

	msg.reply(client, db->change_channel(msg.serverID, channel) ? ("changed channel to <#" + channel + ">") : "no log channel record found, you may first setup notifier");
}

void cmd_funcs::get_mention_role(SleepyDiscord::Message& msg, SleepyDiscord::DiscordClient* client, Database* db, size_t cmd_length)
{
	nlohmann::json role = db->get(msg.serverID);
	if (role.empty())
	{
		msg.reply(client, "no record found for this server, you may first setup notifier");
		return;
	}

	std::string temp = utils::parse_role_mention(role["mention_role"]);
	msg.reply(client, "`" + (temp.empty() ? "none" : temp) + "`");
}

void cmd_funcs::change_mention_role(SleepyDiscord::Message& msg, SleepyDiscord::DiscordClient* client, Database* db, size_t cmd_length)
{
	if (static_cast<int>(msg.content.find(' ')) != cmd_length)
	{
		msg.reply(client, "could not find argument");
		return;
	}

	std::string role = msg.content.substr(msg.content.find(' ') + 1, msg.content.find('\0'));

	if (role.length() <= 0)
	{
		msg.reply(client, "invalid argument");
		return;
	}

	std::string temp = utils::parse_role_mention(role);
	msg.reply(client, db->change_mention_role(msg.serverID, role) ? ("changed mention role to `" + (temp.empty() ? "none" : temp) + "`") : "no mention role record found, you may first setup notifier");
}

void cmd_funcs::lookup(SleepyDiscord::Message& msg, SleepyDiscord::DiscordClient* client, Database* db, size_t cmd_length)
{
	if (static_cast<int>(msg.content.find(' ')) != cmd_length)
	{
		msg.reply(client, "could not find argument");
		return;
	}

	std::string id = msg.content.substr(msg.content.find(' ') + 1, msg.content.find('\0'));

	if (id.length() <= 0)
	{
		msg.reply(client, "invalid argument");
		return;
	}

	SleepyDiscord::User user{ client->getUser(id.c_str()) };

	//temps
	std::string premium;
	switch (user.premiumType)
	{
	case SleepyDiscord::User::PremiumType::None:
		premium = "none";
		break;
	case SleepyDiscord::User::PremiumType::Nitro_Classic:
		premium = "nitro classic";
		break;
	case SleepyDiscord::User::PremiumType::Nitro:
		premium = "nitro boost";
		break;
	default:
		premium = "unknown";
	}

	msg.reply(client, user.empty() ? "could not find user" : "LOOKUP DATA:\n"
								"username - " + user.username + "#" + user.discriminator + "\n"
								"bot - " + (user.bot ? "true" : "false") + "\n"
								"email - " + user.email + "\n"
								"verified - " + (user.verified ? "true" : "false") + "\n"
								"mfa enabled - " + (user.mfa_enabled ? "true" : "false") + "\n"
								"locale - " + user.locale + "\n"
								"premium type - " + premium + "\n"
								"flags - " + std::to_string(static_cast<int>(user.flags)) + "\n"
								"public flags - " + std::to_string(static_cast<int>(user.publieFlags)) + "\n"
	);
}

void cmd_funcs::limit(SleepyDiscord::Message& msg, SleepyDiscord::DiscordClient* client, Database* db, size_t cmd_length)
{
	nlohmann::json twitch = db->get(msg.serverID)["twitch"];
	nlohmann::json custom = db->get(msg.serverID)["custom"];

	msg.reply(client, "twitch - " + std::to_string(twitch.size()) + "/" + std::to_string(db->m_limit) +
					"\ncustom - " + std::to_string(custom.size()) + "/" + std::to_string(db->m_limit));
}

void cmd_funcs::test(SleepyDiscord::Message& msg, SleepyDiscord::DiscordClient* client, Database* db, size_t cmd_length)
{
	if (static_cast<int>(msg.content.find(' ')) != cmd_length)
	{
		msg.reply(client, "could not find argument");
		return;
	}

	std::string args = msg.content.substr(msg.content.find(' ') + 1, msg.content.find('\0'));
	if (args.length() <= 0) { msg.reply(client, "invalid arguments"); return; }

	bool good = args.find(' ') != args.find('\0');
	if (!good) { msg.reply(client, "invalid arguments"); return; }

	std::string first = args.substr(0, args.find(' '));
	if (first.length() <= 0) { msg.reply(client, "invalid arguments"); return; }
	std::string second = args.substr(args.find(' ') + 1, args.find('\0'));
	if (second.length() <= 0) { msg.reply(client, "invalid arguments"); return; }

	msg.reply(client, "first -> " + first + "\nsecond -> " + second);
}

void cmd_funcs::add_custom(SleepyDiscord::Message& msg, SleepyDiscord::DiscordClient* client, Database* db, size_t cmd_length)
{
	if (static_cast<int>(msg.content.find(' ')) != cmd_length)
	{
		msg.reply(client, "could not find argument");
		return;
	}

	std::string args = msg.content.substr(msg.content.find(' ') + 1, msg.content.find('\0'));
	if (args.length() <= 0) { msg.reply(client, "invalid arguments"); return; }

	bool good = args.find(' ') != args.find('\0');
	if (!good) { msg.reply(client, "invalid arguments"); return; }

	std::string first = args.substr(0, args.find(' '));
	if (first.length() <= 0) { msg.reply(client, "invalid arguments"); return; }
	std::string second = args.substr(args.find(' ') + 1, args.find('\0'));
	if (second.length() <= 0) { msg.reply(client, "invalid arguments"); return; }

	msg.reply(client, db->add_custom(first, second, msg.serverID, msg.channelID) ? "custom link has been added" : "custom link could not been added, is already added or maximum of links has been reached");
}

void cmd_funcs::del_custom(SleepyDiscord::Message& msg, SleepyDiscord::DiscordClient* client, Database* db, size_t cmd_length)
{
	if (static_cast<int>(msg.content.find(' ')) != cmd_length)
	{
		msg.reply(client, "could not find argument");
		return;
	}

	std::string link = msg.content.substr(msg.content.find(' ') + 1, msg.content.find('\0'));

	if (link.length() <= 0)
	{
		msg.reply(client, "invalid argument");
		return;
	}

	msg.reply(client, db->_delete(link, "link", msg.serverID, "custom") ? "record has been deleted" : "record could not been found");
}

void cmd_funcs::get_custom(SleepyDiscord::Message& msg, SleepyDiscord::DiscordClient* client, Database* db, size_t cmd_length)
{
	nlohmann::json custom = db->get(msg.serverID)["custom"];

	std::string list;
	for (auto j : custom)
	{
		list += "`link:` " + j["link"].get<std::string>() + " `text:` " + j["text"].get<std::string>() + "\n";
	}

	msg.reply(client, "LINKS IN WATCHLIST (" + std::to_string(custom.size()) + "):\n" + list.substr(0, list.length() - 1) /*removes the last line break*/);
}

void cmd_funcs::check_custom(SleepyDiscord::Message& msg, SleepyDiscord::DiscordClient* client, Database* db, size_t cmd_length)
{
	if (static_cast<int>(msg.content.find(' ')) != cmd_length)
	{
		msg.reply(client, "could not find argument");
		return;
	}

	std::string args = msg.content.substr(msg.content.find(' ') + 1, msg.content.find('\0'));
	if (args.length() <= 0) { msg.reply(client, "invalid arguments"); return; }

	bool good = args.find(' ') != args.find('\0');
	if (!good) { msg.reply(client, "invalid arguments"); return; }

	std::string first = args.substr(0, args.find(' '));
	if (first.length() <= 0) { msg.reply(client, "invalid arguments"); return; }
	std::string second = args.substr(args.find(' ') + 1, args.find('\0'));
	if (second.length() <= 0) { msg.reply(client, "invalid arguments"); return; }

	std::string response = api::custom::request(first);

	msg.reply(client, utils::string_contains(response, second) ? "the website contains the text" : "the website does not contain the text");
}