#define ASIO_NO_EXCEPTIONS

#include "utils/commands.h"
#include "utils/database.h"
#include "utils/spotter.h"
#include "utils/check.h"
#include "utils/personal.h"
#include "config.h"

int main()
{
	Spotter* client = nullptr;
	Commands* cmd = nullptr;
	Database* db = nullptr;
	Personal* personal = nullptr;

start:
	try //slows down code execution, improve
	{
		db = new Database(S_DATABASE_FILE);
		cmd = new Commands(db, G_DEV_ID);

		client = new Spotter(db, cmd, S_TOKEN, SleepyDiscord::USER_CONTROLED_THREADS);
		client->setIntents(0b111111111111111); //everything from SleepyDiscord::Intent

		//add command description to cmd database so we dont have to write help command manually
		if (!cmd->add("help", "**LIST OF COMMANDS:**\n"
							"__twitch:__\n"
							"+is_live_twitch <streamer name> - checks whether streamer is currently live\n"
							"+add_twitch <streamer name> - adds streamer to live notification list\n"
							"+del_twitch <streamer name> - deletes streamer from live notification list\n"
							"+get_twitch - shows list of streamers in watchlist\n"
							"__custom:__\n"
							"+add_custom <link> <required text> - will send request to this website each minute and look for required text, if found (case sensitive), will notify you (good for checking product availability e.g. gpu etc), put 'view-source:' before link and check for text here\n"
							"+del_custom <link> - deletes websites from list\n"
							"+get_custom - shows list of currently checked websites\n"
							"+check_custom <link> <required text> - will send request to this website and look whether it contains required text (case sensitive)\n"
							"__other:__\n"
							"+get_channel - shows channel which is used for notifications\n"
							"+change_channel <channel id> - change channel which is used for notifications\n"
							"+get_mention_role - shows what role is mentioned in notifications\n"
							"+change_mention_role <role id> - changes mention role - use ID '0' to not mention, '1' to mention `@everyone`, '2' to mention `@here` or use regular role id\n"
							"+lookup <user id> - info about the user\n"
							"+limit - shows limit of watched streamers + how many streamers are currently in watchlist\n"
							"+test - you may or may not be able to use this command\n"
							"+help - list of commands") ||
			!cmd->add("is_live_twitch", "reply_none", cmd_funcs::is_live_twitch) ||
			!cmd->add("add_twitch", "reply_none", cmd_funcs::add_twitch) ||
			!cmd->add("del_twitch", "reply_none", cmd_funcs::del_twitch) ||
			!cmd->add("get_twitch", "reply_none", cmd_funcs::get_twitch) ||

			!cmd->add("add_custom", "reply_none", cmd_funcs::add_custom) ||
			!cmd->add("del_custom", "reply_none", cmd_funcs::del_custom) ||
			!cmd->add("get_custom", "reply_none", cmd_funcs::get_custom) ||
			!cmd->add("check_custom", "reply_none", cmd_funcs::check_custom) ||

			!cmd->add("get_channel", "reply_none", cmd_funcs::get_channel) ||
			!cmd->add("change_channel", "reply_none", cmd_funcs::change_channel) ||
			!cmd->add("get_mention_role", "reply_none", cmd_funcs::get_mention_role) ||
			!cmd->add("change_mention_role", "reply_none", cmd_funcs::change_mention_role) ||
			!cmd->add("limit", "reply_none", cmd_funcs::limit) ||
			!cmd->add("test", "reply_none", cmd_funcs::test, true) ||
			!cmd->add("lookup", "reply_none", cmd_funcs::lookup)) throw std::runtime_error("failed to add command");

		std::thread{ check::twitch_run, db, client }.detach(); //no rate limit from twitch
		Sleep(500);
		std::thread{ check::custom_run, db, client }.detach();

		if (P_ENABLE) { personal = new Personal(P_TOKEN, SleepyDiscord::USER_CONTROLED_THREADS); }

		Sleep(500);
		client->run();
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
	std::cout << "restarting...\n";

	delete db;
	delete cmd;
	delete client;
	if (P_ENABLE) { delete personal; }

	goto start;
}
