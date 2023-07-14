#include <nlohmann/json.hpp>
#include <curl/curl.h>

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <ctime>

#include "ControllerOfGenerator.h"
#include "ServerTokenGenerate.h"

int main() {
	std::ifstream file("tokenGenerator.json");
	nlohmann::json data = nlohmann::json::parse(file);
	std::string steamToken = data["SteamToken"];

    ControllerOfGenerator controller(steamToken);
    controller.run();

	return 0;
}
