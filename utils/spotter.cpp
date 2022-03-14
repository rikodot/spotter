#include "spotter.h"

#include "database.h"
#include "commands.h"
#include "check.h"

Spotter::Spotter(Database* d, Commands* c, const std::string& token, char numOfThreads) : m_db{ d }, m_cmd{ c }, SleepyDiscord::DiscordClient{ token, numOfThreads } {}

void Spotter::onMessage(SleepyDiscord::Message message)
{
	try
	{
		if (message.startsWith(m_prefix))
		{
			if (SleepyDiscord::hasPremission(SleepyDiscord::getPermissions(getServer(message.serverID), getMember(message.serverID, message.author.ID), getChannel(message.channelID)), SleepyDiscord::Permission::ADMINISTRATOR))
			{
				std::string name = message.content.substr(m_prefix.length(), message.content.find(' ') - m_prefix.length());
				if (!m_cmd->execute(name, message, this))
				{
					std::cout << "failed to execute command " << name << " by <@" << message.author.ID.string() << ">\n";
				}
			}
			else
			{
				message.reply(this, "you don't have permission to execute commands");
			}
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

std::string Spotter::getPrefix()
{
	return m_prefix;
}
