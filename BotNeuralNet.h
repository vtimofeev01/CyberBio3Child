#pragma once
#include "Neuron.h"



struct BrainInput
{
	float
	energy = 0.0f,
	vision = 0.0f,
	isRelative = 0.0f,
	rotation = 0.0f,
	goal_energy = 0.0f;
    Terrain local_terrain = Terrain::earth;
    Terrain direct_terrain = Terrain::earth;

    // TODO add what he sees
    // TODO what the terrain hi is
    // TODO what the terrain hi wants to be
    // TODO Energy levels
};



union BrainOutput
{
	static const int numFields = 5;

	struct
	{
		int rotate;
		int move;
		int photosynthesis;
		int divide;
		int attack;
	};

	int fields[numFields];
};


class BotNeuralNet
{

private:

	//Activation functions
	static float Activation(float value);
	static float PlusMinusActivation(float value);
	static float RadialBasisActivation(float value);

	//Clear memory
	void ClearMemory();


public:

	//All neurons
	Neuron allNeurons[NumNeuronLayers][NeuronsInLayer];

	//Neuron values and memory
	float allValues[NumNeuronLayers][NeuronsInLayer];
	float allMemory[NumNeuronLayers][NeuronsInLayer];


	//Constructors
	BotNeuralNet();
	explicit BotNeuralNet(BotNeuralNet* prototype);

	//Clear all values
	void Clear();

	//Process data
	void Process();


	/*Very little change, no new connections,
	  experimental*/
	void MutateSlightly();


	//Set one neuron to random
	void Mutate();

	//Change half neurons a lot
	void MutateHarsh();

	//Make completely random
	void Randomize();

	
	BrainOutput GetOutput();


	void SetDummy();

};
