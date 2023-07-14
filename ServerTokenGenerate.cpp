#include "ServerTokenGenerate.h"

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
    size_t totalSize = size * nmemb;
    response->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

void ServerTokenGenerate::makeServerToken(int count) {
    if (count <= 0) {
        std::cout << "The number of created tokens is incorrectly set" << std::endl;
        return;
    }
    if (currentToken + count > 1000) {
        std::cout << "You have " << currentToken << "token(s)" <<
            "\nMax tokens are 1000\nYou're trying to have " << currentToken + count <<
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
        std::cout << "Token " << steamidtoken << " has been successfully created." << std::endl;
    }
    if (writeTokens(result))
        std::cout << "Token has been successfully written to the JSON file." << std::endl;
    else std::cout << "Token was not written to the JSON file." << std::endl;
}

bool ServerTokenGenerate::overwriteTokens(const nlohmann::json& result) {
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
        std::cout << "Failed to open file: \"tokenGenerator.json\"" << std::endl;
        std::cout << "Please check its location and name!" << std::endl;
        return false;
    }
}

bool ServerTokenGenerate::writeTokens(const nlohmann::json& result) {
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
        std::cout << "Failed to open the file: \"tokenGenerator.json\"" << std::endl;
        std::cout << "Please check its location and name!" << std::endl;
        return false;
    }
}

void ServerTokenGenerate::performGetRequest(nlohmann::json& data, const std::string& requestURL) {
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

void ServerTokenGenerate::performPostRequest(nlohmann::json& data, const std::string& requestURL, const std::string& dataURL) {
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

void ServerTokenGenerate::CreateAccountRequest(nlohmann::json& data, const std::string& memo, const std::string& appid) {
    performPostRequest(data, "https://api.steampowered.com/IGameServersService/CreateAccount/v1/", "key=" + tokenSteam + "&appid=" + appid + "&memo=" + memo);
}

void ServerTokenGenerate::GetAccountList(nlohmann::json& data) {
    performGetRequest(data, "https://api.steampowered.com/IGameServersService/GetAccountList/v1/?key=" + tokenSteam);
}

void ServerTokenGenerate::SetMemo(const std::string& steamIdOfServer, const std::string& newMemo) {
    nlohmann::json data;
    performPostRequest(data, "https://api.steampowered.com/IGameServersService/SetMemo/v1/", "key=" + tokenSteam +
        "&steamid=" + steamIdOfServer + "&memo=" + newMemo);
    if (data.contains("response") && data["response"].is_object() && data["response"].empty()) {
        if (UpdateMemoInJsonFile(steamIdOfServer, newMemo)) {
            std::cout << "Token description for " << steamIdOfServer << " has been successfully updated to " << newMemo << "!" << std::endl; --currentToken;
        }
    }
    else
        std::cout << "An error occurred or you entered incorrect data. Please try again later!" << std::endl;
}

void ServerTokenGenerate::ResetLoginToken(const std::string& steamIdOfServer) {
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
                std::cout << "Token data for " << steamIdOfServer << " has been successfully updated!" << std::endl;
            else return;
        }
        else
            std::cout << "You cannot update the token for " << steamIdOfServer << " as it was recently updated!" << std::endl;
    }
    else
        std::cout << "An error occurred or you entered incorrect data. Please try again later!" << std::endl;
}

bool ServerTokenGenerate::UpdateMemoInJsonFile(const std::string& steamIfOfServer, const std::string& newMemo) {
    std::ifstream file("tokenGenerator.json");
    nlohmann::json data = nlohmann::json::parse(file);
    data["ServerTokens"][steamIfOfServer]["Memo"] = newMemo;

    std::ofstream outputFile("tokenGenerator.json");
    if (outputFile.is_open()) {
        outputFile << data.dump(4);
        outputFile.close();
        return true;
    }
    else {
        std::cout << "Failed to open the file: \"tokenGenerator.json\"" << std::endl;
        std::cout << "Please check its location and name!" << std::endl;
        return false;
    }
}

bool ServerTokenGenerate::DeleteTokenFromJsonFile(const std::string& steamIdOfServer) {
    std::ifstream file("tokenGenerator.json");
    nlohmann::json data = nlohmann::json::parse(file);
    data["ServerTokens"].erase(steamIdOfServer);

    std::ofstream outputFile("tokenGenerator.json");
    if (outputFile.is_open()) {
        outputFile << data.dump(4);
        outputFile.close();
        return true;
    }
    else {
        std::cout << "Failed to open the file: \"tokenGenerator.json\"" << std::endl;
        std::cout << "Please check its location and name!" << std::endl;
        return false;
    }
}

bool ServerTokenGenerate::UpdateTokenIdentificatorInJsonFile(const std::string& steamIdOfServer, const std::string& steamIdToken) {
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
        std::cout << "Failed to open the file: \"tokenGenerator.json\"" << std::endl;
        std::cout << "Please check its location and name!" << std::endl;
        return false;
    }
}

void ServerTokenGenerate::DeleteAccount(const std::string& steamIdOfServer) {
    /*
        Possible answers:
            1) IF all good:
                {"response":{}}
            2) IF invalid token:
                answer is trash
    */

    nlohmann::json data;

    performPostRequest(data, "https://api.steampowered.com/IGameServersService/DeleteAccount/v1/",
        "key=" + tokenSteam + "&steamid=" + steamIdOfServer);
    if (data.contains("response") && data["response"].is_object() && data["response"].empty()) {
        if (DeleteTokenFromJsonFile(steamIdOfServer)) {
            std::cout << "Token " << steamIdOfServer << " has been successfully deleted!" << std::endl;
            --currentToken;
        }
    }
    else
        std::cout << "An error occurred or you entered incorrect data. Please try again later!" << std::endl;
}

void ServerTokenGenerate::getUselessTokens(std::vector<std::string>& vectorOfUselessTokens) {
    nlohmann::json data;
    GetAccountList(data);

    for (size_t i = 0; i < data["response"]["servers"].size(); ++i) {
        if (data["response"]["servers"][i]["is_expired"])
            vectorOfUselessTokens.push_back(data["response"]["servers"][i]["steamid"]);
        else continue;
    }
    size_t size = vectorOfUselessTokens.size();
    if (size)
        std::cout << "You have " << size << " dysfunctional tokens!" << std::endl;
    else
        std::cout << "You have no dysfunctional tokens!" << std::endl;
}

void ServerTokenGenerate::fixExpiredTokens() {
    std::vector<std::string> vectorOfUselessTokens;
    getUselessTokens(vectorOfUselessTokens);
    while (!vectorOfUselessTokens.empty()) {
        ResetLoginToken(vectorOfUselessTokens.back());
        vectorOfUselessTokens.pop_back();
    }
    reloadListofTokens();
}

void ServerTokenGenerate::reloadListofTokens() {
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
    }
    currentToken = size;
    if (overwriteTokens(result)) {
        std::cout << "Server tokens have been successfully synchronized with the JSON file." << std::endl;
        std::cout << "You have " << size << " server tokens." << std::endl;
    }
    else {
        std::cout << "Tokens were not synchronized!" << std::endl;
    }
}

void ServerTokenGenerate::ListOfTokensJSON() {
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

void ServerTokenGenerate::ListOfTokensSteam() {
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
    std::cout << "You have " << size << " server tokens." << std::endl;
}
