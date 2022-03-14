#ifndef DATABASE_H
#define DATABASE_H

#include "../nlohmann/json.hpp"

#include "utils.h"
#include "api.h"

/*
{
	"data": [
		{
			"server_id": "00",
			"twitch": [
				{
					"name": "idk",
					"user_id": "00",
					"started_at": "00" //last started_at which has been announced so announce only if started_at != started_at from http req
				},
				{
					"name": "idk",
					"user_id": "00",
					"started_at": "00"
				},
				...
			],
			"custom": [
				{
					"link": "idk",
					"required_text": "00"
				},
				...
			]
			"channel_id": "00",
			"mention_role": "1"
		}
		...
	]
}
*/

class Database
{
protected:
	std::string m_file;
	nlohmann::json m_data;

public:
	const unsigned int m_limit = 20; //maximum of watched streamers for each server

	Database() = delete;

	Database(const std::string& file);

	/*twitch*/
	bool add_twitch(const std::string& name, const std::string& server_id, const std::string& channel_id);

	bool update_started_at(const std::string& name, const std::string& started_at);

	std::string get_twitch_started_at(const std::string& name, const std::string& channel_id);

	/*cutom*/
	bool add_custom(const std::string& link, const std::string& text, const std::string& server_id, const std::string& channel_id);

	/*general*/
	bool save();

	bool change_channel(const std::string& server_id, const std::string& channel_id);

	bool change_mention_role(const std::string& server_id, const std::string& role_id);

	nlohmann::json& get();

	nlohmann::json get(const std::string& server_id, bool use_channel_id_instead = false);

	nlohmann::json get(const std::string& server_id, const std::string& name, const std::string& type);

	bool _delete(const std::string& value, const std::string& key, const std::string& server_id, const std::string& type);
};

#endif
