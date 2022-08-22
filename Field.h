#ifndef b_Field
#define b_Field

#include "Bot.h"
#include "tbb/concurrent_vector.h"



const cv::Scalar color_earth{128, 194, 178};
const cv::Scalar color_sea{148, 105, 0};
const int tbb_step{3};

enum RenderTypes
{
    natural, predators, energy,
};

enum Terrain {
    eart, sea
};

//using tbb_vec_obj = tbb::concurrent_vector<t_object>;
using tbb_vec_obj = std::vector<t_object>;

//Simulation field class
class Field
{

    tbb_vec_obj boots;
    Terrain terrain[FieldCellsWidth][FieldCellsHeight];
    std::vector<int> sequence;
    frame_type BGround{FieldX * 2 + FieldCellsWidth * FieldCellSize, FieldY * 2 + FieldHeight, CV_8UC3};
    //Needed to calculate number of active objects (calculated on every frame)
    int objectsTotal = 0;

    //Find empty cell nearby, otherwise return {-1, -1}
    oPoint FindFreeNeighbourCell(int X, int Y);


    //tick function for single threaded build
    inline void tick_single_thread();

public:
    RenderTypes render = RenderTypeAtStart;
    bool run{true};
    int show_frame{1};
    unsigned long frame_number{0};

    //Add new object
    bool AddObject(t_object &obj);

    //Tick function for every object,
    //Returns true if object was destroyed
//    [[maybe_unused]] void ObjectTick(int &i_xy);
    void ObjectTick1(int &i_xy);
    void ObjectTick2(int &i_xy);

    //Tick function
    void tick(int thisFrame);


    //Draw simulation field with all its objects
    void draw(frame_type &image);

    //Is cell out if bounds?
    static bool IsInBounds(int X, int Y);
    static bool IsInBounds(oPoint &p);


    //This function is needed to tile world horizontally (change X = -1 to X = FieldCellsWidth etc.)
    static int ValidateX(int X);

    //Spawn group of random bots
    void SpawnControlGroup();


    //Create field
    Field();

    void NextView();

    void Annotate(frame_type& image) const;

    int GetSunEnergy(int x, int y) const;

    void ShowMutations();

    bool AddObject(t_object &obj, int coord);
};


#endif