#pragma once

#include <opencv2/opencv.hpp>

#define InterfaceBorder 0

//Field
const cv::Scalar FieldBackgroundColor{255, 255, 255};

#define FieldX InterfaceBorder
#define FieldY InterfaceBorder

const int FieldCellsWidth{35 * 9}; //(192 + 192 / 2)        //?????? ???????? ?? 8 ??? ??????? ???? ????? 4 ??????! ? ?? 16 ??? ??????? ???? 8 ???????!
const int FieldCellsHeight{FieldCellsWidth}; //(192 + 192 / 2)
const int TotalCells = FieldCellsWidth * FieldCellsHeight;
auto XY = [](int x, int y) {return x * FieldCellsWidth + y;};
auto XYr = [](int xy) {return std::tuple(xy / FieldCellsWidth, xy % FieldCellsWidth);};
#define FieldCellSize 3
#define FieldWidth FieldCellSize*FieldCellsWidth
#define FieldHeight FieldCellSize*FieldCellsHeight

//-----------------------------------------------------------------


#define RenderTypeAtStart predators

#define ControlGroupSize 5000
//-----------------------------------------------------------------
//World rules
const int p_24{4};

const int p_year{356};
const int p_half_year{p_year / 2};
const int MaxBotLifetime{5 * p_year};

const int UE{10}; // Energy Unit
const int FoodbaseMineralsTerrain{1 * UE};
const int FoodbaseMineralsSea{4 * UE};
const int PhotosynthesisReward_Summer{8 * UE};
const int MaxPossibleEnergyForABot{50 * UE};
const int EnergyPassedToAChild{50 * UE};
const int EveryTickEnergyPenalty{0 *UE};
const int AttackCost{4 * UE};
const int MoveCost{1 * UE};
const int RotateCost{1 * UE / 5};
const int GiveBirthCost{10 * UE};

#define StunAfterBirth 1    //How many turns creature cannot act after his birth
#define FertilityDelay 0    //Delay before next birth

#define ChangeColorSlightly

#define MutateNeuronsMaximum 5
#define MutateNeuronsSlightlyMaximum 0


//-----------------------------------------------------------------
//Save and load
#define DirectoryName "SavedObjects/"

#define FilenameMaxLen 50
//-----------------------------------------------------------------
//Neural nets
#define NeuronsInLayer 7
#define NumNeuronLayers 6
#define NeuronOutputLayerIndex (NumNeuronLayers-1)
#define NeuronInputLayerIndex 0
#define MaxConnectionsPerNeuron 3
//-----------------------------------------------------------------

//Neural net renderer (bot brain window)
#define Render_PositiveWeightColor cv::Scalar{185, 0, 0}
#define Render_NegativeWeightColor cv::Scalar{0, 0, 185}
#define Render_NeutralWeightColor cv::Scalar{100, 100, 100}

#define Render_GreyThreshold 0.15f

#define Render_NeuronSize 30
#define Render_LayerDistance 100
#define Render_VerticalDistance 60

//-----------------------------------------------------------------

// Population chart window
#define ChartNumValues 1000
//-----------------------------------------------------------------

enum RenderTypes
{
    natural,
    predators,
    abilities,
    energy,
    sun_energy,
    // DNK data
    max_energy,
    defence_attack,
    def_all,
    kill_ability,
    minerals_ability,
    ps_ability,
    mutability_body,
    mutability_brain,
    max_life_time,
    garb,
    lifetime,
    fertility,
    dnk_energy_given_on_birth,
    dnk_move_ability_sea,
    dnk_move_ability_earth

};

enum Terrain {
    earth, sea
};

