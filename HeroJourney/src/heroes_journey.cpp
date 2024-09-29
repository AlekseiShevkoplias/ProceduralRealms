#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <memory>
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <sstream>
#include <chrono>

// Simple logging system
class Logger {
public:
    static void log(const std::string& message) {
        std::cout << "[" << getCurrentTimestamp() << "] " << message << std::endl;
    }

private:
    static std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
        return ss.str();
    }
};

// Enum for menu choices
enum class MenuChoice {
    SummonHero = 1,
    SendHeroOnQuest,
    ProgressTime,
    Exit
};

// Class definitions
class Hero {
public:
    std::string name;
    std::string race;
    int INT, WIS, STR, DEX;
    int level;
    int experience;
    int gold;

    Hero(std::string n, std::string r, int i, int w, int s, int d, int lvl, int exp, int g)
        : name(std::move(n)), race(std::move(r)), INT(i), WIS(w), STR(s), DEX(d), level(lvl), experience(exp), gold(g) {}

    void display() const {
        Logger::log("Hero: " + name + " (" + race + ")");
        Logger::log("Level: " + std::to_string(level) + ", EXP: " + std::to_string(experience) + ", Gold: " + std::to_string(gold));
        Logger::log("INT: " + std::to_string(INT) + ", WIS: " + std::to_string(WIS) + ", STR: " + std::to_string(STR) + ", DEX: " + std::to_string(DEX));
    }
};

class Obstacle {
public:
    std::string name;
    int difficulty;
    int STR_check;
    int reward_gold;

    Obstacle(std::string n, int diff, int str_ch, int reward_g)
        : name(std::move(n)), difficulty(diff), STR_check(str_ch), reward_gold(reward_g) {}

    void display() const {
        Logger::log("Quest: " + name);
        Logger::log("Difficulty: " + std::to_string(difficulty));
        Logger::log("STR Check: " + std::to_string(STR_check));
        Logger::log("Reward: Gold " + std::to_string(reward_gold));
    }
};

class Quest {
public:
    std::shared_ptr<Obstacle> obstacle;
    int remaining_turns;
    std::shared_ptr<Hero> assigned_hero;

    Quest(std::shared_ptr<Obstacle> obs, int turns)
        : obstacle(std::move(obs)), remaining_turns(turns), assigned_hero(nullptr) {}

    void progress() {
        if (remaining_turns > 0) {
            remaining_turns--;
            Logger::log("Quest in progress... Turns remaining: " + std::to_string(remaining_turns));
        } else {
            Logger::log("Quest completed: " + obstacle->name);
            if (assigned_hero) {
                assigned_hero->gold += obstacle->reward_gold;
                assigned_hero->experience += obstacle->difficulty * 10;
                Logger::log(assigned_hero->name + " gained " + std::to_string(obstacle->reward_gold) + " gold and " + 
                            std::to_string(obstacle->difficulty * 10) + " experience.");
            }
        }
    }

    void assign_hero(std::shared_ptr<Hero> hero) {
        assigned_hero = std::move(hero);
        Logger::log(assigned_hero->name + " assigned to quest: " + obstacle->name);
    }
};

class Game {
private:
    std::vector<std::shared_ptr<Hero>> heroes;
    std::vector<std::shared_ptr<Obstacle>> obstacles;
    std::vector<std::shared_ptr<Quest>> quest_log;
    std::mt19937 rng;

public:
    Game() : rng(std::random_device{}()) {}

    void load_data(const std::string& heroes_file, const std::string& obstacles_file) {
        try {
            load_heroes(heroes_file);
            load_obstacles(obstacles_file);
        } catch (const YAML::Exception& e) {
            Logger::log("Error loading YAML file: " + std::string(e.what()));
            throw;
        } catch (const std::exception& e) {
            Logger::log("Error: " + std::string(e.what()));
            throw;
        }
    }

    void run() {
        while (true) {
            display_menu();
            int choice;
            std::cin >> choice;

            switch (static_cast<MenuChoice>(choice)) {
                case MenuChoice::SummonHero:
                    summon_hero();
                    break;
                case MenuChoice::SendHeroOnQuest:
                    send_hero_on_quest();
                    break;
                case MenuChoice::ProgressTime:
                    progress_time();
                    break;
                case MenuChoice::Exit:
                    return;
                default:
                    Logger::log("Invalid choice. Please try again.");
            }
        }
    }

private:
    void load_heroes(const std::string& filename) {
        YAML::Node data = YAML::LoadFile(filename);
        for (const auto& hero_node : data["heroes"]) {
            auto h = std::make_shared<Hero>(
                hero_node["name"].as<std::string>(),
                hero_node["race"].as<std::string>(),
                hero_node["INT"].as<int>(),
                hero_node["WIS"].as<int>(),
                hero_node["STR"].as<int>(),
                hero_node["DEX"].as<int>(),
                hero_node["level"].as<int>(),
                hero_node["experience"].as<int>(),
                hero_node["gold"].as<int>()
            );
            heroes.push_back(h);
        }
    }

    void load_obstacles(const std::string& filename) {
        YAML::Node data = YAML::LoadFile(filename);
        for (const auto& obs_node : data["obstacles"]) {
            auto o = std::make_shared<Obstacle>(
                obs_node["name"].as<std::string>(),
                obs_node["difficulty"].as<int>(),
                obs_node["STR_check"].as<int>(),
                obs_node["reward"]["gold"].as<int>()
            );
            obstacles.push_back(o);
        }
    }

    void display_menu() const {
        Logger::log("1. Summon Hero");
        Logger::log("2. Send Hero on Quest");
        Logger::log("3. Progress Time");
        Logger::log("4. Exit");
    }

    void summon_hero() {
        if (heroes.empty()) {
            Logger::log("No heroes available.");
            return;
        }
        std::uniform_int_distribution<> dis(0, heroes.size() - 1);
        int rand_hero_idx = dis(rng);
        Logger::log("Summoned Hero:");
        heroes[rand_hero_idx]->display();
    }

    void send_hero_on_quest() {
        if (obstacles.empty()) {
            Logger::log("No obstacles available.");
            return;
        }
        std::uniform_int_distribution<> obs_dis(0, obstacles.size() - 1);
        std::uniform_int_distribution<> turns_dis(2, 5);
        int rand_obs_idx = obs_dis(rng);
        int turns = turns_dis(rng);
        
        auto quest = std::make_shared<Quest>(obstacles[rand_obs_idx], turns);
        Logger::log("Generated Quest:");
        quest->obstacle->display();
        quest_log.push_back(quest);

        if (!heroes.empty()) {
            std::uniform_int_distribution<> hero_dis(0, heroes.size() - 1);
            int rand_hero_idx = hero_dis(rng);
            quest->assign_hero(heroes[rand_hero_idx]);
        }
    }

    void progress_time() {
        for (auto& q : quest_log) {
            q->progress();
        }
        // Remove completed quests
        quest_log.erase(
            std::remove_if(quest_log.begin(), quest_log.end(),
                           [](const std::shared_ptr<Quest>& q) { return q->remaining_turns <= 0; }),
            quest_log.end());
    }
};

int main() {
    try {
        Game game;
        game.load_data("../resources/heroes.yaml", "../resources/foes.yaml");
        game.run();
    } catch (const std::exception& e) {
        Logger::log("Fatal error: " + std::string(e.what()));
        return 1;
    }
    return 0;
}
