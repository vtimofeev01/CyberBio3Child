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
};
class Bot;
//using t_object = Bot;
using TObject = Bot;
//using t_object = std::shared_ptr<TObject>;
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
    // TODO Terrain_move_ability and water_move_ability

    DNK() : max_energy(MaxPossibleEnergyForABot), def_front(0), def_all(0), minerals_ability(0),
            kill_ability(0), ps_ability(0), mutability_body(3), mutability_brain(10), max_life_time(MaxBotLifetime),
            fertilityDelay(0){}

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
        return static_cast<float>(res) / 1000 / 9;
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

    //Set random color
    void RandomizeColor();

    //Shift color a little (-10 to +10)
    void ChangeColor(int str = 10);

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
    unsigned long stat_moves = 0;

    int direction{0};
public:

    DNK dnk;
    int energy;
    //Use neural network
    BrainOutput bots_ideas{};

    BrainOutput think(BrainInput input);

    //Bot tick function, it should always call parents tick function first
    int tick();

    //Bot main draw function
    void draw(frame_type &image, int _xy);

    //Bot draw function is energy mode
    void drawEnergy(frame_type &image, int _xy);

    //Bot draw function is predators mode
    void drawPredators(frame_type &image, int _xy);

    //Change rotation function
    void Rotate(int dir = 1);

    //Give a bot some energy
    void GiveEnergy(int num, EnergySource src);

    //Get rotation
    [[nodiscard]] oPoint GetDirection() const;

    //Take away bot energy, return true if 0 or below (bot dies)
    bool TakeEnergy(int val);
    float FindKinship(t_object &stranger) const;

    Bot(int Energy);
    //Inherit from a parent
    Bot(int Energy, t_object &prototype);



};


