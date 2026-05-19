#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include <vector>
#include <map>
#include <memory>
#include "Player.h"
#include "Deck.h"
#include "GameState.h"
#include <fstream>

struct RoundRecord {
    int roundNum;
    std::string dealerName;
    int pot;
    int winnersCount;
    std::string scoresSummary;
};

struct ArchetypeConfig {
    float k = 1.0f;
    float gamma = 2.0f;
    float greedThreshold = 0.5f;
    float minSkill = 0.0f;
    float maxSkill = 1.0f;
    float meanConfidence = 0.0f;
};


class GameManager {
public:
    std::vector<std::shared_ptr<Player>> players;
    int currentPot;
    std::unique_ptr<GameState> currentState;
    Deck deck;
    int currentDealerIndex;
    int roundCount = 0;

    // Streaming file handles for integrity and RAM optimization
    bool isStreaming = false;
    std::ofstream streamSwap;
    std::ofstream streamRound;
    std::ofstream streamHistory;
    std::string currentSessionTS;
    bool isMode3 = false;
    int currentBatchID = 0;
    std::ofstream streamAIConfigs;

    std::vector<std::string> tiltLogs;

    GameManager();
    ~GameManager();
    void setPlayers(const std::vector<std::shared_ptr<Player>>& p);
    void changeState(std::unique_ptr<GameState> newState);
    void playRound();
    void startStreaming();
    void stopStreaming();
    std::string getTimestamp();
    void exportResearchReports(); // Now handles Summary and Tilt logs
    void printSummary();
    void saveInitialState();
    void logAIConfigs();

    static void clearScreen();

    bool logMode = false;
    bool autoExport = false;
    bool isFinalRound = false;
    std::string simulationParams;
    long long simulationSeed = -1;
    bool enableTilt = true;
    std::string sessionPrefix = "";
    std::vector<int> initialBalances;
    void initRNG(); // Initialize RNG with simulationSeed if >= 0

    std::map<std::string, ArchetypeConfig> archetypeConfigs;

    bool loadConfig(const std::string& filename);
    void displayArchetypeConfigs();

};

#endif // GAMEMANAGER_H
