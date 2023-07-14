#include <nlohmann/json.hpp>
#include <curl/curl.h>

#include <iostream>
#include <string>
#include <vector>
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
            std::string nameOfToken = "steamserver_token_" + std::to_string(currentToken);
            CreateAccountRequest(data, nameOfToken, "730");
            ++currentToken;
            nlohmann::json temp;
            temp["Memo"] = nameOfToken;
            temp["Identifier"] = data["response"]["login_token"];
            temp["GameID"] = "730";
            std::string steamidtoken = data["response"]["steamid"];
            result[steamidtoken] = temp;
		}
        writeTokens(result);
	}
    
    bool overwriteTokens(const nlohmann::json& result) {
        std::ifstream file("tokenGenerator.json");
        nlohmann::json temp = nlohmann::json::parse(file);
        temp["ServerTokens"] = result;

        std::ofstream outputFile("tokenGenerator.json");
        if (outputFile.is_open()) {
            outputFile << temp.dump(4);
            outputFile.close();
            return true;
        }
        else {
            std::cout << "Не удалось открыть файл: \"tokenGenerator.json\"" << std::endl;
            std::cout << "Проверьте его расположенине и название!" << std::endl;
            return false;
        }
    }

    bool writeTokens(const nlohmann::json& result) {
        std::ifstream file("tokenGenerator.json");
        nlohmann::json temp = nlohmann::json::parse(file);
        temp["ServerTokens"].merge_patch(result);

        std::ofstream outputFile("tokenGenerator.json");
        if (outputFile.is_open()) {
            outputFile << temp.dump(4);
            outputFile.close();
            return true;
        }
        else {
            std::cout << "Не удалось открыть файл: \"tokenGenerator.json\"" << std::endl;
            std::cout << "Проверьте его расположенине и название!" << std::endl;
            return false;
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

	void CreateAccountRequest(nlohmann::json& data, const std::string& memo, const std::string& appid) {
        performPostRequest(data, "https://api.steampowered.com/IGameServersService/CreateAccount/v1/", "key=" + tokenSteam + "&appid=" + appid + "&memo=" + memo);
	}

    void GetAccountList(nlohmann::json& data) {
        performGetRequest(data, "https://api.steampowered.com/IGameServersService/GetAccountList/v1/?key=" + tokenSteam);
    }

    void SetMemo(const std::string& steamIdOfServer, const std::string& newMemo) {
        nlohmann::json data;
        performPostRequest(data, "https://api.steampowered.com/IGameServersService/SetMemo/v1/", "key=" + tokenSteam +
            "&steamid=" + steamIdOfServer + "&memo=" + newMemo);
    }

    void ResetLoginToken(const std::string& steamIdOfServer) {
        /*
            Possible answers:
                1) IF all good:
                    {"response":{"login_token":"new_token"}}
                2) IF token was recently regenerated:
                    {"response":{}}
                3) IF invalid token:
                    answer is trash
        */
        nlohmann::json data;
        performPostRequest(data, "https://api.steampowered.com/IGameServersService/ResetLoginToken/v1/",
        "key=" + tokenSteam + "&steamid=" + steamIdOfServer);

        if (data.contains("response") && data["response"].is_object()) {
            if (!data["response"].empty()) {
                if (UpdateTokenIdentificatorInJsonFile(steamIdOfServer, data["response"]["login_token"]))
                    std::cout << "Данные токена " << steamIdOfServer << " успешно обновлены!" << std::endl;
                else return;
            }
            else
                std::cout << "Вы не можете обновить токен " << steamIdOfServer << ", так как недавно обновляли его!" << std::endl;
        }
        else
            std::cout << "Произошла ошибка или Вы ввели неверные данные, попробуйте позже!" << std::endl;
    }

    bool UpdateTokenIdentificatorInJsonFile(const std::string& steamIdOfServer, const std::string& steamIdToken) {
        std::ifstream file("tokenGenerator.json");
        nlohmann::json data = nlohmann::json::parse(file);
        data["ServerTokens"][steamIdOfServer]["Identifier"] = steamIdToken;

        std::ofstream outputFile("tokenGenerator.json");
        if (outputFile.is_open()) {
            outputFile << data.dump(4);
            outputFile.close();
            return true;
        }
        else {
            std::cout << "Не удалось открыть файл: \"tokenGenerator.json\"" << std::endl;
            std::cout << "Проверьте его расположенине и название!" << std::endl;
            return false;
        }
    }

    void DeleteAccount(const std::string& steamIdOfServer) {
        nlohmann::json data;
        performPostRequest(data, "https://api.steampowered.com/IGameServersService/DeleteAccount/v1/",
            "key=" + tokenSteam + "&steamid=" + steamIdOfServer);
        --currentToken;
    }

    void getUselessTokens(std::vector<std::string>& vectorOfUselessTokens) {
        nlohmann::json data;
        GetAccountList(data);

        for (size_t i = 0; i < data["response"]["servers"].size(); ++i) {
            if (data["response"]["servers"][i]["is_expired"])
                vectorOfUselessTokens.push_back(data["response"]["servers"][i]["steamid"]);
            else continue;
        }
        size_t size = vectorOfUselessTokens.size();
        if (size)
            std::cout << "У Вас " << " неработоспособных токенов!" << std::endl;
        else
            std::cout << "У Вас нет неработоспособных токенов!" << std::endl;
    }

    void fixExpiredTokens() {
        std::vector<std::string> vectorOfUselessTokens;
        getUselessTokens(vectorOfUselessTokens);
        while (!vectorOfUselessTokens.empty()) {
            ResetLoginToken(vectorOfUselessTokens.back());
            vectorOfUselessTokens.pop_back();
        }
        reloadListofTokens();
    }

    void reloadListofTokens() {
        nlohmann::json data;
        nlohmann::json result;
        nlohmann::json temp;
        
        GetAccountList(data);
        
        size_t size = data["response"]["servers"].size();

        for (size_t i = 0; i < size; ++i) {
            std::string steamIDToken = data["response"]["servers"][i]["steamid"];
            temp["GameID"] = data["response"]["servers"][i]["appid"];
            temp["Identifier"] = data["response"]["servers"][i]["login_token"];
            temp["Memo"] = data["response"]["servers"][i]["memo"];
            result[steamIDToken] = temp;
            ++currentToken;
        }
        if (overwriteTokens(result)) {
            std::cout << "Токены сервера успешно синхронизированы с файлом JSON" << std::endl;
            std::cout << "У вас " << size << " токенов сервера." << std::endl;
        }
        else {
            std::cout << "Токены не были синхронизированы!" << std::endl;
        }
    }

    void ListOfTokensJSON() {
        std::ifstream file("tokenGenerator.json");
        nlohmann::json data = nlohmann::json::parse(file);
        for (auto it = data["ServerTokens"].begin(); it != data["ServerTokens"].end(); ++it) {
            std::cout << "MEMO: " << it.value()["Memo"] << std::endl;
            std::cout << "\tSTEAM ID TOKEN: " << it.value()["Identifier"] << std::endl;
            std::cout << "\tAPP ID: " << it.value()["GameID"] << std::endl;
            std::cout << "\tLOGIN TOKEN: " << it.key() << std::endl;
            std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
        }
        std::cout << "У вас " << data["ServerTokens"].size() << " токенов сервера." << std::endl;
    }

    void ListOfTokensSteam() {
        nlohmann::json data;
        GetAccountList(data);
        size_t size = data["response"]["servers"].size();
        for (size_t i = 0; i < size; ++i) {
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
            if (temp) {
                time_t lastLogonTime = static_cast<time_t>(temp);
                std::tm* utcTime = std::gmtime(&lastLogonTime);
                std::cout << asctime(utcTime) << "(UTC)" << std::endl;
            }
            else std::cout << "NEVER" << std::endl;
            std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
        }
        std::cout << "У вас " << size << " токенов сервера." << std::endl;
    }

};

enum class StageInput {
    START_PROGFRAMM,
    LIST_TOKES,
    CHECK_TOKENS,
    DELETE_TOKEN
};

class ControllerOfGenerator {
private:
    ServerTokenGenerate generator;
    StageInput stageInput = StageInput::START_PROGFRAMM;
public:
    ControllerOfGenerator(const std::string& steamToken) :
        generator(steamToken) {
        generator.reloadListofTokens();
    }

    void run() {
        std::string userInput;

        while (userInput != "exit") {

            if (stageInput == StageInput::START_PROGFRAMM) {
                std::cout << "1. Список токенов;" << std::endl;
                std::cout << "2. Проверить работоспособность всех токенов;" << std::endl;
                std::cout << "3. Удалить токен(ны);" << std::endl;
                std::cout << "4. Создать токен;" << std::endl;
                std::cout << "Завершить программу: \"exit\"" << std::endl;
                std::cout << "Введите необходимую цифру: ";
                std::getline(std::cin, userInput);
                if (userInput == "1") {
                    stageInput = StageInput::LIST_TOKES;
                    ListOfTokens();
                }
                else if (userInput == "2") {
                    std::vector<std::string> vectorOfUselessTokens;
                    generator.getUselessTokens(vectorOfUselessTokens);
                    if (vectorOfUselessTokens.size()) {
                        stageInput = StageInput::CHECK_TOKENS;
                        CheckTokens(vectorOfUselessTokens);
                    }
                }
                else if (userInput == "3") {
                    stageInput = StageInput::DELETE_TOKEN;
                    DeleteToken();
                }

            }
        }
    }
private:
    
    void DeleteToken() {
        std::string userInput;

        while (userInput != "4") {
            std::cout << "1. Получить список всех токенов STEAM;" << std::endl;
            std::cout << "2. Удалить конкертный токен;" << std::endl;
            std::cout << "3. Удалить все токены;" << std::endl;
            std::cout << "4. Вернуться обратно; " << std::endl;
            std::cout << "Введите необходимую цифру: ";

            std::getline(std::cin, userInput);

            if (userInput == "1") {

                generator.ListOfTokensJSON();
            }
            else if (userInput == "2")
                generator.ListOfTokensSteam();
            else if (userInput == "4") {
                stageInput = StageInput::START_PROGFRAMM;
                return;
            }
        }
    }

    void CheckTokens(std::vector<std::string>& vectorOfUselessTokens) {
        std::string userInput;

        while (userInput != "5") {
            std::cout << "1. Вывести все нерабочие токены;" << std::endl;
            std::cout << "2. Обновить все нерабочие токены;" << std::endl;
            std::cout << "3. Обновить конкертный нерабочий токен;" << std::endl;
            std::cout << "4. Удалить нерабочие токены;" << std::endl;
            std::cout << "5. Вернуться обратно; " << std::endl;
            std::cout << "Введите необходимую цифру: ";

            std::getline(std::cin, userInput);

            if (userInput == "1") {
                for (size_t i = 0; i < vectorOfUselessTokens.size(); ++i)
                    std::cout << i + 1 << ": " << vectorOfUselessTokens[i] << std::endl;
            }
            else if (userInput == "2") {
                for (size_t i = 0; i < vectorOfUselessTokens.size(); ++i)
                    generator.ResetLoginToken(vectorOfUselessTokens[i]);
            }
            else if (userInput == "3") {
                std::string getTokenSteamId;
                std::getline(std::cin, getTokenSteamId);
                for (size_t i = 0; i < vectorOfUselessTokens.size(); ++i) {
                    if (getTokenSteamId == vectorOfUselessTokens[i]) {
                        generator.ResetLoginToken(getTokenSteamId);
                        continue;
                    }
                }
                std::cout << "Вы ввели неверный токен: " << getTokenSteamId << std::endl;
            }
            else if (userInput == "4") {
                for (size_t i = 0; i < vectorOfUselessTokens.size(); ++i)
                    generator.DeleteAccount(vectorOfUselessTokens[i]);
            }
            else if (userInput == "5") {
                stageInput = StageInput::START_PROGFRAMM;
                return;
            }
        }
    }

    void ListOfTokens() {
        std::string userInput;

        while (userInput != "4") {
            std::cout << "1. Вывести список из JSON-файла;" << std::endl;
            std::cout << "2. Вывести список из STEAM;" << std::endl;
            std::cout << "3. Синхронизировать токены;" << std::endl;
            std::cout << "4. Вернуться обратно; " << std::endl;
            std::cout << "Введите необходимую цифру: ";

            std::getline(std::cin, userInput);

            if (userInput == "1")
                generator.ListOfTokensJSON();
            else if (userInput == "2")
                generator.ListOfTokensSteam();
            else if (userInput == "3")
                generator.reloadListofTokens();
            else if (userInput == "4") {
                stageInput = StageInput::START_PROGFRAMM;
                return;
            }
        }
    }
};

int main() {
    setlocale(LC_ALL, "rus");

	std::ifstream file("tokenGenerator.json");
	nlohmann::json data = nlohmann::json::parse(file);
	std::string steamToken = data["SteamToken"];

    //ControllerOfGenerator controller(steamToken);
    //controller.run();

    ServerTokenGenerate generator(steamToken);
    //generator.reloadListofTokens();
    generator.ResetLoginToken("asd");

	return 0;
}