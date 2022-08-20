#include <tbb/parallel_for.h>
#include "Field.h"
#include "MyTypes.h"


oPoint Field::FindFreeNeighbourCell(int X, int Y) {
    //If cell itself is empty
    if (get(X, Y) == nullptr) {
        return {X, Y};
    }

    int tx;
    oPoint tmpArray[9];
    int i = 0;

    for (int cx = -1; cx < 2; ++cx) {
        for (int cy = -1; cy < 2; ++cy) {

            tx = ValidateX(X + cx);

            if (IsInBounds(tx, Y + cy)) {
                if (get(tx, Y + cy) == nullptr) {
                    tmpArray[i++].x = tx;   //Set(tx, Y + cy);
                    tmpArray[i].y = Y + cy;   //Set(tx, Y + cy);
                }
            }
        }
    }

    //Get random free cell from array
    if (i > 0) {
        return tmpArray[RandomVal(i)];
    }

    //No free cells nearby
    return {-1, -1};
}


int Field::MoveObject(int fromX, int fromY, int toX, int toY) {

    if (!IsInBounds(toX, toY))
        return -2;

    if (get(toX, toY)) return -1;

//    auto* tmpObj = animals[fromX][fromY];

    if (get(fromX, fromY)) {
        auto val = get(fromX, fromY);
        set(toX, toY, val);
        set_to_null(fromX, fromY);
        return 0;
    }
    return -3;
}

int Field::MoveObject(const t_object &obj, int toX, int toY) {
    auto x = obj->x;
    auto y = obj->y;
    auto val = get(x, y);
    set(toX, toY, val);
    set_to_null(x, y);
}

bool Field::AddObject(t_object &obj) {
    if (get(obj->x, obj->y)) return false;
    set(obj->x, obj->y, obj);
    return true;
}

void Field::RemoveObject(int X, int Y) {
    set_to_null(X, Y);
}

void Field::RepaintBot(t_object &b, cv::Scalar &newColor, int differs) {
    t_object tmpObj;

    for (unt ix = 0; ix < FieldCellsWidth; ++ix)
        for (unt iy = 0; iy < FieldCellsHeight; ++iy) {
            auto val = get(ix, iy);
            if (val)
                if (val->FindKinship(b) >= (NumberOfMutationMarkers - differs)) {
                    val->Repaint(newColor);
                }
        }

}

//Tick function for every object,
//Returns true if object was destroyed
void Field::ObjectTick(t_object &bbot) {
    auto bot = bbot;
    int t = bot->tick();

    if (t == 2) { return; }

    int TEMP_x1 = bot->x;
    BrainOutput bots_ideas{};
    auto lookAt = bot->GetDirection();

    //Fill brain input structure
    BrainInput b_input;

    //Desired destination,
    //that is what bot is lookinig at
    int cx = bot->x + lookAt.x;
    int cy = bot->y + lookAt.y;

    cx = ValidateX(cx);

    if ((abs(cx - TEMP_x1) > 1) && (abs(cx - TEMP_x1) < 190)) {
        TEMP_x1 = bot->x;
    }

    //If destination is out of bounds
    if (!IsInBounds(cx, cy)) {
        b_input.vision = 1.0f; //1 if unpassable
    } else {
        //Destination cell is empty
        auto obj_cxcy = get(cx, cy);
        if (!obj_cxcy) {
            //0 if empty
            b_input.vision = 0.0f;
        } else {
            //0.5 if someone in that cell
            b_input.vision = 0.5f;

            //Calculate how close they are as relatives, based on mutation markers
            b_input.isRelative = 1.0f - (obj_cxcy->FindKinship(bot) * 1.0f) /
                                        (NumberOfMutationMarkers * 1.0f);
        }
    }

    //Bot brain does its stuff
    bots_ideas = bot->think(b_input);

    //IGNORE NEXT LINES
    //Temporary!!! TODO
    //bool minerals = ((tmpOut.fields[0] == 1) && (tmpOut.fields[1] == 1) && (tmpOut.fields[2] == 1) && (tmpOut.fields[3] == 0) && (tmpOut.fields[4] == 0));
    bool minerals = false;
    //if (minerals == true)
    //goto InsaneJump;
    //bool minerals = ((tmpOut.fields[0] == 0) && (tmpOut.fields[1] == 0) && (tmpOut.fields[2] == 1) && (tmpOut.fields[3] == 0));

    //Multiply first
    for (int b = 0; b < bots_ideas.divide; ++b) {
        //Dies if energy is too low
        if (bot->GetEnergy() <= EnergyPassedToAChild + GiveBirthCost) {
            RemoveObject(bot->x, bot->y);
            return;
        } else {
            //Gives birth otherwise
            auto freeSpace = FindFreeNeighbourCell(bot->x, bot->y);
            if (freeSpace.x != -1) {
                bot->TakeEnergy(EnergyPassedToAChild + GiveBirthCost);
                auto mutation = RandomPercent(MutationChancePercent);
                auto val = std::make_shared<Bot>(freeSpace.x, freeSpace.y,
                                                 EnergyPassedToAChild,
                                                 bot,
                                                 mutation);
                AddObject(val);
                return;
            }
        }
    }

    //Then attack
    if (bots_ideas.attack) {
        //If dies of low energy
        if (bot->TakeEnergy(AttackCost)) {
            RemoveObject(bot->x, bot->y);
            return;
        } else {
            //Get direction of attack
            auto dir = bot->GetDirection();

            cx = ValidateX(bot->x + dir.x);
            cy = bot->y + dir.y;

            if (IsInBounds(cx, cy)) {
                //If there is an object
                auto val = get(cx, cy);
                if (val) {
                    //Kill an object
                    bot->GiveEnergy(val->GetEnergy(), kills);
                    RemoveObject(cx, cy);
                }
                else { // when the attacked escapes

                }
            }
        }
    } else {
        //Rotate after
        if (bots_ideas.rotate != 0.0f) {
            //If dies of low energy
            if (bot->TakeEnergy(RotateCost)) {
                RemoveObject(bot->x, bot->y);
                return;
            }

            bot->Rotate(bots_ideas.rotate);
        }

        //Move
        if (bots_ideas.move) {
            if (bot->TakeEnergy(MoveCost)) {
                RemoveObject(bot->x, bot->y);
                return;
            }

            auto dir = bot->GetDirection();

            cx = bot->x + dir.x;
            cy = bot->y + dir.y;
            cy = std::max(cy, 0);
            cy = std::min(cy, FieldCellsHeight - 1);

            cx = ValidateX(cx);

            if ((abs(cx - TEMP_x1) > 1) && (abs(cx - TEMP_x1) < 190)) {
                TEMP_x1 = bot->x;
            }

            //Place object in a new place
            MoveObject(bot, cx, cy);
        }
            //Photosynthesis
        else if (bots_ideas.photosynthesis) {
            //InsaneJump: <- ignore this
            //
            //If not in ocean
            //if (tmpBot->y < FieldCellsHeight - OceanHeight)
            if (true) {
                if (minerals)
                    return;

                int toGive;

                //Give energy depending on a season
                switch (season) {
                    case summer:
#ifdef ChangeSeasons
                        toGive = PhotosynthesisReward_Summer;
#else
                        toGive = foodBase;
#endif
                        break;
                    case autumn:
                    case spring:
                        toGive = PhotosynthesisReward_Autumn;
                        break;
                    case winter:
                        //toGive = (ticknum%4 == 0)?PhotosynthesisReward/8:0;
                        toGive = PhotosynthesisReward_Winter;
                        //toGive = (ticknum % 5 == 0) ? 2 : 1;
                        break;
                }

                bot->GiveEnergy(toGive, PS);
            } else  //not used atm
            {
                if (minerals) {
                    bot->GiveEnergy(foodBase, mineral);
                }
            }
        }
    }

    if ((abs(bot->x - TEMP_x1) > 1) && (abs(bot->x - TEMP_x1) < 190)) {
        TEMP_x1 = bot->x;
    }

}

//tick function for single threaded build
inline void Field::tick_single_thread() {
    objectsTotal = 0;
    tbb::parallel_for(0, FieldCellsWidth * FieldCellsHeight, [&](int i_xy) {
        auto ix = i_xy / FieldCellsWidth;
        auto iy = i_xy % FieldCellsHeight;
        auto v = get(ix, iy);
        if (v && (v->tick() == 1)) set_to_null(ix, iy);

    });
    const auto total = FieldCellsWidth * FieldCellsHeight;
    auto f = [&](int i_xy) {
//    for (auto i_xy = 0; i_xy < FieldCellsWidth * FieldCellsHeight; i_xy++) {
        auto m_ix = sequence[i_xy];
        auto ix = m_ix / FieldCellsWidth;
        auto iy = m_ix % FieldCellsHeight;
        auto tmpObj = get(ix, iy);
        if (tmpObj) {
            ++objectsTotal;
            ObjectTick(tmpObj);
        }
    };
    tbb::parallel_for(0, total / 4, f);
    tbb::parallel_for(total/4, total / 2, f);
    tbb::parallel_for(total / 2, total * 3 / 4, f);
    tbb::parallel_for(total * 4 / 4, total , f);
}

////Wait for a signal
//inline void Field::ThreadWait(const unt index)
//{
//    for (;;)
//    {
//        if (threadGoMarker[index])
//            return;
//
//        if (terminateThreads)
//            TerminateThread();
//
//        std::this_thread::yield();
//    }
//}
//
////Process function for 4 threaded simulation
//void Field::ProcessPart_4Threads(const unt X1, const unt Y1, const unt X2, const unt Y2, const unt index)
//{
//
//    Object* tmpObj;
//
//Again:
//
//    ThreadWait(index);
//
//    for (int X = X1; X < X1 + ((X2 - X1) / 2); ++X)
//    {
//
//        for (int Y = Y1; Y < Y2; ++Y)
//        {
//
//            tmpObj = allCells[X][Y];
//
//            if (tmpObj == nullptr)
//                continue;
//
//            ++objectCounters[index];
//            ObjectTick(tmpObj);
//
//        }
//
//    }
//
//    threadGoMarker[index] = false;
//
//    ThreadWait(index);
//
//    for (int X = X1 + ((X2 - X1) / 2); X < X2; ++X)
//    {
//
//        for (int Y = Y1; Y < Y2; ++Y)
//        {
//
//            tmpObj = allCells[X][Y];
//
//            if (tmpObj == nullptr)
//                continue;
//
//            ++objectCounters[index];
//            ObjectTick(tmpObj);
//
//        }
//
//    }
//
//    threadGoMarker[index] = false;
//
//    goto Again;
//
//}
//
////Start all threads
//void Field::StartThreads()
//{
//    repeat(NumThreads)
//    {
//        threadGoMarker[i] = true;
//    }
//}
//
////Wait for all threads to finish their calculations
//void Field::WaitForThreads()
//{
//
//    unt threadsReady;
//
//    for (;;)
//    {
//
//        threadsReady = 0;
//
//        repeat(NumThreads)
//        {
//            if (threadGoMarker[i] == false)
//                threadsReady++;
//        }
//
//        if (threadsReady == NumThreads)
//            break;
//
//        std::this_thread::yield();
//
//    }
//
//}
//
////Multithreaded tick function
//inline void Field::tick_multiple_threads()
//{
//    auto clear_counters = [&]()
//    {
//        repeat(NumThreads)
//            objectCounters[i] = 0;
//    };
//
//    objectsTotal = 0;
//
//    //Clear object counters
//    clear_counters();
//
//    //Starting signal for all 4 threads
//    StartThreads();
//
//    //Wait for threads to synchronize first time
//    WaitForThreads();
//
//    //Add object counters
//    repeat(NumThreads)
//        objectsTotal += objectCounters[i];
//
//    //Clear object counters
//    clear_counters();
//
//    //Starting signal for all 4 threads
//    StartThreads();
//
//    //Wait for threads to synchronize second time
//    WaitForThreads();
//
//    //Add object counters
//    repeat(NumThreads)
//        objectsTotal += objectCounters[i];
//
//}

//Tick function
void Field::tick(int thisFrame) {
//    Object::currentFrame = thisFrame;
    if (!run) return;
    frame_number++;
    tick_single_thread();
}


//Draw simulation field with all its objects
void Field::draw(frame_type &image) {

    //Background
    auto bgColor = cv::Scalar(255, 255, 255);
    if (render == energy) bgColor = cv::Scalar(255, 200, 200);
    cv::rectangle(image, mainRect, bgColor,
                  -1, cv::LINE_8, 0);

    //Ocean
#ifdef DrawOcean
    cv::rectangle(image, oceanRect, OceanColor,
                  -1, cv::LINE_8, 0);
#endif

    //Bots
//    Object* tmpObj;

    auto f = [&](int m_ix) {
        auto ix = m_ix / FieldCellsWidth;
        auto iy = m_ix % FieldCellsHeight;
        auto tmpObj = get(ix, iy);
        if (tmpObj) {

            //Draw function switch, based on selected render type
            switch (render) {
                case natural:
                    tmpObj->draw(image);
                    break;
                case predators:
                    tmpObj->drawPredators(image);
                    break;
                case energy:
                    tmpObj->drawEnergy(image);
                    break;
            }

        }
    };
    tbb::parallel_for(0, FieldCellsWidth * FieldCellsHeight, f);
//    for (auto ix = 0; ix < FieldCellsWidth; ++ix)
//        for (auto iy = 0; iy < FieldCellsHeight; ++iy)
    Annotate(image);
}

//Is cell out if bounds?
bool Field::IsInBounds(int X, int Y) {
    return ((X >= 0) && (Y >= 0) && (X < FieldCellsWidth) && (Y < FieldCellsHeight));
}

bool Field::IsInBounds(oPoint &p) {
    return IsInBounds(p.x, p.y);
}


//This function is needed to tile world horizontally (change X = -1 to X = FieldCellsWidth etc.)
int Field::ValidateX(int X) {
#ifdef TileWorldHorizontally
    if (X < 0) {
        return (X % FieldCellsWidth) + FieldCellsWidth;
    } else if (X >= FieldCellsWidth) {
        return (X % FieldCellsWidth);
    }
#endif

    return X;
}

//Is cell out of bounds, given absolute screen space coordinates
bool Field::IsInBoundsScreenCoords(int X, int Y) {
    return ((X >= mainRect.x) && (X <= mainRect.x + mainRect.width) &&
            (Y >= mainRect.y) && (Y <= mainRect.y + mainRect.height));
}

//Transform absolute screen coords to cell position on field
oPoint Field::ScreenCoordsToLocal(int X, int Y) {
    X -= FieldX;
    Y -= FieldY;

    X /= FieldCellSize;
    Y /= FieldCellSize;

    return {X, Y};
}

//Get object at certain point on field
t_object Field::GetObjectLocalCoords(int X, int Y) {
    return get(X, Y);
}

//Validates if object exists
bool Field::ValidateObjectExistance(t_object obj) {
    for (int ix = 0; ix < FieldCellsWidth; ++ix) {
        for (int iy = 0; iy < FieldCellsHeight; ++iy) {
            if (get(ix, iy) == obj)
                return true;
        }
    }

    return false;
}

//How many objects on field, last frame
int Field::GetNumObjects() {
    return objectsTotal;
}


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
bool Field::SaveObject(t_object obj, char *filename) {

    //Open file for writing, binary type, all contents to be deleted
    std::ofstream file(filename, std::ios::in | std::ios::binary | std::ios::trunc);

    if (file.is_open()) {
        int i = 0xfafa458e;

        file.write((char *) &i, sizeof(int));
        i = 1;
        file.write((char *) &i, sizeof(int));
        i = NumNeuronLayers;
        file.write((char *) &i, sizeof(int));
        i = NeuronsInLayer;
        file.write((char *) &i, sizeof(int));
        i = sizeof(Neuron);
        file.write((char *) &i, sizeof(int));

        file.write((char *) obj->GetNeuralNet(), NumNeuronLayers * NeuronsInLayer * sizeof(Neuron));

        file.close();

        return true;
    }

    return false;

}

bool Field::LoadObject(t_object obj, char *filename) {
    //Open file for reading, binary type
//    std::ifstream file(filename, std::ios::in | std::ios::binary | std::ios::beg);
    std::ifstream file(filename);//, std::ios::in | std::ios::binary | std::ios::beg);

    if (file.is_open()) {
        int i = 0;

        file.read((char *) &i, sizeof(int));

        if (i != 0xfafa458e)
            return false;

        file.read((char *) &i, sizeof(int));

        if (i != 1)
            return false;

        file.read((char *) &i, sizeof(int));
        if (i != NumNeuronLayers)
            return false;
        file.read((char *) &i, sizeof(int));
        if (i != NeuronsInLayer)
            return false;
        file.read((char *) &i, sizeof(int));
        if (i != sizeof(Neuron))
            return false;
        file.read((char *) obj->GetNeuralNet(), NumNeuronLayers * NeuronsInLayer * sizeof(Neuron));
        file.close();
        obj->GiveInitialEnergyAndMarkers();
        return true;
    }
    return false;
}


//Spawn group of random bots
void Field::SpawnControlGroup() {
    for (int i = 0; i < ControlGroupSize; ++i) {
        auto tmpBot = std::make_shared<Bot>(RandomVal(FieldCellsWidth), RandomVal(FieldCellsHeight), 100);
        AddObject(tmpBot);
    }
}


//Create field
Field::Field() {
    boots.resize(FieldCellsHeight * FieldCellsWidth, nullptr);
    //Clear array
    int main_ix = 0;
    for (auto x0 = 0; x0 < 2; x0++)
        for (auto y0 = 0; y0 < 2; y0++)
            for (auto x = 0; x < FieldCellsWidth; x += 2)
                for (auto y = 0; y < FieldCellsHeight; y += 2) {
                    sequence[main_ix++] = (x + x0) * FieldCellsWidth + y + y0;
                }

    //Spawn objects
#ifdef SpawnControlGroupAtStart
    SpawnControlGroup();
#endif

}

t_object & Field::get(int x, int y) {
    return boots[x * FieldCellsWidth + y];
}

void Field::set(int x, int y, t_object &obj) {
    boots[x * FieldCellsWidth + y] = obj;
    obj->x = x;
    obj->y = y;
}

void Field::set_to_null(int x, int y) {
    boots[x * FieldCellsWidth + y] = nullptr;
}

void Field::NextView() {
    if (render == RenderTypes::natural) render = RenderTypes::energy;
    else if (render == RenderTypes::energy) render = RenderTypes::predators;
    else if (render == RenderTypes::predators) render = RenderTypes::natural;
}

void Field::Annotate(frame_type &image) const {
    std::vector<std::string> lines;
    if (render == RenderTypes::natural) lines.emplace_back("view: natural");
    else if (render == RenderTypes::energy) lines.emplace_back("view: energy");
    else if (render == RenderTypes::predators) lines.emplace_back("view: predators");
    std::ostringstream stringStream;
    stringStream << "gen:" << frame_number;
    lines.emplace_back(stringStream.str());

    int x{FieldX}, y{FieldY}, baseline{0};;
    auto font = cv::FONT_HERSHEY_SIMPLEX;
    auto font_size = 0.85;
    auto font_thickness = 2;
    for (auto l: lines) {
        auto textsize = cv::getTextSize(l, font, font_size, font_thickness, &baseline);
        y += textsize.height + font_thickness;
        auto text_org = cv::Point(x, y);
//        rectangle(image, text_org,
//                  text_org + cv::Point(textsize.width, textsize.height),
//                  cv::Scalar(255,255,255));

        cv::putText(image, l, text_org, font, font_size,
                    cv::Scalar(255, 100, 200), font_thickness, 8);
    }


}




