#include <iostream>
#include <nlohmann/json.hpp>
#include <curl/curl.h>

#include <string>
#include <fstream>

// Функция обратного вызова для обработки полученных данных от CURL
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
    size_t totalSize = size * nmemb;
    response->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

class ServerTokenGenerate {
private:
	std::string tokenSteam = "";
	int currentToken = 0;
	int deleteToken = 0;
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
		for (int i = 0; i < count; ++i) {
            nlohmann::json data;
            makeReq(data);
            
		}
	}

    std::string makeUrl() {
        return ("https://api.steampowered.com/IGameServersService/CreateAccount/v1/");
    }

	void makeReq(nlohmann::json& data) {
        // Инициализация CURL
        CURL* curl = curl_easy_init();
        if (!curl) {
            std::cout << "Failed to initialize CURL" << std::endl;
            return;
        }

        std::string reqURL = makeUrl();

        // Установка URL-адреса для отправки запроса
        curl_easy_setopt(curl, CURLOPT_URL, reqURL.c_str());

        // Установка метода запроса на POST
        curl_easy_setopt(curl, CURLOPT_POST, 1L);

        // Установка данных для отправки
        std::string postData = "key=" + tokenSteam +  "&appid=730&memo=\"TokenGenerator\"";
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());

        // Установка функции обратного вызова для обработки полученных данных
        std::string response;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        // Выполнение запроса
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cout << "Failed to execute CURL request: " << curl_easy_strerror(res) << std::endl;
            curl_easy_cleanup(curl);
            return;
        }

        // Завершение работы с CURL
        curl_easy_cleanup(curl);

        // Парсинг JSON-ответа
        try {
            std::cout << response << std::endl;
            data = nlohmann::json::parse(response);
        }
        catch (const nlohmann::json::exception& e) {
            std::cout << "Failed to parse JSON response: " << e.what() << std::endl;
            return;
        }
	}

};

int main() {
	std::ifstream file("tokenGenerator.json");
	nlohmann::json data = nlohmann::json::parse(file);
	std::string steamToken = data["SteamToken"];

	ServerTokenGenerate generator(steamToken);

	std::cout << generator.getSteamToken() << std::endl;

    generator.makeServerToken(1);
    
	return 0;
}