#ifndef CHECK_H
#define CHECK_H

#include "../nlohmann/json.hpp"

class Database;
class Spotter;

namespace check
{
	void twitch_run(Database* db, Spotter* client);

	void custom_run(Database* db, Spotter* client);
}

#endif