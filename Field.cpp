#include <tbb/parallel_for.h>
#include "Field.h"
#include "MyTypes.h"


oPoint Field::FindFreeNeighbourCell(int X, int Y) {
    //If cell itself is empty
    if (boots[XY(X, Y)] == nullptr) {
        return {X, Y};
    }

    int tx;
    oPoint tmpArray[9];
    int i = 0;

    for (int cx = -1; cx < 2; ++cx) {
        for (int cy = -1; cy < 2; ++cy) {

            tx = ValidateX(X + cx);

            if (IsInBounds(tx, Y + cy)) {
                if (boots[XY(tx, Y + cy)] == nullptr) {
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


bool Field::AddObject(t_object &obj) {
    if (boots[obj->coord()] != nullptr) return false;
    boots[obj->coord()] = std::move(obj);
    return true;
}

//Tick function for every object,
//Returns true if object was destroyed
[[maybe_unused]] void Field::ObjectTick(int &i_xy) {
    assert(boots[i_xy] != nullptr);
    int t = boots[i_xy]->tick();
    if (t == 2) { return; }

    //Fill brain input structure
    BrainInput b_input;
    auto lookAt = boots[i_xy]->GetDirection();

    //Desired destination,
    //that is what bot is lookinig at
    int cx = boots[i_xy]->x + lookAt.x;
    int cy = boots[i_xy]->y + lookAt.y;

    cx = ValidateX(cx);

    //If destination is out of bounds
    if (!IsInBounds(cx, cy)) {
        b_input.vision = 1.0f; //1 if unpassable
    } else {
        auto cxy = XY(cx, cy);
        //Destination cell is empty

        if (boots[cxy] == nullptr) {
            //0 if empty
            b_input.vision = 0.0f;
        } else {
            //0.5 if someone in that cell
            b_input.vision = 0.5f;

            //Calculate how close they are as relatives, based on mutation markers
//            b_input.isRelative = 1.0f - (boots[cxy]->FindKinship(boots[i_xy]) * 1.0f) /
//                                        (NumberOfMutationMarkers * 1.0f);
            b_input.isRelative = boots[cxy]->dnk.distance(boots[i_xy]->dnk);
        }
    }
    assert(boots[i_xy] != nullptr);
    //Bot brain does its stuff
    auto bots_ideas = boots[i_xy]->think(b_input);
    assert(boots[i_xy] != nullptr);
    //Multiply first
    for (int b = 0; b < bots_ideas.divide; ++b) {
        //Dies if energy is too low
        if (boots[i_xy]->energy <= EnergyPassedToAChild + GiveBirthCost) {
            boots[i_xy] = nullptr;
            return;
        } else {
            //Gives birth otherwise
            auto freeSpace = FindFreeNeighbourCell(boots[i_xy]->x, boots[i_xy]->y);
            if (freeSpace.x != -1) {
                boots[i_xy]->TakeEnergy(EnergyPassedToAChild + GiveBirthCost);
                auto mutation = RandomPercent(MutationChancePercent);
                auto val = MAKE_TObj(freeSpace.x, freeSpace.y,
                                     EnergyPassedToAChild,
                                     boots[i_xy]);
                AddObject(val);
                return;
            }
        }
    }
    assert(boots[i_xy] != nullptr);
    //Then attack
    if (bots_ideas.attack) {
        //If dies of low energy
        if (boots[i_xy]->TakeEnergy(AttackCost)) {
            boots[boots[i_xy]->coord()] = nullptr;
            return;
        } else {
            //Get direction of attack
            auto dir = boots[i_xy]->GetDirection();
            cx = ValidateX(boots[i_xy]->x + dir.x);
            cy = boots[i_xy]->y + dir.y;
            if (IsInBounds(cx, cy)) {
                //If there is an object
                auto cxy = XY(cx, cy);
                if (boots[cxy]) {
                    //Kill an object
                    boots[i_xy]->GiveEnergy(boots[cxy]->energy, kills);
                    boots[cxy] = nullptr;
                } else { // when the attacked escapes
                    // only stay and cry
                }
            }
        }
    } else {
        //Rotate after
        if (bots_ideas.rotate != 0) {
            //If dies of low energy
            if (boots[i_xy]->TakeEnergy(RotateCost)) {
                boots[i_xy] = nullptr;
                return;
            }

            boots[i_xy]->Rotate(bots_ideas.rotate);
        }

        //Move
        if (bots_ideas.move) {
            if (boots[i_xy]->TakeEnergy(MoveCost)) {
                boots[i_xy] = nullptr;
                return;
            }

            auto dir = boots[i_xy]->GetDirection();

            cx = boots[i_xy]->x + dir.x;
            cy = boots[i_xy]->y + dir.y;
            cy = std::max(cy, 0);
            cy = std::min(cy, FieldCellsHeight - 1);

            cx = ValidateX(cx);

//            if ((abs(cx - TEMP_x1) > 1) && (abs(cx - TEMP_x1) < 190)) {
//                TEMP_x1 = bot->x;
//            }

            boots[XY(cx, cy)] = std::move(boots[i_xy]);
        }
            //Photosynthesis
        else if (bots_ideas.photosynthesis) {
            boots[i_xy]->GiveEnergy(GetSunEnergy(boots[i_xy]->x, boots[i_xy]->y), PS);
        }

    }
}

void Field::ObjectTick1(int &i_xy) {
    assert(boots[i_xy] != nullptr);
    int t = boots[i_xy]->tick();
    if (t == 2) { return; }

    //Fill brain input structure
    BrainInput b_input;
    auto lookAt = boots[i_xy]->GetDirection();
    b_input.energy = static_cast<float>(boots[i_xy]->energy);
    b_input.rotation = static_cast<float>(boots[i_xy]->direction);
    //Desired destination,
    //that is what bot is lookinig at
    int cx = boots[i_xy]->x + lookAt.x;
    int cy = boots[i_xy]->y + lookAt.y;

    cx = ValidateX(cx);

    //If destination is out of bounds
    if (!IsInBounds(cx, cy)) {
        b_input.vision = 1.0f; //1 if unpassable
    } else {
        auto cxy = XY(cx, cy);
        //Destination cell is empty
        if (boots[cxy] == nullptr) {
            b_input.vision = 0.0f; //0 if empty
            b_input.isRelative = 0.f;
        } else {
            int lvl;
            if (boots[cxy] != nullptr)
            lvl = boots[cxy]->energy;
            //0.5 if someone in that cell
            b_input.vision = 0.5f;
            if (boots[cxy] != nullptr)
                b_input.isRelative = boots[cxy]->FindKinship(boots[i_xy]);

            b_input.goal_energy = static_cast<float>(lvl);
        }
    }
    assert(boots[i_xy] != nullptr);
    //Bot brain does its stuff
    boots[i_xy]->bots_ideas = boots[i_xy]->think(b_input);

}

void Field::ObjectTick2(int &i_xy) {
    if (boots[i_xy] == nullptr) return;
    auto bots_ideas = boots[i_xy]->bots_ideas;
    //Multiply first
    for (int b = 0; b < bots_ideas.divide; ++b) {
        //Dies if energy is too low
        // TODO make depended creaton on size
        if (boots[i_xy]->energy <= EnergyPassedToAChild + GiveBirthCost) {
            boots[i_xy] = nullptr;
            return;
        } else {
            //Gives birth otherwise
            auto freeSpace = FindFreeNeighbourCell(boots[i_xy]->x, boots[i_xy]->y);
            if (freeSpace.x != -1) {
                boots[i_xy]->TakeEnergy(EnergyPassedToAChild + GiveBirthCost);
                auto mutation = RandomPercent(MutationChancePercent);
                auto val = MAKE_TObj(freeSpace.x, freeSpace.y,
                                     EnergyPassedToAChild,
                                     boots[i_xy]);
                AddObject(val);
                return;
            }
        }
    }
    assert(boots[i_xy] != nullptr);
    //Then attack
    if (bots_ideas.attack) {
        //If dies of low energy
        auto dir = boots[i_xy]->GetDirection();
        auto cx = ValidateX(boots[i_xy]->x + dir.x);
        auto cy = boots[i_xy]->y + dir.y;
        auto cxy = XY(cx, cy);
        if (IsInBounds(cx, cy) && boots[cxy] != nullptr) {
            auto ad_diff = boots[cxy]->dnk.protetction_others - boots[i_xy]->dnk.atack_ability;
            auto ad_summ = boots[cxy]->dnk.protetction_others + boots[i_xy]->dnk.atack_ability;
            auto attack_cost = AttackCost * ad_diff / (ad_summ + 2) + AttackCost;
            if (boots[i_xy]->TakeEnergy(attack_cost)) {
                boots[boots[i_xy]->coord()] = nullptr;
                return;
            } else { //Kill an object
                boots[i_xy]->GiveEnergy(boots[cxy]->energy, kills);
                boots[cxy] = nullptr;
            }
        }
    } else {
        //Rotate after
        if (bots_ideas.rotate != 0) {
            //If dies of low energy
            if (boots[i_xy]->TakeEnergy(RotateCost)) {
                boots[i_xy] = nullptr;
                return;
            }

            boots[i_xy]->Rotate(bots_ideas.rotate);
        }

        //Move
        if (bots_ideas.move) { // TODO make move cost on weight and terrain
            if (boots[i_xy]->TakeEnergy(MoveCost)) {
                boots[i_xy] = nullptr;
                return;
            }

            auto dir = boots[i_xy]->GetDirection();

            auto cx = boots[i_xy]->x + dir.x;
            auto cy = boots[i_xy]->y + dir.y;
            cy = std::max(cy, 0);
            cy = std::min(cy, FieldCellsHeight - 1);
            cx = ValidateX(cx);
            boots[XY(cx, cy)] = std::move(boots[i_xy]);
        }
            //Photosynthesis
        else if (bots_ideas.photosynthesis) { // TOO make photosyntes effectivity
            auto ps_ab = (10 + boots[i_xy]->dnk.ps_ability) / 10;
            auto sun = GetSunEnergy(boots[i_xy]->x, boots[i_xy]->y) + ps_ab;
            boots[i_xy]->GiveEnergy(sun, PS);
        }
    }
}

//tick function for single threaded build
inline void Field::tick_single_thread() {
    objectsTotal = 0;
    auto f1 = [&](int i_xy) {
        if (boots[i_xy]) {
            if (boots[i_xy]->tick() == 1) {
                // too old or expired
                boots[i_xy] = nullptr;
                return;
            }
            auto [x_, y_] = XYr(i_xy);
            boots[i_xy]->GiveEnergy(terrain[x_][y_] == Terrain::eart ?
                                    FoodbaseMineralsTerrain : FoodbaseMineralsSea,
                                    EnergySource::mineral);
            ObjectTick1(i_xy);
        }
    };
//    tbb::parallel_for(0, FieldCellsWidth * FieldCellsHeight, f1);
    const auto total = FieldCellsWidth * FieldCellsHeight;
    auto f2 = [&](int i_xy) {
//    for (auto i_xy = 0; i_xy < FieldCellsWidth * FieldCellsHeight; i_xy++) {
        auto m_ix = sequence[i_xy];
        if (boots[m_ix]) {
            ++objectsTotal;
            ObjectTick2(m_ix);
        }
    };


//    tbb::parallel_for(0, total / 4, f);
//    tbb::parallel_for(total / 4, total / 2, f);
//    tbb::parallel_for(total / 2, total * 3 / 4, f);
//    tbb::parallel_for(total * 4 / 4, total, f);
    for (auto i = 0; i < total; i++) f1(i);
    for (auto i = 0; i < total; i++) f2(i);

    unsigned long sPS{0}, sK{0}, sM{0};
    for (auto i = 0; i < total; i++) {
        if (boots[i] == nullptr) continue;
        sPS += boots[i]->energyFromPS;
        sK += boots[i]->energyFromKills;
        sM += boots[i]->energyFromMinerals;
    }
    if (frame_number % 10 != 9) return;
    unsigned long E = sPS + sK + sM + 1;
    std::cout << " Total:" << E
              << "PS:" << 100 * sPS / E << " Kills:" << 100 * sK / E << " Minerals:" << 100 * sM / E << std::endl;
}

//Tick function
void Field::tick(int thisFrame) {
//    Object::currentFrame = thisFrame;
    if (!run) return;
    frame_number++;
    tick_single_thread();
}


//Draw simulation field with all its objects
void Field::draw(frame_type &image) {
    BGround.copyTo(image);
    auto f = [&](int m_ix) {
        if (boots[m_ix]) {
            //Draw function switch, based on selected render type
            switch (render) {
                case natural:
                    boots[m_ix]->draw(image);
                    break;
                case predators:
                    boots[m_ix]->drawPredators(image);
                    break;
                case energy:
                    boots[m_ix]->drawEnergy(image);
                    break;
            }
        }
    };
    tbb::parallel_for(0, FieldCellsWidth * FieldCellsHeight, f);
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
    if (X < 0) {
        return (X % FieldCellsWidth) + FieldCellsWidth;
    } else if (X >= FieldCellsWidth) {
        return (X % FieldCellsWidth);
    }
    return X;
}


//Spawn group of random bots
void Field::SpawnControlGroup() {
    int x, y, xy, cnt{0};
    for (int i = 0; i < ControlGroupSize * 3; i++) {
        x = RandomVal(FieldCellsWidth);
        y = RandomVal(FieldCellsHeight);
        xy - XY(x, y);
        if (boots[xy] != nullptr) continue;
        cnt++;
        if (cnt > ControlGroupSize) break;
        auto tmpBot = MAKE_TObj(x, y, 100);
        AddObject(tmpBot);
    }
}

//Create field
Field::Field() {
    for (auto i = 0; i < FieldCellsHeight * FieldCellsWidth; i++) boots.push_back(nullptr);
    //Clear array
    int xy;
    int main_ix = 0;
    for (auto x0 = 0; x0 < tbb_step; x0++)
        for (auto y0 = 0; y0 < tbb_step; y0++)
            for (auto x = 0; x < FieldCellsWidth; x += tbb_step)
                for (auto y = 0; y < FieldCellsHeight; y += tbb_step) {
                    xy = (x + x0) * FieldCellsWidth + y + y0;
                    assert(std::find(sequence.begin(), sequence.end(), (xy)) == sequence.end());
                    sequence.push_back(xy);
                    main_ix++;
                }

    // load old europe for ex.
    auto img = cv::imread("/home/imt/work/BrainLife/fields/field_base_eu.jpg", CV_8UC1);
    cv::resize(img, img, cv::Size(FieldCellsWidth, FieldCellsHeight));
    frame_type sBGround{FieldCellsWidth, FieldCellsHeight, CV_8UC3};
    sBGround.setTo(color_sea);
    sBGround.setTo(color_earth, img);
    cv::resize(sBGround, BGround, cv::Size(FieldWidth, FieldHeight));
    for (auto x = 0; x < FieldCellsWidth; x++)
        for (auto y = 0; y < FieldCellsHeight; y++) {
            if (img.at<uchar>(x, y) > 127) {
                terrain[x][y] = Terrain::eart;
            } else {
                terrain[x][y] = Terrain::sea;
            }
        }

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

    int x{FieldX}, y{FieldY}, baseline{0};
    auto font = cv::FONT_HERSHEY_SIMPLEX;
    auto font_size = 0.85;
    auto font_thickness = 2;
    for (const auto &l: lines) {
        auto textSize = cv::getTextSize(l, font, font_size, font_thickness, &baseline);
        y += textSize.height + font_thickness;
        auto text_org = cv::Point(x, y);
        cv::putText(image, l, text_org, font, font_size,
                    cv::Scalar(255, 100, 200), font_thickness, 8);
    }


}

int Field::GetSunEnergy(int x, int y) const {
    auto day_part = (int) (frame_number % p_24) * 4 / p_24;
    auto year_part = (int) (frame_number % p_year) * 4 / p_year;
    auto region = y / Region_Polar;
    const int d[] = {0, 2, 4, 3};
    const int st[] = {1, 3, 3, 3};
    const int nt[] = {1, 2, 3, 2};
    const int pt[] = {0, 0, 2, 1};
    int out;
    if (region == 3) {
        out = PhotosynthesisReward_Summer * (d[day_part]) / 4;
    } else if (region == 2) {
        out = PhotosynthesisReward_Summer * (d[day_part]) * (st[year_part]) / 16;
    } else if (region == 1) {
        out = PhotosynthesisReward_Summer * (d[day_part]) * (nt[year_part]) / 16;
    } else {
        out = PhotosynthesisReward_Summer * (d[day_part]) * (pt[year_part]) / 16;
    }
    if (terrain[x][y] == Terrain::sea) out /= 2;
    return out;
}

void Field::ShowMutations() {

    int max_energy{0};
    int protetction_front{0};
    int protetction_others{0};
    int atack_ability{0};
    int minerals_ability{0};
    int ps_ability{0};
    int mutability_body{0};
    int mutability_brain{0};
    int max_life_time{0};
    for (auto ix = 0; ix < FieldCellsWidth *FieldCellsHeight; ix++){
        if (boots[ix] == nullptr) continue;
        max_energy = std::max(max_energy, boots[ix]->dnk.max_energy);
        protetction_front = std::max(protetction_front, boots[ix]->dnk.protetction_front);
        protetction_others = std::max(protetction_others, boots[ix]->dnk.protetction_others);
        atack_ability = std::max(atack_ability, boots[ix]->dnk.atack_ability);
        minerals_ability = std::max(minerals_ability, boots[ix]->dnk.minerals_ability);
        ps_ability = std::max(ps_ability, boots[ix]->dnk.ps_ability);
        mutability_body = std::max(mutability_body, boots[ix]->dnk.mutability_body);
        mutability_brain = std::max(mutability_brain, boots[ix]->dnk.mutability_brain);
        max_life_time = std::max(max_life_time, boots[ix]->dnk.max_life_time);
    }

//    for (auto x = 0; x < FieldCellsWidth; x++)
//        for (auto y = 0; y < FieldCellsHeight; y++) {
//            if (img.at<uchar>(x, y) > 127) {
//                terrain[x][y] = Terrain::eart;
//            } else {
//                terrain[x][y] = Terrain::sea;
//            }
//        }

}




