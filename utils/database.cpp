#include "database.h"

bool Database::save()
{
	return io::write(m_file, m_data.dump().c_str(), m_data.dump().length());
}

Database::Database(const std::string& file)
{
	m_file = file;
start:
	std::string data;
	if (!io::read(m_file, data))
	{
		if (!io::create(m_file))
		{
			throw std::runtime_error("could not create database file");
		}
	}
	else
	{
		try //if file aint json
		{
			m_data = nlohmann::json::parse(data);
		}
		catch (...)
		{
			int i = 0;
			while (io::exists(m_file + "_bckup_" + std::to_string(i)))
			{
				++i;
			}
			if (!io::create(m_file + "_bckup_" + std::to_string(i)) ||
			!io::write(m_file + "_bckup_" + std::to_string(i), data.data(), data.size()) ||
			std::remove(m_file.data()) != 0)
			{
				throw std::runtime_error("failed to save backup of database");
			}
			std::cout << "failed to read database - removed it, saved old one to '" + m_file + "_bckup_" + std::to_string(i) + "' and created new one\n";
			goto start;
		}
	}
}

bool Database::add_twitch(const std::string& name, const std::string& server_id, const std::string& channel_id)
{
	std::string id = api::twitch::get_user_id(name);
	if (id == "-1")
	{
		return false;
	}

	for (auto& j : m_data["data"])
	{
		if (!j.contains("server_id"))
		{
			throw std::runtime_error("database data seem to be corrupted");
		}

		if (j["server_id"].get<std::string>() == server_id)
		{
			if (j["twitch"].size() >= m_limit)
			{
				return false;
			}

			for (auto& j_ : j["twitch"])
			{
				if (j_["name"].get<std::string>() == name)
				{
					return false; //already added this user
				}
			}

			nlohmann::json jj;
			jj["name"] = name;
			jj["user_id"] = id;
			jj["started_at"] = "-1";

			j["twitch"].emplace_back(jj);
			return save();
		}
	}

	nlohmann::json j;
	j["server_id"] = server_id;
	j["channel_id"] = channel_id;
	j["mention_role"] = "0";

	nlohmann::json jj;
	jj["name"] = name;
	jj["user_id"] = id;
	jj["started_at"] = "-1";

	j["twitch"].emplace_back(jj);

	m_data["data"].emplace_back(j);
	return save();
}

bool Database::add_custom(const std::string& link, const std::string& text, const std::string& server_id, const std::string& channel_id)
{
	for (auto& j : m_data["data"])
	{
		if (!j.contains("server_id"))
		{
			throw std::runtime_error("database data seem to be corrupted");
		}

		if (j["server_id"].get<std::string>() == server_id)
		{
			if (j["custom"].size() >= m_limit)
			{
				return false;
			}

			for (auto& j_ : j["custom"])
			{
				if (j_["link"].get<std::string>() == link)
				{
					return false; //already added this link (we limit records by requiring all links being unique)
				}
			}

			nlohmann::json jj;
			jj["link"] = link;
			jj["text"] = text;

			j["custom"].emplace_back(jj);
			return save();
		}
	}

	nlohmann::json j;
	j["server_id"] = server_id;
	j["channel_id"] = channel_id;
	j["mention_role"] = "0";

	nlohmann::json jj;
	jj["link"] = link;
	jj["text"] = text;

	j["custom"].emplace_back(jj);

	m_data["data"].emplace_back(j);
	return save();
}

nlohmann::json& Database::get()
{
	return m_data["data"];
}

nlohmann::json Database::get(const std::string& server_id, bool use_channel_id_instead)
{
	std::string id = use_channel_id_instead ? "channel_id" : "server_id";

	for (auto j : m_data["data"])
	{
		if (!j.contains(id))
		{
			throw std::runtime_error("database data seem to be corrupted");
		}

		if (j[id].get<std::string>() == server_id)
		{
			return j;
		}
	}

	return nlohmann::json{};
}

nlohmann::json Database::get(const std::string& server_id, const std::string& name, const std::string& type) //after call check if is empty
{
	nlohmann::json temp = get(server_id)[type];
	if (temp.empty())
	{
		return nlohmann::json{};
	}

	for (auto j : temp)
	{
		if (!j.contains("name"))
		{
			throw std::runtime_error("database data seem to be corrupted");
		}

		if (j["name"].get<std::string>() == name)
		{
			return j;
		}
	}

	return nlohmann::json{};
}

bool Database::_delete(const std::string& value, const std::string& key, const std::string& server_id, const std::string& type)
{
	for (auto& j : m_data["data"])
	{
		if (!j.contains("server_id"))
		{
			throw std::runtime_error("database data seem to be corrupted");
		}

		if (j["server_id"] == server_id)
		{
			int i = 0;
			for (auto& jj : j[type])
			{
				if (!jj.contains(key))
				{
					throw std::runtime_error("database data seem to be corrupted or _delete call is invalid");
				}

				if (jj[key] == value)
				{
					j[type].erase(static_cast<size_t>(i));
					return save();
				}
				++i;
			}
		}
	}
	return false;
}

bool Database::update_started_at(const std::string& name, const std::string& started_at)
{
	for (auto& j : m_data["data"])
	{
		for (auto& jj : j["twitch"])
		{
			if (jj["name"] == name)
			{
				jj["started_at"] = started_at;
			}
		}
	}
	return save();
}

std::string Database::get_twitch_started_at(const std::string& name, const std::string& channel_id)
{
	//channel ids are unique across servers so we can use it instead of server ids
	for (auto j : m_data["data"])
	{
		if (!j.contains("channel_id"))
		{
			throw std::runtime_error("database data seem to be corrupted");
		}

		if (j["channel_id"] == channel_id)
		{
			for (auto jj : j["twitch"])
			{
				if (jj["name"] == name)
				{
					return jj["started_at"].get<std::string>();
				}
			}
		}
	}
	return "";
}

bool Database::change_channel(const std::string& server_id, const std::string& channel_id)
{
	for (auto& j : m_data["data"])
	{
		if (!j.contains("channel_id") || !j.contains("server_id"))
		{
			throw std::runtime_error("database data seem to be corrupted");
		}

		if (j["server_id"] == server_id)
		{
			j["channel_id"] = channel_id;
			return save();
		}
	}
	return false;
}

bool Database::change_mention_role(const std::string& server_id, const std::string& role_id)
{
	for (auto& j : m_data["data"])
	{
		if (!j.contains("mention_role") || !j.contains("server_id"))
		{
			throw std::runtime_error("database data seem to be corrupted");
		}

		if (j["server_id"] == server_id)
		{
			j["mention_role"] = role_id;
			return save();
		}
	}
	return false;
}