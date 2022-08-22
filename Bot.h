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
    mineral
};

struct summary_return {
    int simple, radialBasis, random, memory, connections, deadend, neurons;
};

//Preselected colors for bots
const uint8_t presetColors[][4] =
        {
                {255, 0,   0},
                {0,   255, 0},
                {0,   0,   255},
                {223, 52,  210},
                {200, 14,  84},
                {60,  60,  163},
                {160, 160, 200},
                {0,   255, 255},
                {100, 170, 170},
                {80,  90,  90},
                {70,  200, 160},
                {0,   130, 86},
                {0,   133, 0},
                {140, 255, 0},
                {136, 160, 103},
                {200, 255, 0},
                {160, 180, 100},
                {255, 255, 0},
                {255, 190, 0},
                {170, 150, 85},
                {255, 120, 0},
                {240, 200, 200}
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
    int protetction_front;
    int protetction_others;
    int atack_ability;
    int minerals_ability;
    int ps_ability;
    int mutability_body;
    int mutability_brain;
    int max_life_time;

    DNK() : max_energy(MaxPossibleEnergyForABot), protetction_front(0), protetction_others(0), minerals_ability(0),
            atack_ability(0), ps_ability(0), mutability_body(3), mutability_brain(5), max_life_time(MaxBotLifetime) {}

    DNK &operator=(const DNK &dnk2);

    float distance(DNK &d2) const {
        int res{0};
        res += std::abs(1000 * (max_energy - d2.max_energy)) / (max_energy + d2.max_energy + 1);
        res += std::abs(1000 * (protetction_front - d2.protetction_front)) /
               (protetction_front + d2.protetction_front + 1);
        res += std::abs(1000 * (protetction_others - d2.protetction_others)) /
               (protetction_others + d2.protetction_others + 1);
        res += std::abs(1000 * (atack_ability - d2.atack_ability)) / (atack_ability + d2.atack_ability + 1);
        res += std::abs(1000 * (minerals_ability - d2.minerals_ability)) / (minerals_ability + d2.minerals_ability + 1);
        res += std::abs(1000 * (ps_ability - d2.ps_ability)) / (ps_ability + d2.ps_ability + 1);
        res += std::abs(1000 * (mutability_body - d2.mutability_body)) / (mutability_body + d2.mutability_body + 1);
        res += std::abs(1000 * (mutability_brain - d2.mutability_brain)) / (mutability_brain + d2.mutability_brain + 1);
        res += std::abs(1000 * (max_life_time - d2.max_life_time)) / (max_life_time + d2.max_life_time + 1);
        return static_cast<float>(res) / 1000 / 9;
    }

    void mutate(int d);

    std::string descript() const;


};

//Bot class
class Bot {
    int lifetime{0};
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

    //Experimental
    //Total mutation function - rarely used
//	void TotalMutation();

    //Shift color a little (-10 to +10)
    void ChangeColor(int str = 10);

    //Experimental
    void SlightlyMutate();

    //Main mutate function
    void Mutate();

    //Draw bot outline and his head
    void drawOutlineAndHead(frame_type &image, cv::Point &p1, cv::Point &p2) const;

public:
    static int selectionStep;
    //----------------------------------------------------------------------------------------------


    int x;
    int y;
    //Energy acquired from different sources
    unsigned long energyFromPS = 0;
    unsigned long energyFromKills = 0;
    unsigned long energyFromMinerals = 0;
    //Bot energy, if this is 0 bot dies

    int direction{0};
public:

    DNK dnk;
    int energy;
    //Experimental
    //This function is used to simulate mutagen

    //Use neural network
    BrainOutput bots_ideas{};

    BrainOutput think(BrainInput input);

    //Bot tick function, it should always call parents tick function first
    int tick();

    //Bot main draw function
    void draw(frame_type &image);

    //Bot draw function is energy mode
    void drawEnergy(frame_type &image);

    //Bot draw function is predators mode
    void drawPredators(frame_type &image);

    //Change rotation function
    void Rotate(int dir = 1);

    //Give a bot some energy
    void GiveEnergy(int num, EnergySource src);

    //Get neural net pointer
    Neuron *GetNeuralNet();

    //Get rotation
    oPoint GetDirection() const;

    //Take away bot energy, return true if 0 or below (bot dies)
    bool TakeEnergy(int val);


    /*Get neuron summary(info)
    Format: (all integers)
    -simple neurons
    -radial basis neurons
    -random neurons
    -memory neurons (if any)
    -total connections
    -dead end neurons
    -total neurons
    */
    summary_return GetNeuronSummary();

//
//	/*Find out how close these two are as relatives,
//	returns number of matching mutation markers*/
    float FindKinship(t_object &stranger) const;


    //Inherit from a parent
    Bot(int X, int Y, int Energy, t_object &prototype);

    //New bot
    Bot(int X, int Y, int Energy = 100);

    int coord() const;

    //This function is used only after a bot was loaded from file!!
    void GiveInitialEnergyAndMarkers();


};


