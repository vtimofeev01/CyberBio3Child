#pragma once

#include <opencv2/opencv.hpp>
#include "BotNeuralNet.h"
#include "Systems.h"


//Rotations array, contains where a bot would look with every
//position of its head
const oPoint Rotations[] =
{
	{0,-1},
	{1,-1},
	{1,0},
	{1,1},
	{0,1},
	{-1,1},
	{-1,0},
	{-1,-1}
};

enum EnergySource
{
	PS,
	kills,
	mineral,
	unknown
};

struct summary_return{ int simple, radialBasis, random, memory, connections, deadend, neurons; };

struct s_Color{ uint8_t rgb[3]; };


//Preselected colors for bots
const uint8_t presetColors[][4] =
{
	{255, 0, 0},
	{0, 255, 0},
	{0, 0, 255},
	{223, 52, 210},
	{200, 14, 84},
	{60, 60, 163},
	{160, 160, 200},
	{0, 255, 255},
	{100, 170, 170},
	{80, 90, 90},
	{70, 200, 160},
	{0, 130, 86},
	{0, 133, 0},
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



//Bot class
class Bot
{
	//Rotation, see Rotations[]
	int direction{0};
    int lastTickFrame{0};
    int lifetime{0};
    ObjectTypes type;
	BotNeuralNet brain;
    int currentFrame;
    //Bot energy, if this is 0 bot dies
	int energy;

	//if this is not 0, bot does nothing at his turn
	int stunned;

	//How long a bot should wait to give birth after his own birth 
	int fertilityDelay;

	cv::Scalar color {static_cast<double>(rand()%250), static_cast<double>(rand()%250), static_cast<double>(rand()%250)};

	//Energy acquired from different sources
	int energyFromPS = 0;
	int energyFromKills = 0;
	int energyFromMinerals = 0;


	//Mutation markers used to decide how close two bots are to each other as relatives
	int mutationMarkers[NumberOfMutationMarkers];
    int nextMarker;



	void ChangeMutationMarker();

	//Set random mutation markers
	void RandomizeMarkers();

	//Set random color
	void RandomizeColor();

	//Experimental
	//Total mutation function - rarely used
	void TotalMutation();

	//Shift color a little (-10 to +10)
	void ChangeColor(int str = 10);

	//Experimental
	void SlightlyMutate();

	//Main mutate function
	void Mutate();

	//Draw bot outline and his head
    void drawOutlineAndHead(frame_type &image, cv::Point &p1, cv::Point &p2);


	//----------------------------------------------------------------------------------------------
	//Функции для осуществления селекции, вызывается в конце функции tick(), либо не использовать

	int selection_numTicks = 0;
	int selection_numRightSteps = 0;
	int selection_lastX;

	bool SelectionWatcher();

	public: static int selectionStep;
	//----------------------------------------------------------------------------------------------


    int x;
    int y;
public:

	//Experimental
	//This function is used to simulate mutagen
	void Radiation();

	//Use neural network
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
	void GiveEnergy(int num, EnergySource src = unknown);

	//Return current energy amount from different sources
	int GetEnergy() const;
	int GetEnergyFromPS() const;
	int GetEnergyFromKills() const;

	//Returns a pointer to mutation markers array
	int* GetMarkers();

	//Get bot color
    cv::Scalar GetColor();

	//Get neural net pointer
	Neuron* GetNeuralNet();

	//Get brain
	BotNeuralNet* GetBrain();

	//Get rotation
    oPoint GetDirection();

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

	/*Find out how close these two are as relatives,
	returns number of matching mutation markers*/
	int FindKinship(std::shared_ptr<Bot> &stranger);

	//Change bot color
	void Repaint(cv::Scalar &newColor);

	//Inherit from a parent
	Bot(int X, int Y, int Energy, std::shared_ptr<Bot> prototype, bool mutate = false);

	//New bot
	Bot(int X, int Y, int Energy = 100);



	//This function is used only after a bot was loaded from file!!
	void GiveInitialEnergyAndMarkers();



};

//using t_object = Bot;
using TObject = Bot;
//using t_object = std::shared_ptr<TObject>;
using t_object = std::unique_ptr<TObject>;
using make_t_object = std::;make_unique<TObject>;