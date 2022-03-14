#ifndef API_H
#define API_H

#include <iostream>
#include <string>
#include "../curl/include/curl.h"
#include "../nlohmann/json.hpp"

/*
https://api.twitch.tv/helix/users?login=name
https://api.twitch.tv/helix/streams?user_id=id
*/

namespace api //lets not use OOP so we dont have to pass class object to each function or making it global var
{
	size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp);

	std::string request(const std::string& link, const struct curl_slist* headers, std::string* response_headers);

	namespace custom
	{
		std::string request(const std::string& link);
	}

	namespace twitch
	{
		std::string request(const std::string& link);

		std::string get_user_id(const std::string& name);

		bool is_live(const std::string& name);

		nlohmann::json all_streams_data(const std::string& get_attributes);
	}
}

#endif
