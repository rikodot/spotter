#include "api.h"

#include "../config.h"

size_t api::write_callback(void* contents, size_t size, size_t nmemb, void* userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

std::string api::request(const std::string& link, const struct curl_slist* headers, std::string* response_headers)
{
	std::string response;
	char error[CURL_ERROR_SIZE];
	CURL* curl = curl_easy_init();

	curl_easy_setopt(curl, CURLOPT_URL, link.c_str());
	if (headers != nullptr)
	{
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	}
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, &error);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, write_callback);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, response_headers);

	int i = 0;
exec:
	try
	{
		CURLcode code = curl_easy_perform(curl);
		if (code != CURLE_OK)
		{
			throw std::runtime_error("CURLcode: " + std::to_string(code) + " - " + (strlen(error) ? std::string{ error, strlen(error) }.c_str() : curl_easy_strerror(code)));
		}
	}
	catch (...)
	{
		++i;
		if (i <= 5)
		{
			std::cout << "unknown exception in api::request, retrying " << i << "/5\n";
			goto exec;
		}
		std::cout << "unknown exception in api::request, continuing...\n";
	}

	return response;
}

std::string api::custom::request(const std::string& link)
{
	std::string response_headers;
	struct curl_slist* headers = nullptr;
	headers = curl_slist_append(headers, "User-Agent: Spotter");

	std::string response = api::request(link, headers, &response_headers);
	curl_slist_free_all(headers);
	return response;
}

std::string api::twitch::request(const std::string& link)
{
	struct curl_slist* headers = nullptr;
	headers = curl_slist_append(headers, G_TWITCH_CLIENT);
	headers = curl_slist_append(headers, G_TWITCH_BEARER);

	std::string response_headers;
	std::string response = api::request(link, headers, &response_headers);
	curl_slist_free_all(headers);

	//as of today twitch api is not rate limited, in case that changes - there is one (bad) way to check whether we are limited:
	/*std::string before = "\r\nRatelimit-Remaining: ";
	std::string after = "\r\nRatelimit-Reset: ";
	std::string limit = response_headers.substr(response_headers.find(before) + before.length(), response_headers.find(after) - (response_headers.find(before) + before.length()));
	if (std::stoi(limit) == 0)
	{
		before = "\r\nRatelimit-Reset: ";
		after = "\r\nTiming-Allow-Origin: ";
		limit = response_headers.substr(response_headers.find(before) + before.length(), response_headers.find(after) - (response_headers.find(before) + before.length()));
		reset_time = std::stoi(limit);
	}*/

	return response;
}

std::string api::twitch::get_user_id(const std::string& name)
{
	std::string res = request("https://api.twitch.tv/helix/users?login=" + name);
	if (!nlohmann::json::accept(res))
	{
		throw std::runtime_error("api request response is not json");
	}

	auto j = nlohmann::json::parse(res);
	if (!j.contains("data") || !j["data"][0].contains("view_count")) //if(con1 || con2) - con1 is validated first
	{
		return "-1";
	}

	return j["data"][0]["id"].get<std::string>();
}

bool api::twitch::is_live(const std::string& name)
{
	std::string id = get_user_id(name);
	if (id == "-1")
	{
		return false;
	}

	std::string res = request("https://api.twitch.tv/helix/streams?user_id=" + id);
	if (!nlohmann::json::accept(res))
	{
		throw std::runtime_error("api request response is not json");
	}

	auto j = nlohmann::json::parse(res);
	return j["data"][0].contains("viewer_count");
}

nlohmann::json api::twitch::all_streams_data(const std::string& get_attributes)
{
	std::string res = request("https://api.twitch.tv/helix/streams?" + get_attributes);
	if (!nlohmann::json::accept(res))
	{
		throw std::runtime_error("api request response is not json");
	}
	return nlohmann::json::parse(res);
}
