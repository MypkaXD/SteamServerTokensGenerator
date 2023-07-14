#pragma once

#include "ServerTokenGenerate.h"

enum class StageInput {
    START_PROGRAMM,
    LIST_TOKES,
    CHECK_TOKENS,
    DELETE_TOKEN,
    CREATE_TOKEN,
    RENAME_MEMO_TOKEN
};

class ControllerOfGenerator {
private:
    ServerTokenGenerate generator;
    StageInput stageInput = StageInput::START_PROGRAMM;
public:
    ControllerOfGenerator(const std::string& steamToken) :
        generator(steamToken) {
        generator.reloadListofTokens();
    }

    void run();

private:

    void RenameMemo();

    void CreateToken();

    void DeleteToken();

    void CheckTokens(std::vector<std::string>& vectorOfUselessTokens);

    void ListOfTokens();

};