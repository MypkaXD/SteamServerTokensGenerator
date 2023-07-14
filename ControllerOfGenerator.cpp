#include "ControllerOfGenerator.h"

void ControllerOfGenerator::RenameMemo() {
    std::string steamIDToken;
    std::string newMemo;

    std::cout << "Enter the new description for the token: ";
    std::getline(std::cin, newMemo);
    std::cout << "Enter the Steam ID of the token: ";
    std::getline(std::cin, steamIDToken);
    generator.SetMemo(steamIDToken, newMemo);
    stageInput = StageInput::START_PROGRAMM;
}

void ControllerOfGenerator::CreateToken() {
    std::string inputString;

    std::cout << "Enter the desired number of tokens to create: ";
    std::getline(std::cin, inputString);
    generator.makeServerToken(std::stoi(inputString));
    stageInput = StageInput::START_PROGRAMM;
    return;
}

void ControllerOfGenerator::DeleteToken() {
    std::string userInput;

    while (userInput != "4") {
        std::cout << "1. Get a list of all STEAM tokens;" << std::endl;
        std::cout << "2. Delete a specific token;" << std::endl;
        std::cout << "3. Delete all tokens;" << std::endl;
        std::cout << "4. Go back;" << std::endl;
        std::cout << "Enter the corresponding number: ";

        std::getline(std::cin, userInput);

        if (userInput == "1")
            generator.ListOfTokensJSON();
        else if (userInput == "2") {
            std::cout << "Enter the STEAM ID of the token: ";
            std::string steamID;
            std::getline(std::cin, steamID);
            generator.DeleteAccount(steamID);
        }
        else if (userInput == "3") {
            std::ifstream file("tokenGenerator.json");
            nlohmann::json data = nlohmann::json::parse(file);
            for (auto it = data["ServerTokens"].begin(); it != data["ServerTokens"].end(); ++it)
                generator.DeleteAccount(it.key());
        }
        else if (userInput == "4") {
            stageInput = StageInput::START_PROGRAMM;
            return;
        }
    }
}

void ControllerOfGenerator::CheckTokens(std::vector<std::string>& vectorOfUselessTokens) {
    std::string userInput;

    while (userInput != "5") {
        std::cout << "1. Print all dysfunctional tokens;" << std::endl;
        std::cout << "2. Update all dysfunctional tokens;" << std::endl;
        std::cout << "3. Update a specific dysfunctional token;" << std::endl;
        std::cout << "4. Delete dysfunctional tokens;" << std::endl;
        std::cout << "5. Go back;" << std::endl;
        std::cout << "Enter the corresponding number: ";

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
            std::cout << "You entered an incorrect token: " << getTokenSteamId << std::endl;
        }
        else if (userInput == "4") {
            for (size_t i = 0; i < vectorOfUselessTokens.size(); ++i)
                generator.DeleteAccount(vectorOfUselessTokens[i]);
        }
        else if (userInput == "5") {
            stageInput = StageInput::START_PROGRAMM;
            return;
        }
    }
}

void ControllerOfGenerator::ListOfTokens() {
    std::string userInput;

    while (userInput != "4") {
        std::cout << "1. Print the list from the JSON file;" << std::endl;
        std::cout << "2. Print the list from STEAM;" << std::endl;
        std::cout << "3. Synchronize the tokens;" << std::endl;
        std::cout << "4. Go back;" << std::endl;
        std::cout << "Enter the corresponding number: ";

        std::getline(std::cin, userInput);

        if (userInput == "1")
            generator.ListOfTokensJSON();
        else if (userInput == "2")
            generator.ListOfTokensSteam();
        else if (userInput == "3")
            generator.reloadListofTokens();
        else if (userInput == "4") {
            stageInput = StageInput::START_PROGRAMM;
            return;
        }
    }
}

void ControllerOfGenerator::run() {
    std::string userInput;

    while (userInput != "exit") {

        if (stageInput == StageInput::START_PROGRAMM) {
            std::cout << "1. Token list;" << std::endl;
            std::cout << "2. Check the functionality of all tokens;" << std::endl;
            std::cout << "3. Delete token(s);" << std::endl;
            std::cout << "4. Create a token;" << std::endl;
            std::cout << "5. Rename the token memo;" << std::endl;
            std::cout << "Exit the program: \"exit\"" << std::endl;
            std::cout << "Enter the corresponding number: ";
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
            else if (userInput == "4") {
                stageInput = StageInput::CREATE_TOKEN;
                CreateToken();
            }
            else if (userInput == "5") {
                stageInput == StageInput::RENAME_MEMO_TOKEN;
                RenameMemo();
            }
        }
    }
}