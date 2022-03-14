#include "utils.h"

bool io::exists(const std::string& name)
{
    return std::ifstream{ name }.good();
}

size_t io::file_size(const std::string& name)
{
    return static_cast<size_t>(std::ifstream{ name, std::ifstream::ate | std::ifstream::binary }.tellg());
}

bool io::create(const std::string& name)
{
    std::ofstream{ name }.close();
    return exists(name);
}

bool io::write(const std::string& name, const char* buffer, size_t size, bool append)
{
    std::ofstream file(name, std::ios_base::out | std::ios_base::binary | (append ? std::ios_base::app : 0));

    if (!file.write(buffer, size))
    {
        file.close();
        return false;
    }

    file.close();
    return true;
}

bool io::read(const std::string& name, std::vector<unsigned char>& out)
{
    std::ifstream file(name, std::ios::binary);
    if (!file.good())
    {
        return false;
    }

    file.unsetf(std::ios::skipws);
    file.seekg(0, std::ios::end);
    std::streampos size = file.tellg();
    file.seekg(0, std::ios::beg);
    out.reserve(size);
    out.insert(out.begin(), std::istream_iterator<unsigned char>(file), std::istream_iterator<unsigned char>());
    file.close();

    return true;
}

bool io::read(const std::string& name, std::string& out)
{
    std::vector<unsigned char> vec;
    if (!read(name, vec))
    {
        return false;
    }
    out.assign(vec.begin(), vec.end());
    return true;
}

std::string utils::parse_role_mention(const std::string& id)
{
    if (id == "0") return "";
    else if (id == "1") return "@everyone";
    else if (id == "2") return "@here";
    else return "<@&" + id + ">";
}

bool utils::string_contains(const std::string& s1, const std::string& s2)
{
    return s1.find(s2) != std::string::npos;
}

bool utils::am_mentioned(SleepyDiscord::Message& msg, SleepyDiscord::DiscordClient* client)
{
    std::string id = SleepyDiscord::User{ client->getCurrentUser() }.ID.string();
    for (auto j : msg.mentions)
    {
        if (j.ID.string() == id)
        {
            return true;
        }
    }
    return false;
}