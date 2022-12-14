#pragma once

#include <opencv2/opencv.hpp>
#include "BotNeuralNet.h"
#include "Systems.h"


//Rotations array, contains where a bot would look with every
//position of its head
const oPoint Rotations[] =
        {
                {0,  -1},
                {1,  -1},
                {1,  0},
                {1,  1},
                {0,  1},
                {-1, 1},
                {-1, 0},
                {-1, -1}
        };

enum EnergySource {
    PS,
    kills,
    mineral,
    ES_garbage,
    birth,
};
class Bot;
using TObject = Bot;
using t_object = std::unique_ptr<TObject>;
#define MAKE_TObj std::make_unique<TObject>

class DNK {
public:
    int max_energy;
    int def_front;
    int def_all;
    int kill_ability;
    int minerals_ability;
    int ps_ability;
    int mutability_body;
    int mutability_brain;
    int max_life_time;
    int fertilityDelay;
    int energy_given_on_birth;
    int move_ability_earth;
    int move_ability_sea;

    // TODO Terrain_move_ability and water_move_ability

    DNK() : max_energy(MaxPossibleEnergyForABot), def_front(0), def_all(0), minerals_ability(0),
            kill_ability(0), ps_ability(0), mutability_body(3), mutability_brain(10), max_life_time(MaxBotLifetime),
            fertilityDelay(0), energy_given_on_birth(EnergyPassedToAChild),
            move_ability_earth(0), move_ability_sea(0){}

    DNK &operator=(const DNK &dnk2);

    float distance(DNK &d2) const {
        int res{0};
        res += std::abs(1000 * (max_energy - d2.max_energy)) / (max_energy + d2.max_energy + 1);
        res += std::abs(1000 * (def_front - d2.def_front)) /
               (def_front + d2.def_front + 1);
        res += std::abs(1000 * (def_all - d2.def_all)) /
               (def_all + d2.def_all + 1);
        res += std::abs(1000 * (kill_ability - d2.kill_ability)) / (kill_ability + d2.kill_ability + 1);
        res += std::abs(1000 * (minerals_ability - d2.minerals_ability)) / (minerals_ability + d2.minerals_ability + 1);
        res += std::abs(1000 * (ps_ability - d2.ps_ability)) / (ps_ability + d2.ps_ability + 1);
        res += std::abs(1000 * (mutability_body - d2.mutability_body)) / (mutability_body + d2.mutability_body + 1);
        res += std::abs(1000 * (mutability_brain - d2.mutability_brain)) / (mutability_brain + d2.mutability_brain + 1);
        res += std::abs(1000 * (max_life_time - d2.max_life_time)) / (max_life_time + d2.max_life_time + 1);
        res += std::abs(1000 * (fertilityDelay - d2.fertilityDelay)) / (fertilityDelay + d2.fertilityDelay + 1);
        res += std::abs(1000 * (energy_given_on_birth - d2.energy_given_on_birth)) / (energy_given_on_birth + d2.energy_given_on_birth + 1);
        res += std::abs(1000 * (move_ability_earth - d2.move_ability_earth)) / (move_ability_earth + d2.move_ability_earth + 1);
        res += std::abs(1000 * (move_ability_sea - d2.move_ability_sea)) / (move_ability_sea + d2.move_ability_sea + 1);
        return static_cast<float>(res) / 1000 / 11;
    }

    void mutate(int d);

    [[maybe_unused]] [[nodiscard]] std::string descript() const;


};

//Bot class
class Bot {

    BotNeuralNet brain;
    int weight; // for motion capability

    //if this is not 0, bot does nothing at his turn
    int stunned;

    //How long a bot should wait to give birth after his own birth
    int fertilityDelay;

    cv::Scalar color{static_cast<double>(rand() % 250), static_cast<double>(rand() % 250),
                     static_cast<double>(rand() % 250)};

    cv::Scalar ab_color{static_cast<double>(rand() % 250), static_cast<double>(rand() % 250),
                     static_cast<double>(rand() % 250)};
    //Set random color
    void RandomizeColor();

    //Shift color a little (-10 to +10)
    void ChangeColor(int str, int pref_b, int pref_g, int pref_r);

    //Experimental
    [[maybe_unused]] void SlightlyMutate();

    //Main mutate function
    void Mutate();

    //Draw bot outline and his head
    void drawOutlineAndHead(frame_type &image, cv::Point &p1, cv::Point &p2) const;

public:
    //----------------------------------------------------------------------------------------------
    int lifetime{0};
    //Energy acquired from different sources
    unsigned long energyFromPS = 0;
    unsigned long energyFromKills = 0;
    unsigned long energyFromMinerals = 0;
    unsigned long energyFromOrganic = 0;

    unsigned long stat_steps = 0;
    unsigned long stat_kills = 0;
    unsigned long stat_birth = 0;
    unsigned long stat_ps = 0;

    // Energy acquired from different sources
    unsigned long step_energyBirth       = 0;
    unsigned long step_energyFromPS      = 0;
    unsigned long step_energyFromKills    = 0;
    unsigned long step_energyFromMinerals = 0;
    unsigned long step_energyFromOrganic = 0;

    unsigned long step_spend_front_def   = 0;
    unsigned long step_spend_front_all   = 0;
    unsigned long step_spend_mineral_ab  = 0;
    unsigned long step_spend_kill_ab     = 0;
    unsigned long step_spend_ps_ab       = 0;
    unsigned long step_spend_max_en      = 0;
    unsigned long step_spend_birth       = 0;
    unsigned long step_spend_attack      = 0;
    unsigned long step_spend_rotate      = 0;
    unsigned long step_spend_move        = 0;
    unsigned long step_move_ability_earth = 0;
    unsigned long step_move_ability_sea = 0;



    int direction{0};
public:

    DNK dnk;
    int energy;
    //Use neural network
    BrainOutput bots_ideas{};

    BrainOutput think(BrainInput input);

    //Bot tick function, it should always call parents tick function first
    int tick(Terrain terr);

    //Bot main draw function
    void draw(frame_type &image, int _xy, bool use_own_color);

    //Bot draw function is energy mode
    void drawEnergy(frame_type &image, int _xy) const;

    //Bot draw function is predators mode
    void drawPredators(frame_type &image, int _xy);

    //Change rotation function
    void Rotate(int dir = 1);

    //Give a bot some energy
    // when there are too much energy it goes to organic
    int GiveEnergy(int in_energy, EnergySource src);

    //Get rotation
    [[nodiscard]] oPoint GetDirection() const;

    //Take away bot energy, return true if 0 or below (bot dies)
    void TakeEnergy(int val);
    float FindKinship(t_object &stranger) const;

    explicit Bot(int Energy);
    //Inherit from a parent
    Bot(int Energy, t_object &prototype);



};


