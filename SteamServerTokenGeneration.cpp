#include <iostream>
#include <nlohmann/json.hpp>
#include <curl/curl.h>

#include <string>
#include <fstream>
#include <ctime>

// Функция обратного вызова для обработки полученных данных от CURL
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
    size_t totalSize = size * nmemb;
    response->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

class ServerTokenGenerate {
private:
	std::string tokenSteam = "";
	int currentToken = 0; // max 1000
public:
	ServerTokenGenerate(const std::string& steamToken) :
		tokenSteam(steamToken) {}

	std::string getSteamToken() const {
		return tokenSteam;
	}

	void makeServerToken(int count) {
		if (count <= 0) {
			std::cout << "The number of created tokens is incorrectly set" << std::endl;
			return;
		}
		if (currentToken + count > 1000) {
			std::cout << "You have " << currentToken << "token(s)" <<
				"\nMax tokens are 1000\nYou're trying to make " << currentToken + count <<
				" token(s)" << std::endl;
			return;
		}
        nlohmann::json data;
        nlohmann::json result;
		for (int i = 0; i < count; ++i) {
            createAccountRequest(data);
            std::string nameOfToken = "steamserver_token_" + std::to_string(currentToken);
            ++currentToken;
            result[nameOfToken] = data["response"]["login_token"];
		}

        writeTokens(result);
	}
    
    void writeTokens(const nlohmann::json& result) {
        std::ifstream file("tokenGenerator.json");
        nlohmann::json temp = nlohmann::json::parse(file);
        temp["ServerTokens"] = result;

        std::ofstream outputFile("tokenGenerator.json");
        if (outputFile.is_open()) {
            outputFile << temp.dump(4);
            outputFile.close();
        }
    }
    
    // Функция для выполнения GET-запроса
    void performGetRequest(nlohmann::json& data, const std::string& requestURL) {
        std::string response;

        CURL* curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, requestURL.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                std::cout << "Failed to perform GET request: " << curl_easy_strerror(res) << std::endl;
            }

            curl_easy_cleanup(curl);
        }
        else {
            std::cout << "Failed to initialize cURL" << std::endl;
        }
        // Парсинг JSON-ответа
        try {
            data = nlohmann::json::parse(response);
        }
        catch (const nlohmann::json::exception& e) {
            std::cout << "Failed to parse JSON response: " << e.what() << std::endl;
            return;
        }
    }

    // Функция для выполнения POST-запроса
    void performPostRequest(nlohmann::json& data, const std::string& requestURL, const std::string& dataURL) {
        std::string response;

        CURL* curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, requestURL.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, dataURL.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                std::cout << "Failed to perform POST request: " << curl_easy_strerror(res) << std::endl;
            }

            curl_easy_cleanup(curl);
        }
        else {
            std::cout << "Failed to initialize cURL" << std::endl;
        }

        // Парсинг JSON-ответа
        try {
            data = nlohmann::json::parse(response);
        }
        catch (const nlohmann::json::exception& e) {
            std::cout << "Failed to parse JSON response: " << e.what() << std::endl;
            return;
        }
    }

	void createAccountRequest(nlohmann::json& data) {
        performPostRequest(data, "https://api.steampowered.com/IGameServersService/CreateAccount/v1/", "key=" + tokenSteam + "&appid=730&memo=" + std::to_string(currentToken));
	}

    void getAccountList(nlohmann::json& data) {
        performGetRequest(data, "https://api.steampowered.com/IGameServersService/GetAccountList/v1/?key=" + tokenSteam);
    }

    void getInfoAboutAcc(nlohmann::json& profile) {
        performGetRequest(profile, "https://api.steampowered.com/ISteamUser/GetFriendList/v1/?key=71E0A7D32231C2476D87BAA309D4AA94&steamid=76561198240949338");
    }

    void SetMemo() {

    }

    void ListOfTokens() {
        nlohmann::json data;
        getAccountList(data);
        for (size_t i = 0; i < data["response"]["servers"].size(); ++i) {
            std::cout << "MEMO: " << data["response"]["servers"][i]["memo"] << std::endl;
            std::cout << "\tSTEAM ID TOKEN: " << data["response"]["servers"][i]["steamid"] << std::endl;
            std::cout << "\tAPP ID: " << data["response"]["servers"][i]["appid"] << std::endl;
            std::cout << "\tLOGIN TOKEN: " << data["response"]["servers"][i]["login_token"] << std::endl;
            std::cout << "\tDELETED: ";
            if (data["response"]["servers"][i]["is_deleted"])
                std::cout << "YES" << std::endl;
            else std::cout << "NO" << std::endl;
            std::cout << "\tIS EXPIRED: ";
            if (data["response"]["servers"][i]["is_expired"])
                std::cout << "YES" << std::endl;
            else std::cout << "NO" << std::endl;
            // Преобразование в нормальное время по UTC
            std::cout << "\tLAST LOGON : ";
            unsigned int temp = data["response"]["servers"][i]["rt_last_logon"];
            std::cout << temp << std::endl;
            if (temp) {
                time_t lastLogonTime = static_cast<time_t>(temp);
                std::tm* utcTime = std::gmtime(&lastLogonTime);
                std::cout << asctime(utcTime) << "(UTC)" << std::endl;
            }
            else std::cout << "never" << std::endl;
            std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
        }

    }
};

int main() {
    setlocale(LC_ALL, "rus");

	std::ifstream file("tokenGenerator.json");
	nlohmann::json data = nlohmann::json::parse(file);
	std::string steamToken = data["SteamToken"];

	ServerTokenGenerate generator(steamToken);

	std::cout << generator.getSteamToken() << std::endl;

    generator.makeServerToken(1);
    generator.ListOfTokens();

	return 0;
}