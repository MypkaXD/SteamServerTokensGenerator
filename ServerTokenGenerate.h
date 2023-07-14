#pragma once

#include <nlohmann/json.hpp>
#include <curl/curl.h>

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <ctime>

// Функция обратного вызова для обработки полученных данных от CURL
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response);

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

    void makeServerToken(int count);

    bool overwriteTokens(const nlohmann::json& result);

    bool writeTokens(const nlohmann::json& result);

    // Функция для выполнения GET-запроса
    void performGetRequest(nlohmann::json& data, const std::string& requestURL);

    // Функция для выполнения POST-запроса
    void performPostRequest(nlohmann::json& data, const std::string& requestURL, const std::string& dataURL);

    void CreateAccountRequest(nlohmann::json& data, const std::string& memo, const std::string& appid);

    void GetAccountList(nlohmann::json& data);

    void SetMemo(const std::string& steamIdOfServer, const std::string& newMemo);

    void ResetLoginToken(const std::string& steamIdOfServer);

    bool UpdateMemoInJsonFile(const std::string& steamIfOfServer, const std::string& newMemo);

    bool DeleteTokenFromJsonFile(const std::string& steamIdOfServer);

    bool UpdateTokenIdentificatorInJsonFile(const std::string& steamIdOfServer, const std::string& steamIdToken);

    void DeleteAccount(const std::string& steamIdOfServer);

    void getUselessTokens(std::vector<std::string>& vectorOfUselessTokens);

    void fixExpiredTokens();

    void reloadListofTokens();

    void ListOfTokensJSON();

    void ListOfTokensSteam();

};