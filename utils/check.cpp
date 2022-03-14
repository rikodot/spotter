#include "check.h"

#include "database.h"
#include "spotter.h"

#include <unordered_map>
#include <set>
#include <tuple>

void check::custom_run(Database* db, Spotter* client)
{
	Sleep(500);
start:
	const nlohmann::json& data = db->get();
	if (!data.empty())
	{
		for (auto j : data) //each server
		{
			//let's pretend database ain't corrupted since we check each time we manipulate with its data
			for (auto jj : j["custom"]) //each website
			{
				std::string res = api::custom::request(jj["link"].get<std::string>());
				if (utils::string_contains(res, jj["text"].get<std::string>()))
				{
					client->sendMessage(j["channel_id"].get<std::string>(), jj["link"].get<std::string>() + " contains " + jj["text"].get<std::string>() + " " + utils::parse_role_mention(db->get(j["server_id"])["mention_role"].get<std::string>()));
				}
				Sleep(10);
			}
		}
	}

	Sleep(60000);
	goto start;
}

void check::twitch_run(Database* db, Spotter* client)
{
	//twitch: 100 max at one request
	//twitch: should be able to do 800 requests per minute, seems to be broken though
	Sleep(500);
start:
	const nlohmann::json& data = db->get();
	if (!data.empty())
	{
		//checking 100 users per each http request
		std::unordered_map<std::string, std::tuple<std::string, std::string, std::set<std::string>>> channels; //std::set ignores duplicates

		for (auto j : data) //each server
		{
			//let's pretend database ain't corrupted since we check each time we manipulate with its data
			for (auto jj : j["twitch"]) //each streamer
			{
				std::get<2>(channels[jj["user_id"].get<std::string>()]).insert(j["channel_id"].get<std::string>());
			}
		}

		//channels.first - user_id
		//std::get<0>(channels.second) - name
		//std::get<1>(channels.second) - started_at
		//std::get<2>(channels.second) - set of channel

		std::string get_attributes;
		nlohmann::json response;

		//request
		for (int i = 1; auto j : channels)
		{
			if (i % 100 == 0) //max user_ids we can have in one request
			{
				response += api::twitch::all_streams_data(get_attributes);

				i = 1;
				get_attributes.clear();
			}
			get_attributes += "&user_id=" + j.first;
			++i;
		}
		response += api::twitch::all_streams_data(get_attributes);

		for (auto j : response)
		{
			for (auto jj : j["data"])
			{
				std::get<0>(channels[jj["user_id"].get<std::string>()]) = jj["user_login"].get<std::string>(); //if this is empty = user is not live
				std::get<1>(channels[jj["user_id"].get<std::string>()]) = jj["started_at"].get<std::string>(); //if this is empty = user is not live
			}
		}

		//send messages about live streamers
		for (auto j : channels)
		{
			if (std::get<0>(j.second).empty())
			{
				continue;
			}

			for (auto k : std::get<2>(j.second))
			{
				if (db->get_twitch_started_at(std::get<0>(j.second), k) != std::get<1>(j.second))
				{
					client->sendMessage(k, std::get<0>(j.second) + " is now live! " + utils::parse_role_mention(db->get(k, true)["mention_role"].get<std::string>()));
				}
			}

			if (!db->update_started_at(std::get<0>(j.second), std::get<1>(j.second)))
			{
				throw std::runtime_error("failed to update started_at in database");
			}
		}
	}

	Sleep(10000);
	goto start;
}