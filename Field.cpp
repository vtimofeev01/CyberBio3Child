#include <tbb/parallel_for.h>
#include "Field.h"
#include "MyTypes.h"
#include <math.h>

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
            tx = X + cx;
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


bool Field::AddObject(t_object &obj, int _x, int _y) {
    auto ixy = XY(_x, _y);
    if (boots[ixy] != nullptr) return false;
    boots[ixy] = std::move(obj);
    return true;
}

bool Field::AddObject(t_object &obj, int coord) {
    if (boots[coord] != nullptr) return false;
    boots[coord] = std::move(obj);
    return true;
}

void Field::ObjectTick1(int &i_xy) {
    assert(boots[i_xy] != nullptr);
    auto [i_x, i_y] = XYr(i_xy);
    int t = boots[i_xy]->tick();
    if (t == 2) { return; }

    //Fill brain input structure
    BrainInput b_input;

    auto lookAt = boots[i_xy]->GetDirection();
    b_input.energy = static_cast<float>(boots[i_xy]->energy);
    b_input.rotation = static_cast<float>(boots[i_xy]->direction);
    //Desired destination,
    //that is what bot is looking at
    int cx = i_x + lookAt.x;
    int cy = i_y + lookAt.y;
    b_input.local_terrain = terrain[i_x][i_y];
    b_input.direct_terrain = terrain[cx][cy];

//    cx = ValidateX(cx);

    //If destination is out of bounds
    if (!IsInBounds(cx, cy)) {
        b_input.vision = 1.0f; //1 if unpassable
    } else {
        auto cxy = XY(cx, cy);
        //Destination cell is empty
        if (boots[cxy] == nullptr) {
            b_input.vision = 0.0f; //0 if empty
            b_input.isRelative = 0.f;
            b_input.goal_energy = static_cast<float>(sun_power[cx][cy] +
                    terrain[cx][cy] == Terrain::earth ? FoodbaseMineralsTerrain : FoodbaseMineralsSea);
        } else {
            int lvl;
            assert(boots[cxy] != nullptr);
            lvl = boots[cxy]->energy;
            //0.5 if someone in that cell
            b_input.vision = 0.5f;
            b_input.isRelative = boots[cxy]->FindKinship(boots[i_xy]);
            b_input.goal_energy = static_cast<float>(lvl);
        }
    }
    b_input.local_terrain = terrain[i_x][i_y];
    b_input.direct_terrain = terrain[cx][cy];
    assert(boots[i_xy] != nullptr);
    //Bot brain does its stuff
    boots[i_xy]->bots_ideas = boots[i_xy]->think(b_input);

}

void Field::ObjectTick2(int &i_xy) {
    if (boots[i_xy] == nullptr) return;
    auto bots_ideas = boots[i_xy]->bots_ideas;
    auto [i_x, i_y] = XYr(i_xy);
    // i am a tree


    //Multiply
    for (int b = 0; b < bots_ideas.divide; ++b) {
        //Dies if energy is too low
        // TODO make depended creation on size
        if (boots[i_xy]->energy > EnergyPassedToAChild + GiveBirthCost) {
            //Gives birth otherwise
            auto freeSpace = FindFreeNeighbourCell(i_x, i_y);
            if (IsInBounds(freeSpace)) {
                boots[i_xy]->TakeEnergy(EnergyPassedToAChild + GiveBirthCost);
                auto val = MAKE_TObj( EnergyPassedToAChild,
                                     boots[i_xy]);
                AddObject(val, XY(freeSpace.x, freeSpace.y));
                boots[i_xy]->stat_birth++;
            }
        }
    }
    assert(boots[i_xy] != nullptr);

    //Then attack
    if (bots_ideas.attack) {
        //If dies of low energy
        auto dir = boots[i_xy]->GetDirection();
        auto cx = i_x + dir.x; // ValidateX(i_x + dir.x);
        auto cy = i_y + dir.y;
        auto cxy = XY(cx, cy);
        assert(cxy != i_xy);
        if (IsInBounds(cx, cy) && boots[cxy] != nullptr) {
            auto defense = boots[cxy]->dnk.def_all;

            // see where looks the attacked
            auto cxy_directon = boots[cxy]->GetDirection();
            auto cx_d_x = cx + cxy_directon.x;
            auto cx_d_y = cy + cxy_directon.y;
            if ((cx_d_x == i_x) && (cx_d_y == i_y)) defense += boots[cxy]->dnk.def_front;


            auto ad_diff = defense - boots[i_xy]->dnk.kill_ability;
            auto ad_summ = defense + boots[i_xy]->dnk.kill_ability;
            auto attack_cost = AttackCost * ad_diff / (ad_summ + 2) + AttackCost;
            if (boots[i_xy]->energy > attack_cost) { //Kill an object
                boots[i_xy]->stat_kills++;
                boots[i_xy]->TakeEnergy(attack_cost);
                boots[i_xy]->GiveEnergy(boots[cxy]->energy, kills);
                boots[cxy] = nullptr;
            }
        }
    }

    assert(boots[i_xy] != nullptr);
    if (bots_ideas.rotate != 0) {
        //If dies of low energy
        if (boots[i_xy]->energy > RotateCost) {
            boots[i_xy]->Rotate(bots_ideas.rotate);
            boots[i_xy]->TakeEnergy(RotateCost);
        }
    }

    //Move
    //Photosynthesis
    if (bots_ideas.photosynthesis) { // TOO make photosynthesis effectivity
        auto ps_ab = (10 + boots[i_xy]->dnk.ps_ability) / 10;
        auto sun = GetSunEnergy(i_x, i_y) * ps_ab;
        boots[i_xy]->GiveEnergy(sun, PS);
        return;
    }

    assert(boots[i_xy] != nullptr);
    if (bots_ideas.move) { // TODO make move cost on weight and terrain
        auto mc = terrain[i_x][i_y] == Terrain::earth ? MoveCost : MoveCost / 2;
        if (boots[i_xy]->energy > mc) {
            auto dir = boots[i_xy]->GetDirection();
            auto cx = i_x + dir.x;
            auto cy = i_y + dir.y;
//            cy = std::max(cy, 0);
//            cx = ValidateX(cx);
            auto c_xy = XY(cx, cy);
            if (IsInBounds(cx, cy) && (boots[c_xy] == nullptr)) {
                boots[i_xy]->stat_steps++;
                boots[i_xy]->TakeEnergy(MoveCost);
                boots[c_xy] = std::move(boots[i_xy]);
            } else {
                boots[i_xy] = nullptr;
            }
        }
    }

}


//tick function for single threaded build
inline void Field::tick_single_thread() {
    objectsTotal = 0;
    updateSunEnergy();
    auto f1 = [&](int i_xy) {
        if (boots[i_xy]) {
            auto [x_, y_] = XYr(i_xy);
            if (boots[i_xy]->tick() == 1) {
                organic[x_][y_] = GiveBirthCost;
                // too old or expired
                boots[i_xy] = nullptr;
                return;
            }

            boots[i_xy]->GiveEnergy(terrain[x_][y_] == Terrain::earth ?
                                    FoodbaseMineralsTerrain : FoodbaseMineralsSea,
                                    EnergySource::mineral);
            auto till_limit = boots[i_xy]->dnk.max_energy - boots[i_xy]->energy;
            auto to_take = std::min(till_limit, organic[x_][y_]);
            if (to_take) {
                boots[i_xy]->GiveEnergy(to_take, EnergySource::ES_garbage);
                organic[x_][y_] -= to_take;
                assert(organic[x_][y_] >= 0);
            }
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
    for (auto i = 0; i < total; i++) f1(i);
    for (auto i = 0; i < total; i++) f2(i);

    unsigned long sPS{0}, sK{0}, sM{0}, kills{0}, birth{0}, steps{0};
    for (auto i = 0; i < total; i++) {
        if (boots[i] == nullptr) continue;
        sPS += boots[i]->energyFromPS;
        sK += boots[i]->energyFromKills;
        sM += boots[i]->energyFromMinerals;
        kills += boots[i]->stat_kills;
        birth += boots[i]->stat_birth;
        steps += boots[i]->stat_steps;
    }
    if (frame_number % 10 != 9) return;
    unsigned long E = sPS + sK + sM + 1;
    std::cout << " Total:" << E
              << "PS:" << 100 * sPS / E << " Kills:" << 100 * sK / E << " Minerals:" << 100 * sM / E
              << " kills:" << kills << " birth:" << birth << " steps:" << steps
              << std::endl;
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
    std::vector<std::string> extra_data;
    auto f = [&](int m_ix) {
        if (boots[m_ix]) {
            //Draw function switch, based on selected render type
            switch (render) {
                case natural:
                    boots[m_ix]->draw(image, m_ix);
                    break;
                case predators:
                    boots[m_ix]->drawPredators(image, m_ix);
                    break;
                case energy:
                    boots[m_ix]->drawEnergy(image, m_ix);
                    break;
            }
        }
    };

    switch (render) {

        case natural: {
            tbb::parallel_for(0, FieldCellsWidth * FieldCellsHeight, f);
            break;
        }
        case predators: {
            tbb::parallel_for(0, FieldCellsWidth * FieldCellsHeight, f);
            break;
        }
        case energy: {
            tbb::parallel_for(0, FieldCellsWidth * FieldCellsHeight, f);
            break;
        }
        case sun_energy:
        {
            auto max_val = drawAnyGrayScale(image, &sun_power);
//            std::cout << max_val << std::endl;
            std::ostringstream v;
            v << "max val:" << max_val;
            extra_data.emplace_back("SUN");
            extra_data.emplace_back(v.str());
        }
            break;
        case max_energy:
        {
            fill_buf_2_draw(max_energy);
            auto max_val = drawAnyGrayScale(image, &tmp_buf2draw);
            std::ostringstream v;
            v << "max val:" << max_val;
            extra_data.emplace_back("max_energy");
            extra_data.emplace_back(v.str());
        }
            break;
        case def_front:
        {
            fill_buf_2_draw(def_front);
            auto max_val = drawAnyGrayScale(image, &tmp_buf2draw);
            std::ostringstream v;
            v << "max val:" << max_val;
            extra_data.emplace_back("def_front");
            extra_data.emplace_back(v.str());
        }
            break;
        case def_all:
        {
            fill_buf_2_draw(def_all);
            auto max_val = drawAnyGrayScale(image, &tmp_buf2draw);
            std::ostringstream v;
            v << "max val:" << max_val;
            extra_data.emplace_back("def_all");
            extra_data.emplace_back(v.str());
        }
            break;
        case kill_ability:
        {
            fill_buf_2_draw(kill_ability);
            auto max_val = drawAnyGrayScale(image, &tmp_buf2draw);
            std::ostringstream v;
            v << "max val:" << max_val;
            extra_data.emplace_back("kill_ability");
            extra_data.emplace_back(v.str());
        }
            break;
        case minerals_ability:
        {
            fill_buf_2_draw(minerals_ability);
            auto max_val = drawAnyGrayScale(image, &tmp_buf2draw);
            std::ostringstream v;
            v << "max val:" << max_val;
            extra_data.emplace_back("minerals_ability");
            extra_data.emplace_back(v.str());
        }
            break;
        case ps_ability:
        {
            fill_buf_2_draw(ps_ability);
            auto max_val = drawAnyGrayScale(image, &tmp_buf2draw);
            std::ostringstream v;
            v << "max val:" << max_val;
            extra_data.emplace_back("ps_ability");
            extra_data.emplace_back(v.str());
        }
            break;
        case mutability_body:
        {
            fill_buf_2_draw(mutability_body);
            auto max_val = drawAnyGrayScale(image, &tmp_buf2draw);
            std::ostringstream v;
            v << "max val:" << max_val;
            extra_data.emplace_back("mutability_body");
            extra_data.emplace_back(v.str());
        }
            break;
        case mutability_brain:
        {
            fill_buf_2_draw(mutability_brain);
            auto max_val = drawAnyGrayScale(image, &tmp_buf2draw);
            std::ostringstream v;
            v << "max val:" << max_val;
            extra_data.emplace_back("mutability_brain");
            extra_data.emplace_back(v.str());
        }
            break;
        case max_life_time:
        {
            fill_buf_2_draw(max_life_time);
            auto max_val = drawAnyGrayScale(image, &tmp_buf2draw);
            std::ostringstream v;
            v << "max val:" << max_val;
            extra_data.emplace_back("max_life_time");
            extra_data.emplace_back(v.str());
        }
            break;
        case ::garb:
        {
            fill_buf_2_draw(garb);
            auto max_val = drawAnyGrayScale(image, &tmp_buf2draw);
            std::ostringstream v;
            v << "max val:" << max_val;
            extra_data.emplace_back("Garbage");
            extra_data.emplace_back(v.str());
        }
            break;
        case lifetime:
        {
            fill_buf_2_draw(lifetime);
            auto max_val = drawAnyGrayScale(image, &tmp_buf2draw);
            std::ostringstream v;
            v << "max val:" << max_val;
            extra_data.emplace_back("lifetime");
            extra_data.emplace_back(v.str());
        }
            break;
        case fertility:
        {
            fill_buf_2_draw(fertility);
            auto max_val = drawAnyGrayScale(image, &tmp_buf2draw);
            std::ostringstream v;
            v << "max val:" << max_val;
            extra_data.emplace_back("fertility");
            extra_data.emplace_back(v.str());
        }
            break;
    }

    Annotate(image, extra_data, cv::Scalar(0, 0, 255));
}

//Is cell out if bounds?
bool Field::IsInBounds(int X, int Y) {
    return ((X >= 0) && (Y >= 0) && (X < FieldCellsWidth) && (Y < FieldCellsHeight));
}

bool Field::IsInBounds(oPoint &p) {
    return IsInBounds(p.x, p.y);
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
        auto tmpBot = MAKE_TObj(100);
        AddObject(tmpBot, x, y);
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
            organic[x][y] = 0;
            if (img.at<uchar>(x, y) > 127) {
                terrain[y][x] = Terrain::earth;
            } else {
                terrain[y][x] = Terrain::sea;
            }
        }

}

void Field::NextView() {
    if (render == RenderTypes::natural) render = RenderTypes::energy;
    else if (render == RenderTypes::energy) render = RenderTypes::predators;
    else if (render == RenderTypes::predators) render = RenderTypes::sun_energy;
    else if (render == RenderTypes::sun_energy) render = RenderTypes::max_energy;
    else if (render == RenderTypes::max_energy) render = RenderTypes::def_front;
    else if (render == RenderTypes::def_front) render = RenderTypes::def_all;
    else if (render == RenderTypes::def_all) render = RenderTypes::kill_ability;
    else if (render == RenderTypes::kill_ability) render = RenderTypes::minerals_ability;
    else if (render == RenderTypes::minerals_ability) render = RenderTypes::ps_ability;
    else if (render == RenderTypes::ps_ability) render = RenderTypes::mutability_body;
    else if (render == RenderTypes::mutability_body) render = RenderTypes::mutability_brain;
    else if (render == RenderTypes::mutability_brain) render = RenderTypes::max_life_time;
    else if (render == RenderTypes::max_life_time) render = RenderTypes::garb;
    else if (render == RenderTypes::garb) render = RenderTypes::lifetime;
    else if (render == RenderTypes::lifetime) render = RenderTypes::fertility;
    else if (render == RenderTypes::fertility) render = RenderTypes::natural;
}

void Field::Annotate(frame_type &image, const std::vector<std::string> &extra, const cv::Scalar& color) const {

    std::vector<std::string> lines;
    if (render == RenderTypes::natural) lines.emplace_back("view: natural");
    else if (render == RenderTypes::energy) lines.emplace_back("view: energy");
    else if (render == RenderTypes::predators) lines.emplace_back("view: predators");
    std::ostringstream stringStream;
    stringStream << "gen:" << frame_number;
    lines.emplace_back(stringStream.str());
    for (auto &s: extra) lines.push_back(s);

    int x{FieldX}, y{FieldY}, baseline{0};
    auto font = cv::FONT_HERSHEY_SIMPLEX;
    auto font_size = 0.85;
    auto font_thickness = 2;
    for (const auto &l: lines) {
        auto textSize = cv::getTextSize(l, font, font_size, font_thickness, &baseline);
        y += textSize.height + font_thickness;
        auto text_org = cv::Point(x, y);
        cv::putText(image, l, text_org, font, font_size,
                    color, font_thickness, 8);
    }
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
    for (auto ix = 0; ix < FieldCellsWidth * FieldCellsHeight; ix++) {
        if (boots[ix] == nullptr) continue;
        max_energy = std::max(max_energy, boots[ix]->dnk.max_energy);
        protetction_front = std::max(protetction_front, boots[ix]->dnk.def_front);
        protetction_others = std::max(protetction_others, boots[ix]->dnk.def_all);
        atack_ability = std::max(atack_ability, boots[ix]->dnk.kill_ability);
        minerals_ability = std::max(minerals_ability, boots[ix]->dnk.minerals_ability);
        ps_ability = std::max(ps_ability, boots[ix]->dnk.ps_ability);
        mutability_body = std::max(mutability_body, boots[ix]->dnk.mutability_body);
        mutability_brain = std::max(mutability_brain, boots[ix]->dnk.mutability_brain);
        max_life_time = std::max(max_life_time, boots[ix]->dnk.max_life_time);
    }
    // TODO out mutation map
    // TODO add solar map
    // TODO out bot descr
//    for (auto x = 0; x < FieldCellsWidth; x++)
//        for (auto y = 0; y < FieldCellsHeight; y++) {
//            if (img.at<uchar>(x, y) > 127) {
//                terrain[x][y] = Terrain::earth;
//            } else {
//                terrain[x][y] = Terrain::sea;
//            }
//        }

}

void Field::updateSunEnergy() {
    const float deg_up = 80.f;
    const float deg_low = 10.f;
    const float deg_one = (deg_up - deg_low) / FieldCellsHeight;

    const float deg_earth_axe = 23.f;
    const float deg_earth_axe_per_day = 2.f * deg_earth_axe / p_half_year;

    const auto gra_to_rad = 3.14159265f/180;

    auto day_of_year = static_cast<int>(frame_number % p_year);
    auto deg_cur_axe = deg_earth_axe - deg_earth_axe_per_day * std::abs(day_of_year - p_half_year);
//    std::cout << "deg_cur_axe:" << deg_cur_axe;

    for (auto y = 0; y < FieldCellsHeight; y++) {
        auto terr_deg = deg_cur_axe + deg_low + deg_one * static_cast<float>(FieldCellsHeight - y);
        float coef;
        if (terr_deg > 90.f) {coef = 0;}
        else {
            coef = std::cos(terr_deg * gra_to_rad);
        }
        int sp = static_cast<int>(PhotosynthesisReward_Summer * coef);
        for (auto x = 0; x < FieldCellsWidth; x++)
//            sun_power[x][y] = CalcSunEnergy(x, y);
            sun_power[x][y] = terrain[x][y] == Terrain::earth ? sp:sp/2;
    }
}

int Field::GetSunEnergy(int x, int y) const {
    return sun_power[x][y];
}

int Field::drawAnyGrayScale(frame_type &image, int (*data)[FieldCellsWidth][FieldCellsHeight]) {

    int max_val{-10000};
    int min_val{100000000};
    for (auto x = 0; x < FieldCellsWidth; x++)
        for (auto y = 0; y < FieldCellsHeight; y++) {
            max_val = std::max(max_val, (*data)[x][y]);
            min_val = std::min(min_val, (*data)[x][y]);
        }
    int c_;
    for (auto x = 0; x < FieldCellsWidth; x++)
        for (auto y = 0; y < FieldCellsHeight; y++) {
            c_ = (*data)[x][y] * 255 / (max_val + 1);
            cv::rectangle(image,
                          cv::Point(FieldX + x * FieldCellSize + 1, FieldY + y * FieldCellSize + 1),
                          cv::Point(FieldX + x * FieldCellSize + FieldCellSize - 1,
                                    FieldY + y * FieldCellSize + FieldCellSize - 1),
                          cv::Scalar(c_, c_, c_),
                          -1,
                          cv::LINE_8, 0);
        }
    return max_val;

}

void Field::fill_buf_2_draw(RenderTypes val) {
    int t_val, o_val;
    for (auto x = 0; x < FieldCellsWidth; x++)
        for (auto y = 0; y < FieldCellsHeight; y++)
        {
            t_val = XY(x, y);
            if (boots[t_val] == nullptr) { tmp_buf2draw[x][y] = 0; }
            else {
                switch (val) {
                    case natural:
                        break;
                    case predators:
                        break;
                    case energy:
                        break;
                    case sun_energy:
                        break;
                    case max_energy:
                        o_val = boots[t_val]->dnk.max_energy;
                        break;
                    case def_front:
                        o_val = boots[t_val]->dnk.def_front;
                        break;
                    case def_all:
                        o_val = boots[t_val]->dnk.def_all;
                        break;
                    case kill_ability:
                        o_val = boots[t_val]->dnk.kill_ability;
                        break;
                    case minerals_ability:
                        o_val = boots[t_val]->dnk.minerals_ability;
                        break;
                    case ps_ability:
                        o_val = boots[t_val]->dnk.ps_ability;
                        break;
                    case mutability_body:
                        o_val = boots[t_val]->dnk.mutability_body;
                        break;
                    case mutability_brain:
                        o_val = boots[t_val]->dnk.mutability_brain;
                        break;
                    case max_life_time:
                        o_val = boots[t_val]->dnk.max_life_time;
                        break;
                    case garb:
                        o_val = organic[x][y];
                        break;
                    case lifetime:
                        o_val = boots[t_val]->lifetime;
                        break;
                    case fertility:
                        o_val = boots[t_val]->dnk.fertilityDelay;
                        break;
                }
                tmp_buf2draw[x][y] = o_val;
            }

        }
}




// TODO add organic after death
// TODO prpbably add organic on photosynthes