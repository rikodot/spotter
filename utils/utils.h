#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <string_view>

#include "sleepy_discord/sleepy_discord.h"

namespace io
{
    bool exists(const std::string& name);

    size_t file_size(const std::string& name);

    bool create(const std::string& name);

    bool write(const std::string& name, const char* buffer, size_t size, bool append = false);

    bool read(const std::string& name, std::vector<unsigned char>& out);

    bool read(const std::string& name, std::string& out);
}

namespace utils
{
    std::string parse_role_mention(const std::string& id);

    bool string_contains(const std::string& s1, const std::string& s2);

    bool am_mentioned(SleepyDiscord::Message& msg, SleepyDiscord::DiscordClient* client);
}

#endif
