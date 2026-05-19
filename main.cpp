#include "include/GameManager.h"
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#ifdef _WIN32
#include <windows.h>
#endif

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"

template <typename T>
T safeInput(const std::string &prompt, T minVal, T maxVal) {
  T val;
  while (true) {
    std::cout << prompt;
    if (std::cin >> val && val >= minVal && val <= maxVal) return val;
    std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cout << RED << "Invalid input." << RESET << "\n";
  }
}

std::shared_ptr<Player> createAI(int id, const std::string& arch, GameManager& manager, std::mt19937& gen) {
    auto& cfg = manager.archetypeConfigs[arch];
    std::uniform_real_distribution<float> sD(cfg.minSkill, cfg.maxSkill);
    
    float meanC = cfg.meanConfidence;
    std::normal_distribution<float> cD(meanC, 0.1f);
    
    float skill = std::max(0.0f, std::min(1.0f, sD(gen)));
    float conf = std::max(-1.0f, std::min(1.0f, cD(gen)));
    
    return std::make_shared<AIPlayer>("AI_" + std::to_string(id), 10000, skill, conf, cfg.greedThreshold, cfg.k, cfg.gamma, arch, gen());
}

struct SimParams { int nP; std::map<std::string, int> archCounts; };
SimParams getParams(bool isInter, const std::map<std::string, ArchetypeConfig>& configs) {
    SimParams p;
    
    std::cout << "\n" << BOLD << CYAN << "--- Player Archetype Selection ---" << RESET << "\n";
    int totalTarget = safeInput<int>("Total number of players (2-17): ", 2, 17);
    p.nP = totalTarget;
    
    int aiNeeded = totalTarget - (isInter ? 1 : 0);
    int remaining = aiNeeded;
    
    for (auto const& [name, cfg] : configs) {
        int count = safeInput<int>("Number of " + name + " (Max " + std::to_string(remaining) + "): ", 0, remaining);
        if (remaining > 0) {
            remaining -= count;
        } else {
            // If already full, ensure we don't accidentally assign a count if the user somehow bypassed validation
            count = 0; 
        }
        p.archCounts[name] = count;
    }

    if (remaining > 0) {
        p.archCounts["NORMAL"] += remaining;
        std::cout << YELLOW << "[System] Auto-filled " << remaining << " NORMAL players to reach total count." << RESET << "\n";
    }

    return p;
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
  while (true) {
    GameManager::clearScreen();
    std::cout << BOLD << BLUE << "========================================" << RESET << "\n";
    std::cout << BOLD << BLUE << "       BAI CAO CAI SIMULATOR v3.0       " << RESET << "\n";
    std::cout << BOLD << BLUE << "========================================" << RESET << "\n";
    std::cout << "1. Standard Simulation | 2. Interactive | 3. Random | 4. Log Mode | 0. Exit\nSelection: ";
    int choice; if (!(std::cin >> choice)) { std::cin.clear(); std::cin.ignore(1000, '\n'); continue; }
    if (choice == 0) break;
    GameManager manager; if (!manager.loadConfig("config.ini")) { std::cout << "Config error. Press Enter..."; std::cin.ignore(1000, '\n'); std::cin.get(); return 1; }

    unsigned seed;
    if (manager.simulationSeed >= 0) { seed = (unsigned)manager.simulationSeed; std::cout << "[System] Seed from config: " << seed << "\n"; }
    else {
        long long mSeed = safeInput<long long>("Enter seed (-1 for random): ", -1, 999999999999);
        seed = (mSeed == -1) ? std::chrono::system_clock::now().time_since_epoch().count() : (unsigned)mSeed;
        std::cout << "[System] Used seed: " << seed << "\n";
    }
    manager.simulationSeed = (long long)seed; std::mt19937 gen(seed);

    if (choice == 1 || choice == 2 || choice == 4) {
      bool isInter = (choice == 2), isLog = (choice == 4);
      int subMode = 1; if (choice == 1) subMode = safeInput<int>("1. Persistent | 2. Reset\nSelection: ", 1, 2);
      manager.sessionPrefix = std::to_string(choice) + "." + std::to_string(subMode) + "_";
      SimParams p = getParams(isInter, manager.archetypeConfigs);
      manager.displayArchetypeConfigs();
      std::vector<std::shared_ptr<Player>> players;
      if (isInter) { std::string hN; std::cout << "Enter your name: "; std::cin >> hN; players.push_back(std::make_shared<HumanPlayer>(hN, 10000, -1.0f, 0.0f, 0.0f)); }
      
      for (auto const& [archName, count] : p.archCounts) {
          for (int i = 0; i < count; ++i) {
              players.push_back(createAI(players.size() + 1, archName, manager, gen));
          }
      }
      manager.setPlayers(players); if (isInter || isLog) manager.logMode = true;
      std::cout << "\n" << BOLD << CYAN << "--- Player Configuration ---" << RESET << "\n";
      std::cout << std::left << std::setw(12) << "Player" 
                << std::setw(10) << "Type" 
                << std::setw(12) << "Archetype" 
                << std::setw(10) << "Skill" 
                << std::setw(12) << "Confidence" 
                << "Balance" << "\n";
      std::cout << "----------------------------------------------------------------------------\n";
      for (const auto& pl : manager.players) {
        std::cout << std::left << std::setw(12) << pl->getName() 
                  << std::setw(10) << (pl->isHumanPlayer() ? "Human" : "AI") 
                  << std::setw(12) << (pl->isHumanPlayer() ? "---" : pl->getArchetypeString())
                  << std::fixed << std::setprecision(2)
                  << std::setw(10) << pl->getSkillLevel() 
                  << std::setw(12) << pl->getConfidenceLevel() 
                  << pl->getBalance() << "\n";
      }
      
      std::cout << "\nEnable Research Reports (Data Export)? (y/n): ";
      char exT; std::cin >> exT; manager.autoExport = (exT == 'y' || exT == 'Y');

      bool simActive = true;
      while (simActive) {
        if (subMode == 2) { for (auto& pl : manager.players) pl->resetStats(); manager.roundCount = 0; }
        int numR = (isLog) ? 1 : safeInput<int>("Number of rounds: ", 1, 10000000);
        std::stringstream ss;
        ss << "Mode: " << (isInter ? "Interactive" : (isLog ? "Log Mode" : "Standard")) << "\n"
           << "Submode: " << (subMode == 1 ? "Persistent" : "Reset Every Round") << "\n"
           << "Used Seed: " << manager.simulationSeed << "\n"
           << "Players: " << p.nP << "\n"
           << "Distribution: ";
        for (auto const& [name, count] : p.archCounts) {
            if (count > 0) ss << name << ":" << count << " ";
        }
        ss << "\n" << "Rounds: " << numR;
        manager.simulationParams = ss.str();
        if (manager.autoExport) {
            manager.startStreaming();
            manager.logAIConfigs();
        }
        auto tS = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < numR; ++i) {
          if (subMode == 2) { for (auto& pl : manager.players) { pl->setBalance(10000); pl->setEliminated(false); } }
          int active = 0; for (auto& pl : manager.players) if (!pl->getIsEliminated()) active++;
          if (active <= 1) break;
          if (i == numR - 1) manager.isFinalRound = true;
          manager.playRound();
          if (!isInter && !isLog && ((i + 1) % 100 == 0 || i == numR - 1)) std::cout << "\rProgress: " << (i + 1) << " / " << numR << " (" << std::fixed << std::setprecision(1) << (float)(i+1)/numR*100.0f << "%)" << std::flush;
        }
        if (manager.autoExport) {
            manager.stopStreaming();
            manager.exportResearchReports();
        }
        std::cout << "\nDone in " << std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - tS).count() << "s.\n";
        manager.printSummary();
        if (isLog) { std::cout << "\n" << YELLOW << "[Log Mode] Press Enter to return to menu..." << RESET; std::cin.ignore(1000, '\n'); std::cin.get(); break; }
        std::cout << "\n1. Run more | 2. Menu\nSelection: "; if (safeInput<int>("", 1, 2) == 2) simActive = false;
      }
    } else if (choice == 3) {
      manager.sessionPrefix = "3.0_";
      int batches = safeInput<int>("Number of batches: ", 1, 1000);
      std::cout << "Enable Data Export? (y/n): "; char exT; std::cin >> exT; 
      manager.autoExport = (exT == 'y' || exT == 'Y');
      manager.isMode3 = true;

      if (manager.autoExport) {
          manager.startStreaming();
      }

      std::stringstream ss;
      ss << "Mode: Random (Continuous Research)\n"
         << "Used Seed: " << manager.simulationSeed << "\n"
         << "Batches: " << batches;
      manager.simulationParams = ss.str();

      std::cout << GREEN << "Running Random Mode (Continuous Research)..." << RESET << "\n";
      for (int b = 0; b < batches; ++b) {
          manager.currentBatchID = b + 1;
          int nP = std::uniform_int_distribution<int>(2, 17)(gen);
          std::vector<std::shared_ptr<Player>> players;
          for (int i = 0; i < nP; ++i) players.push_back(createAI(i + 1, "NORMAL", manager, gen));
          manager.setPlayers(players); 
          if (manager.autoExport) manager.logAIConfigs();
          manager.isFinalRound = true; 
          manager.playRound();
          if ((b + 1) % 10 == 0) std::cout << "\rBatch Progress: " << (b + 1) << " / " << batches << std::flush;
      }

      if (manager.autoExport) {
          manager.stopStreaming();
          manager.exportResearchReports();
      }
      std::cout << "\nDone. Press Enter..."; std::cin.ignore(1000, '\n'); std::cin.get();
    }
  }
  return 0;
}
