#ifndef _h_Field
#define _h_Field



#include "Bot.h"
#include "tbb/concurrent_vector.h"


//Don't touch
#define NumThreads 1
#ifdef UseFourThreads
#undef NumThreads
#define NumThreads 4
#endif
#ifdef UseEightThreads
#undef NumThreads
#define NumThreads 8
#endif



enum RenderTypes
{
    natural,
    predators,
    energy,
//    noRender
};

enum Season
{
    summer,
    autumn,
    winter,
    spring
};

//extern

using tbb_vec_obj = tbb::concurrent_vector<t_object>;

//Simulation field class
class Field
{
    //All cells as 2d array
    t_object animals[FieldCellsWidth][FieldCellsHeight];
    tbb_vec_obj boots;
    int sequence[FieldCellsWidth * FieldCellsHeight];

//    .resize(23);


    Season season = Season::summer;
    //Rectangles
    cv::Rect mainRect = { FieldX , FieldY, FieldWidth, FieldHeight };
    cv::Rect oceanRect = { FieldX , FieldY + FieldHeight - OceanHeight * FieldCellSize, FieldWidth, OceanHeight * FieldCellSize };

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
    //Number of food for photosynthesis and other means
    int foodBase = FoodbaseInitial;

    t_object & get(int x, int y);
    void set(int x, int y, t_object &obj);
    void set_to_null(int x, int y);

    //Move objects from one cell to another
    int MoveObject(int fromX, int fromY, int toX, int toY);
    int MoveObject(const t_object& obj, int toX, int toY);

    //Add new object
    bool AddObject(t_object &obj);

    //Remove object and delete object class
    void RemoveObject(int X, int Y);

    //Repaint bot
    void RepaintBot(t_object &b, cv::Scalar &newColor, int differs = 1);

    //Tick function for every object,
    //Returns true if object was destroyed
    void ObjectTick(t_object &bbot);

    //Tick function
    void tick(int thisFrame);


    //Draw simulation field with all its objects
    void draw(frame_type &image);

    //Is cell out if bounds?
    bool IsInBounds(int X, int Y);
    bool IsInBounds(oPoint &p);


    //This function is needed to tile world horizontally (change X = -1 to X = FieldCellsWidth etc.)
    int ValidateX(int X);

    //Is cell out of bounds, given absolute screen space coordinates
    bool IsInBoundsScreenCoords(int X, int Y);

    //Transform absolute screen coords to cell position on field
    oPoint ScreenCoordsToLocal(int X, int Y);

    //Get object at certain point on field
    t_object GetObjectLocalCoords(int X, int Y);

    //Validates if object exists
    bool ValidateObjectExistance(t_object obj);

    //How many objects on field, last frame
    int GetNumObjects();


    /*Save / load
    TODO!!!
    File format:
    4b - 0xfafa458e (no meaning)
    4b - creature type 
    4b - unt num layers
    4b - unt neurons in layer
    4b - sizeof (Neuron)
    following all neurons from first to last layer by layer
    */
    bool SaveObject(t_object obj, char* filename);
    bool LoadObject(t_object obj, char* filename);


    //Spawn group of random bots
    void SpawnControlGroup();


    //Create field
    Field();

    void NextView();

    void Annotate(frame_type& image) const;

};


#endif