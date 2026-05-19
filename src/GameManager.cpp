#include "../include/GameManager.h"
#include "../include/ConcreteStates.h"

#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

GameManager::GameManager() : currentPot(0), currentState(nullptr), currentDealerIndex(0), isStreaming(false) {
#ifdef _WIN32
    _mkdir("data");
#else
    mkdir("data", 0777);
#endif
}

GameManager::~GameManager() {
    if (isStreaming) stopStreaming();
    // Smart pointers handle deletion
}

void GameManager::setPlayers(const std::vector<std::shared_ptr<Player>>& p) {
    players = p;
    if (!players.empty()) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(0, (int)players.size() - 1);
        
        currentDealerIndex = dist(gen);
        players[currentDealerIndex]->isDealer = true;
    }

    initialBalances.clear();
    for (auto& pl : players) {
        initialBalances.push_back(pl->getBalance());
    }
}

void GameManager::changeState(std::unique_ptr<GameState> newState) {
    currentState = std::move(newState);
}

void GameManager::playRound() {
    roundCount++;
    for (auto& p : players) {
        if (!p->isEliminated) {
            p->roundsPlayed++;
        }
        p->bankrollHistory.push_back(p->balance);
    }
    changeState(StateFactory::getBettingState());
    
    // The states will transition: Betting -> Dealing -> Trading -> Eval -> nullptr
    while (currentState != nullptr) {
        currentState->update(this);
    }
}

void GameManager::startStreaming() {
    currentSessionTS = sessionPrefix + getTimestamp();
    streamSwap.open("data/" + currentSessionTS + "_swap_decisions.csv");
    streamRound.open("data/" + currentSessionTS + "_rounds_summary.csv");
    streamHistory.open("data/" + currentSessionTS + "_bankroll_history.csv");
    
    streamAIConfigs.open("data/" + currentSessionTS + "_ai_configs.csv");
    
    if (isMode3) {
        streamAIConfigs << "Batch,PlayerName,Archetype,Skill,Confidence,InitialBalance\n";
        streamSwap << "Batch,RoundID,PlayerName,SwapTurn,Satisfaction,Desire,Probability,Decision,ScoreBefore,ScoreAfter,CardOut,CardIn\n";
        streamRound << "Batch,RoundNum,Dealer,Pot,WinnersCount,ScoresSummary\n";
        streamHistory << "Batch,Round,PlayerName,Balance\n";
    } else {
        streamAIConfigs << "PlayerName,Archetype,Skill,Confidence,InitialBalance\n";
        streamSwap << "RoundID,PlayerName,SwapTurn,Satisfaction,Desire,Probability,Decision,ScoreBefore,ScoreAfter,CardOut,CardIn\n";
        streamRound << "RoundNum,Dealer,Pot,WinnersCount,ScoresSummary\n";
        streamHistory << "Round";
        for(auto& p : players) streamHistory << "," << p->getName();
        streamHistory << "\n";
    }
    isStreaming = true;
}

void GameManager::stopStreaming() {
    if (streamSwap.is_open()) streamSwap.close();
    if (streamRound.is_open()) streamRound.close();
    if (streamHistory.is_open()) streamHistory.close();
    if (streamAIConfigs.is_open()) streamAIConfigs.close();
    isStreaming = false;
}

std::string GameManager::getTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    struct tm *parts = std::localtime(&now_c);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", parts);
    
    std::string ts(buf);
    char ms_buf[8];
    std::snprintf(ms_buf, sizeof(ms_buf), "_%03d", (int)ms.count());
    return ts + ms_buf;
}

void GameManager::logAIConfigs() {
    if (!isStreaming) return;
    if (!streamAIConfigs.is_open()) return;
    for (const auto& p : players) {
        if (isMode3) streamAIConfigs << currentBatchID << ",";
        streamAIConfigs << p->getName() << ","
                        << p->getArchetypeString() << ","
                        << p->getSkillLevel() << ","
                        << p->getConfidenceLevel() << ","
                        << p->getBalance() << "\n";
    }
    streamAIConfigs.flush();
}

void GameManager::printSummary() {
    std::cout << "\n" << BOLD << BLUE << "=========================================================================================================" << RESET << "\n";
    std::cout << BOLD << BLUE << "                                         FINAL SIMULATION REPORT                                         " << RESET << "\n";
    std::cout << BOLD << BLUE << "=========================================================================================================" << RESET << "\n";
    std::cout << std::left << std::setw(12) << "Player" 
              << std::setw(10) << "Skill" 
              << std::setw(12) << "Balance" 
              << std::setw(10) << "Wins" 
              << std::setw(10) << "Rounds" 
              << std::setw(12) << "Win Rate" 
              << std::setw(25) << "Last Hand" 
              << "Score" << "\n";
    std::cout << "---------------------------------------------------------------------------------------------------------\n";
    
    for (auto& p : players) {
        float winRate = (p->roundsPlayed > 0) ? (float)p->wins / p->roundsPlayed * 100.0f : 0.0f;
        std::cout << std::left << std::setw(12) << p->getName()
                  << std::fixed << std::setprecision(2) << std::setw(10) << p->getSkillLevel()
                  << std::setw(12) << p->getBalance()
                  << std::setw(10) << p->wins
                  << std::setw(10) << p->roundsPlayed;
        std::stringstream ss;
        ss << std::fixed << std::setprecision(1) << winRate << "%";
        std::cout << std::left << std::setw(12) << ss.str();
        std::cout << p->lastHand;
        int visibleLen = p->lastHandPlain.length();
        int pad = std::max(0, 25 - visibleLen);
        std::cout << std::string(pad, ' ') << p->lastScore << "\n";
    }
    std::cout << BOLD << BLUE << "=========================================================================================================" << RESET << "\n";
}

void GameManager::exportResearchReports() {
    std::string filename = "data/" + currentSessionTS + "_summary.txt";
    std::ofstream out(filename);
    if (!out.is_open()) return;

    out << "SIMULATION RESEARCH SUMMARY\n";
    out << "Timestamp: " << currentSessionTS << "\n";
    out << "Parameters:\n" << simulationParams << "\n\n";
    out << "Final Results:\n";
    out << std::left << std::setw(12) << "Player" << std::setw(10) << "Skill" << std::setw(12) << "Balance" << "\n";
    for (auto& p : players) {
        out << std::left << std::setw(12) << p->getName() << std::setw(10) << p->getSkillLevel() << std::setw(12) << p->getBalance() << "\n";
    }
    out.close();

    std::ofstream tiltOut("data/" + currentSessionTS + "_tilt_events.log");
    tiltOut << "--- AI TILT LOG ---\n";
    tiltOut << "Used Seed: " << (simulationSeed >= 0 ? std::to_string(simulationSeed) : "Random") << "\n";
    tiltOut << "-------------------\n";
    if (tiltLogs.empty()) tiltOut << "No tilt events recorded.\n";
    else {
        for (const auto& log : tiltLogs) tiltOut << log << "\n";
    }
    tiltOut.close();
}

void GameManager::saveInitialState() {
    // Already handled in startStreaming for history file header
}

bool GameManager::loadConfig(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "\n" << BOLD << RED << "[ERROR] CRITICAL: Could not open " << filename << RESET << "\n";
        std::cerr << YELLOW << "The simulation requires " << filename << " to define AI behavioral models.\n";
        std::cerr << "Please ensure the file is in the same directory as the executable." << RESET << "\n\n";
        return false;
    }
    
    std::string line, currentSection;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#' || line[0] == ';') continue;
        if (line[0] == '[' && line.back() == ']') {
            currentSection = line.substr(1, line.size() - 2);
            continue;
        }
        
        size_t eqPos = line.find('=');
        if (eqPos == std::string::npos) continue;
        
        std::string key = line.substr(0, eqPos);
        std::string valStr = line.substr(eqPos + 1);
        float value = 0;
        try { value = std::stof(valStr); } catch (...) {}
        
        if (currentSection == "GLOBAL") {
            if (key == "seed") simulationSeed = (long long)value;
            else if (key == "enable_tilt") {
                if (valStr == "true" || valStr == "1") enableTilt = true;
                else if (valStr == "false" || valStr == "0") enableTilt = false;
            }
            continue;
        }

        // Dynamic Archetype loading
        if (key == "k") archetypeConfigs[currentSection].k = value;
        else if (key == "gamma") archetypeConfigs[currentSection].gamma = value;
        else if (key == "greed_threshold") archetypeConfigs[currentSection].greedThreshold = value;
        else if (key == "min_skill") archetypeConfigs[currentSection].minSkill = value;
        else if (key == "max_skill") archetypeConfigs[currentSection].maxSkill = value;
        else if (key == "mean_confidence") archetypeConfigs[currentSection].meanConfidence = value;
    }
    std::cout << "[Config] Successfully loaded archetype personalities from " << filename << "\n";
    return true;
}

void GameManager::displayArchetypeConfigs() {
    std::cout << BOLD << CYAN << "\n--- AI BEHAVIORAL PROFILES (From Config) ---" << RESET << "\n";
    std::cout << std::left << std::setw(10) << "Type" 
              << std::setw(10) << "k" 
              << std::setw(10) << "gamma" 
              << std::setw(10) << "Greed" 
              << std::setw(10) << "Conf"
              << std::setw(15) << "Skill Range" << "\n";
    std::cout << "------------------------------------------------------------------\n";
    
    for (auto const& [name, cfg] : archetypeConfigs) {
        std::cout << std::left << std::setw(10) << name 
                  << std::fixed << std::setprecision(2)
                  << std::setw(10) << cfg.k 
                  << std::setw(10) << cfg.gamma 
                  << std::setw(10) << cfg.greedThreshold
                  << std::setw(10) << cfg.meanConfidence
                  << "[" << cfg.minSkill << " - " << cfg.maxSkill << "]\n";
    }
    std::cout << "------------------------------------------------------------------\n\n";
}

void GameManager::clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    std::cout << "\033[2J\033[1;1H";
#endif
}
