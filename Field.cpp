#include <tbb/parallel_for.h>
#include "Field.h"
#include "MyTypes.h"
#include <cmath>

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
    assert(organic[i_x][i_y] >= 0);
    assert(organic[i_x][i_y] >= 0);
    //Fill brain input structure
    BrainInput b_input;

    auto lookAt = boots[i_xy]->GetDirection();
    b_input.energy = static_cast<float>(1000 * (boots[i_xy]->energy + 1) / (boots[i_xy]->dnk.max_energy + 1)) / 1000.f;
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
            b_input.isRelative = -1.f;

        } else {
            assert(boots[cxy] != nullptr);
            //0.5 if someone in that cell
            b_input.vision = 0.5f;
            b_input.isRelative = boots[i_xy]->FindKinship(boots[cxy]);
            b_input.goal_energy = static_cast<float>(boots[cxy]->energy);
        }
        auto min_terra = terrain[cx][cy] == Terrain::earth ? FoodbaseMineralsTerrain
                                                     : FoodbaseMineralsSea;
        b_input.goal_energy += static_cast<float>(sun_power[cx][cy] + min_terra);
    }
    assert(organic[i_x][i_y] >= 0);
    assert(organic[i_x][i_y] >= 0);
    b_input.local_terrain = terrain[i_x][i_y];
    b_input.direct_terrain = terrain[cx][cy];
    assert(boots[i_xy] != nullptr);
    //Bot brain does its stuff
    boots[i_xy]->bots_ideas = boots[i_xy]->think(b_input);
    assert(organic[i_x][i_y] >= 0);
    assert(organic[i_x][i_y] >= 0);

}

void Field::ObjectTick2(int &i_xy) {
    if (boots[i_xy] == nullptr) return;
    auto bots_ideas = boots[i_xy]->bots_ideas;
    auto [i_x, i_y] = XYr(i_xy);
    assert(organic[i_x][i_y] >= 0);
    assert(organic[i_x][i_y] < 50000 );
    //Multiply
    for (int b = 0; b < bots_ideas.divide; ++b) {
        //Dies if energy is too low
        // TODO make depended creation on size
        // TODO add EnergyPassedToAChild to DNK
        auto birth_cost = GiveBirthCost;
        auto birthE = boots[i_xy]->dnk.energy_given_on_birth + birth_cost;
        if (boots[i_xy]->energy > birthE) {
            //Gives birth otherwise
            auto freeSpace = FindFreeNeighbourCell(i_x, i_y);
            if (IsInBounds(freeSpace)) {
                boots[i_xy]->TakeEnergy(birthE);
                auto val = MAKE_TObj(boots[i_xy]->dnk.energy_given_on_birth,
                                     boots[i_xy]);
                AddObject(val, XY(freeSpace.x, freeSpace.y));
                boots[i_xy]->stat_birth++;
                boots[i_xy]->step_spend_birth = GiveBirthCost + boots[i_xy]->dnk.energy_given_on_birth;
            }
        }
    }
    assert(boots[i_xy] != nullptr);
    assert(organic[i_x][i_y] >= 0);
    assert(organic[i_x][i_y] < 50000 );
    //Then attack
    if (bots_ideas.attack) {
        // No attack if low energy
        auto dir = boots[i_xy]->GetDirection();
        auto cx = i_x + dir.x; // ValidateX(i_x + dir.x);
        auto cy = i_y + dir.y;
        auto cxy = XY(cx, cy);
        assert(cxy != i_xy);
        if (IsInBounds(cx, cy) && (boots[cxy] != nullptr)) {
            auto defense = boots[cxy]->dnk.def_all;
            // see where looks the attacked
            auto cxy_directon = boots[cxy]->GetDirection();
            auto cx_d_x = cx + cxy_directon.x;
            auto cx_d_y = cy + cxy_directon.y;
            if ((cx_d_x == i_x) && (cx_d_y == i_y)) defense += boots[cxy]->dnk.def_front;
            auto attack_cost = AttackCost * (defense + 1) /(boots[i_xy]->dnk.kill_ability + 1) ;
            assert(attack_cost > 0);
            if (boots[i_xy]->energy > attack_cost) { //Kill an object
                boots[i_xy]->step_spend_attack = attack_cost;
                boots[i_xy]->stat_kills++;
                boots[i_xy]->TakeEnergy(attack_cost);
                auto goes_to_organic = boots[i_xy]->GiveEnergy(boots[cxy]->energy, kills);
                organic[i_x][i_y] += goes_to_organic;
                boots[cxy] = nullptr;
                assert(organic[i_x][i_y] >= 0);
                assert(organic[i_x][i_y] < 50000 );
            }
            assert(organic[i_x][i_y] >= 0);
            assert(organic[i_x][i_y] < 50000 );
        }
        assert(organic[i_x][i_y] >= 0);
        assert(organic[i_x][i_y] < 50000 );
    }
    assert(organic[i_x][i_y] >= 0);
    assert(organic[i_x][i_y] < 50000 );
    assert(boots[i_xy] != nullptr);
    if (bots_ideas.rotate != 0) {
        //If dies of low energy
        if (boots[i_xy]->energy > RotateCost) {
            boots[i_xy]->Rotate(bots_ideas.rotate);
            boots[i_xy]->TakeEnergy(RotateCost);
            boots[i_xy]->step_spend_rotate = RotateCost;
        }
    }
    assert(organic[i_x][i_y] >= 0);
    assert(organic[i_x][i_y] < 50000 );
    //Move
    //Photosynthesis
    if (bots_ideas.photosynthesis) { // TOO make photosynthesis effectivity
        auto ps_ab = std::sqrt((10 + boots[i_xy]->dnk.ps_ability) / 10);
        auto sun = static_cast<int>(GetSunEnergy(i_x, i_y) * ps_ab);
        // TODO extra sun to garbage @chernosjom@
        auto goes_to_org = boots[i_xy]->GiveEnergy(sun, PS);
        organic[i_x][i_y] += goes_to_org;
        boots[i_xy]->stat_ps++;
        // TODO WTF
//        organic[i_x][i_y] = std::max(0, organic[i_x][i_y]);
        assert(organic[i_x][i_y] >= 0);
        // and take minerals
        auto mineral = terrain[i_x][i_y] == Terrain::earth ? FoodbaseMineralsTerrain : FoodbaseMineralsSea;
        organic[i_x][i_y] += boots[i_xy]->GiveEnergy(mineral, EnergySource::mineral);

        assert(organic[i_x][i_y] >= 0);
        auto till_limit = boots[i_xy]->dnk.max_energy - boots[i_xy]->energy;
        auto to_take = std::min(till_limit, organic[i_x][i_y]);
        if (to_take > 0) {
            boots[i_xy]->GiveEnergy(to_take, EnergySource::ES_garbage);
            organic[i_x][i_y] -= to_take;
            assert(organic[i_x][i_y] >= 0);
        }
        assert(organic[i_x][i_y] >= 0);
        assert(organic[i_x][i_y] >= 0);
        if (organic[i_x][i_y] >= 0) organic[i_x][i_y] = 40000;
        return;
    }

    assert(boots[i_xy] != nullptr);
    if (bots_ideas.move) { // TODO make move cost on weight and terrain
        auto mc = MoveCost;
        mc += boots[i_xy]->dnk.max_energy / 100;
        mc += boots[i_xy]->dnk.def_front / 3;
        mc += boots[i_xy]->dnk.def_all;
        mc += boots[i_xy]->dnk.ps_ability;
        if (terrain[i_x][i_y] == Terrain::sea) { mc /= (2 * boots[i_xy]->dnk.move_ability_sea + 1); }
        else { mc /= (boots[i_xy]->dnk.move_ability_earth + 1); }

        if (boots[i_xy]->energy > mc) {
            auto dir = boots[i_xy]->GetDirection();
            auto cx = i_x + dir.x;
            auto cy = i_y + dir.y;
            auto c_xy = XY(cx, cy);
            if (IsInBounds(cx, cy) && (boots[c_xy] == nullptr)) {
                boots[i_xy]->stat_steps++;
                boots[i_xy]->TakeEnergy(MoveCost);
                boots[i_xy]->step_spend_move = MoveCost;
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

    auto tbb_f1 = [&](auto &ri_xy) {
        for (int i_xy = ri_xy.begin(); i_xy < ri_xy.end(); i_xy++) {
            if (boots[i_xy]) {
                auto [x_, y_] = XYr(i_xy);
                auto o_tick = boots[i_xy]->tick(terrain[x_][y_]);
                if (o_tick == 1) {
                    organic[x_][y_] = GiveBirthCost + std::max(0, boots[i_xy]->energy);
                    boots[i_xy] = nullptr;
                    return;
                }
            }
        }
    };

    const auto total = FieldCellsWidth * FieldCellsHeight;

    auto f2 = [&](int i_xy) {
//    for (auto i_xy = 0; i_xy < FieldCellsWidth * FieldCellsHeight; i_xy++) {
        auto m_ix = sequence[i_xy];
        if (boots[m_ix]) {
            ++objectsTotal;
            ObjectTick2(m_ix);
        }
    };
    tbb::parallel_for(tbb::blocked_range<int>(0, total), tbb_f1);
    tbb::parallel_for(0, total, [&](auto ix) {
        auto [i_x, i_y] = XYr(ix);
        assert(organic[i_x][i_y] >= 0);
        if (boots[ix])ObjectTick1(ix);
    });
    const auto tbb_step_ = total / 9;
    //    for (auto i = 0; i < total; i++) f2(i);
    for (auto ii = 0; ii < total; ii += tbb_step_) {
        tbb::parallel_for(ii, ii + tbb_step_, f2);
    }

    unsigned long sPS{0}, sK{0}, sM{0}, kills{0}, birth{0}, steps{0},
            seb{0}, sefps{0}, sefk{0}, sefm{0}, sefo{0}, ssfd{0}, ssfa{}, ssma{0},
            sska{0}, sspa{0}, ssme{0}, ssb{0}, ssa{0}, ssr{0}, ssm{0};
    for (auto i = 0; i < total; i++) {
        if (boots[i] == nullptr) continue;
//        sPS += boots[i]->energyFromPS;
//        sK += boots[i]->energyFromKills;
//        sM += boots[i]->energyFromMinerals;
        seb += boots[i]->step_energyBirth;
        sefps += boots[i]->step_energyFromPS;
        sefk += boots[i]->step_energyFromKills;
        sefm += boots[i]->step_energyFromMinerals;
        sefo += boots[i]->step_energyFromOrganic;
        ssfd += boots[i]->step_spend_front_def;
        ssfa += boots[i]->step_spend_front_all;
        ssma += boots[i]->step_spend_mineral_ab;
        sska += boots[i]->step_spend_kill_ab;
        sspa += boots[i]->step_spend_ps_ab;
        ssme += boots[i]->step_spend_max_en;
        ssb += boots[i]->step_spend_birth;
        ssa += boots[i]->step_spend_attack;
        ssr += boots[i]->step_spend_rotate;
        ssm += boots[i]->step_spend_move;


        kills += boots[i]->stat_kills;
        birth += boots[i]->stat_birth;
        steps += boots[i]->stat_steps;
    }
    if (frame_number % 10 != 9) return;
    unsigned long E = sPS + sK + sM + 1;
    std::cout << " T/K/B/S:" << objectsTotal << "/" << kills << "/" << birth << "/" << steps << std::endl;
    std::cout << "        income E=B/PS/K/M/O:" << "E:" << (seb + sefps + sefk + sefm + sefo) << "=" << seb << "/"
              << sefps
              << "/" << sefk << "/" << sefm << "/" << sefo << std::endl;
    std::cout << "        tick  E=D(F/A)+AB(M/K/PS)+En :" << "E:" << (ssfd + ssfa + ssma + sska + sspa + ssme) << "="
              << seb <<
              "(" << ssfd << "/" << ssfa << ")+(" << ssma << "/" << sska << "/ " << sspa << ")+" << ssme << std::endl;
    std::cout << "        step  B A R M :" << "E:" << (ssb + ssa + ssr + ssm) << "=" <<
              " " << ssb << "/" << ssa << "/" << ssr << "/" << ssm << std::endl;
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

    switch (render) {

        case natural: {
            tbb::parallel_for(0, TotalCells, [&](auto m_ix){
                if (boots[m_ix]) boots[m_ix]->draw(image, m_ix, true);
            });
            break;
        }
        case predators: {
            tbb::parallel_for(0, FieldCellsWidth * FieldCellsHeight, [&](auto m_ix){
                if (boots[m_ix]) boots[m_ix]->drawPredators(image, m_ix);
            });
            break;
        }
        case abilities: {
            extra_data.emplace_back("Character");
            tbb::parallel_for(0, FieldCellsWidth * FieldCellsHeight, [&](auto m_ix){
                if (boots[m_ix]) boots[m_ix]->draw(image, m_ix, false);
        });
            break;}
        case energy: {
            tbb::parallel_for(0, FieldCellsWidth * FieldCellsHeight,[&](auto m_ix){
                if (boots[m_ix]) boots[m_ix]->drawEnergy(image, m_ix);
            });
            break;
        }
        case sun_energy: {
            fill_buf_draw(RenderTypes::sun_energy, 2);
            auto max_val = drawAnyGrayScale(image);
            std::ostringstream v;
            v << "max:" << max_val;
            extra_data.emplace_back("SUN");
            extra_data.emplace_back(v.str());
            break;
        }
        case max_energy: {
            fill_buf_draw(max_energy, 2);
            auto max_val = drawAnyGrayScale(image);
            std::ostringstream v;
            v << "max val:" << max_val;
            extra_data.emplace_back("max_energy");
            extra_data.emplace_back(v.str());
            break;
        }
        case defence_attack: {
            auto daf = fill_buf_draw(defence_attack, 0);
            auto daa = fill_buf_draw(def_all, 1);
            auto dka = fill_buf_draw(kill_ability, 2);
            auto mv = std::max(daf, daa);
            mv = std::max(daa, dka);
            drawAnyBGRScale(image, mv, mv, mv);
            std::ostringstream v;
            v << "max front def.: " << daf << " around def.: " << daa << " attack: " << dka;
            extra_data.emplace_back("FD AD KA");
            extra_data.emplace_back(v.str());
            break;
        }
        case minerals_ability: {
            fill_buf_draw(minerals_ability, 2);
            auto max_val = drawAnyGrayScale(image);
            std::ostringstream v;
            v << "max val:" << max_val;
            extra_data.emplace_back("minerals_ability");
            extra_data.emplace_back(v.str());
            break;
        }
        case ps_ability: {
            fill_buf_draw(ps_ability, 2);
            auto max_val = drawAnyGrayScale(image);
            std::ostringstream v;
            v << "max val:" << max_val;
            extra_data.emplace_back("ps_ability");
            extra_data.emplace_back(v.str());
            break;
        }
        case mutability_body: {
            fill_buf_draw(mutability_body, 2);
            auto max_val = drawAnyGrayScale(image);
            std::ostringstream v;
            v << "max val:" << max_val;
            extra_data.emplace_back("mutability_body");
            extra_data.emplace_back(v.str());
            break;
        }
        case mutability_brain: {
            fill_buf_draw(mutability_brain, 2);
            auto max_val = drawAnyGrayScale(image);
            std::ostringstream v;
            v << "max val:" << max_val;
            extra_data.emplace_back("mutability_brain");
            extra_data.emplace_back(v.str());
        }
            break;
        case max_life_time: {
            fill_buf_draw(max_life_time, 2);
            auto max_val = drawAnyGrayScale(image);
            std::ostringstream v;
            v << "max val:" << max_val;
            extra_data.emplace_back("max_life_time");
            extra_data.emplace_back(v.str());
            break;
        }

        case ::garb: {
            memcpy(&tmp_buf2draw, &organic, sizeof(organic));
            auto max_val = drawAnyGrayScale(image);
            std::ostringstream v;
            v << "max val:" << max_val;
            extra_data.emplace_back("organic");
            extra_data.emplace_back(v.str());
            break;
        }

        case lifetime: {
            fill_buf_draw(lifetime, 2);
            auto max_val = drawAnyGrayScale(image);
            std::ostringstream v;
            v << "max val:" << max_val;
            extra_data.emplace_back("lifetime");
            extra_data.emplace_back(v.str());
            break;        }

        case fertility: {
            fill_buf_draw(fertility, 2);
            auto max_val = drawAnyGrayScale(image);
            std::ostringstream v;
            v << "max val:" << max_val;
            extra_data.emplace_back("fertility");
            extra_data.emplace_back(v.str());
            break;
        }

        case dnk_energy_given_on_birth: {
            fill_buf_draw(dnk_energy_given_on_birth, 2);
            auto max_val = drawAnyGrayScale(image);
            std::ostringstream v;
            v << "max val:" << max_val;
            extra_data.emplace_back("dnk_energy_given_on_birth");
            extra_data.emplace_back(v.str());
        }
            break;
        case dnk_move_ability_sea:
            auto mas = fill_buf_draw(dnk_move_ability_sea, 0);
            auto mae = fill_buf_draw(dnk_move_ability_earth, 1);
            auto dka = fill_buf_draw(dnk_move_ability_earth, 2);
            auto mv = std::max(mas, mae);
            drawAnyBGRScale(image, mv, mv, 1000000);
            std::ostringstream v;
            v << "max move on seaf.: " << mas << " earth.: " << mae;
            extra_data.emplace_back("Move sea + earth");
            extra_data.emplace_back(v.str());
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
        if (!IsInBounds(x, y)) continue;
        xy = XY(x, y);
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
    else if (render == RenderTypes::predators) render = RenderTypes::abilities;
    else if (render == RenderTypes::abilities) render = RenderTypes::sun_energy;
    else if (render == RenderTypes::sun_energy) render = RenderTypes::max_energy;
    else if (render == RenderTypes::max_energy) render = RenderTypes::defence_attack;
    else if (render == RenderTypes::defence_attack) render = RenderTypes::minerals_ability;
    else if (render == RenderTypes::minerals_ability) render = RenderTypes::ps_ability;
    else if (render == RenderTypes::ps_ability) render = RenderTypes::mutability_body;
    else if (render == RenderTypes::mutability_body) render = RenderTypes::mutability_brain;
    else if (render == RenderTypes::mutability_brain) render = RenderTypes::max_life_time;
    else if (render == RenderTypes::max_life_time) render = RenderTypes::garb;
    else if (render == RenderTypes::garb) render = RenderTypes::lifetime;
    else if (render == RenderTypes::lifetime) render = RenderTypes::fertility;
    else if (render == RenderTypes::fertility) render = RenderTypes::dnk_energy_given_on_birth;
    else if (render == RenderTypes::dnk_energy_given_on_birth) render = RenderTypes::dnk_move_ability_sea;
    else if (render == RenderTypes::dnk_move_ability_sea) render = RenderTypes::natural;
}

void Field::Annotate(frame_type &image, const std::vector<std::string> &extra, const cv::Scalar &color) const {

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

void Field::updateSunEnergy() {
    const float deg_up = 80.f;
    const float deg_low = 10.f;
    const float deg_one = (deg_up - deg_low) / FieldCellsHeight;

    const float deg_earth_axe = 23.f;
    const float deg_earth_axe_per_day = 2.f * deg_earth_axe / p_half_year;

    const auto gra_to_rad = 3.14159265f / 180;

    auto day_of_year = static_cast<int>(frame_number % p_year);
    auto deg_cur_axe = deg_earth_axe - deg_earth_axe_per_day * static_cast<float>(std::abs(day_of_year - p_half_year));
    tbb::parallel_for(0, FieldCellsHeight, [&](auto y) {
        auto terr_deg = deg_cur_axe + deg_low + deg_one * static_cast<float>(FieldCellsHeight - y);
        float coef;
        if (terr_deg > 90.f) { coef = 0; }
        else {
            coef = std::cos(terr_deg * gra_to_rad);
        }
        int sp = static_cast<int>(PhotosynthesisReward_Summer * coef);
        for (auto x = 0; x < FieldCellsWidth; x++)
            sun_power[x][y] = terrain[x][y] == Terrain::earth ? sp : sp / 2;
    });
}

int Field::GetSunEnergy(int x, int y) const {
    return sun_power[x][y];
}

int Field::drawAnyGrayScale(frame_type &image) {
    int max_val{-10000};
//    int min_val{100000000};
    for (auto & x : tmp_buf2draw)
        for (int & y : x) {
            max_val = std::max(max_val, y);
//            min_val = std::min(min_val, y);
        }
    int c_;
//    for (auto x = 0; x < FieldCellsWidth; x++)
    tbb::parallel_for(0, FieldCellsWidth, [&](auto x) {
        for (auto y = 0; y < FieldCellsHeight; y++) {
            c_ = tmp_buf2draw[x][y] * 255 / (max_val + 1);
            cv::rectangle(image,
                          cv::Point(FieldX + x * FieldCellSize + 1, FieldY + y * FieldCellSize + 1),
                          cv::Point(FieldX + x * FieldCellSize + FieldCellSize - 1,
                                    FieldY + y * FieldCellSize + FieldCellSize - 1),
                          cv::Scalar(c_, c_, c_),
                          -1,
                          cv::LINE_8, 0);
        }
    });
    return max_val;
}

void Field::drawAnyBGRScale(frame_type &image, int mx_0, int mx_1, int mx_2) {

    int c_;
//    for (auto x = 0; x < FieldCellsWidth; x++)
    tbb::parallel_for(0, FieldCellsWidth, [&](auto x) {
        for (auto y = 0; y < FieldCellsHeight; y++) {
            cv::rectangle(image,
                          cv::Point(FieldX + x * FieldCellSize + 1, FieldY + y * FieldCellSize + 1),
                          cv::Point(FieldX + x * FieldCellSize + FieldCellSize - 1,
                                    FieldY + y * FieldCellSize + FieldCellSize - 1),
                          cv::Scalar(
                                  static_cast<float>(tmp_buf0draw[x][y] * 255 / (mx_0 + 1)),
                                  static_cast<float>(tmp_buf1draw[x][y] * 255 / (mx_0 + 1)),
                                  static_cast<float>(tmp_buf2draw[x][y] * 255 / (mx_0 + 1))
                          ),
                          -1,
                          cv::LINE_8, 0);
        }
    });
}

int Field::fill_buf_draw(RenderTypes val, int buf_n) {
    int t_val, o_val, max_val{0};
    for (auto x = 0; x < FieldCellsWidth; x++)
        for (auto y = 0; y < FieldCellsHeight; y++) {
            t_val = XY(x, y);
            if (boots[t_val] == nullptr) { o_val = 0; }
            else {
                switch (val) {
                    case natural:
                        break;
                    case predators:
                        break;
                    case energy:
                        break;
                    case sun_energy:
                        o_val = sun_power[x][y];
                        break;
                    case max_energy:
                        o_val = boots[t_val]->dnk.max_energy;
                        break;
                    case defence_attack:
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
                    case dnk_energy_given_on_birth:
                        o_val = boots[t_val]->dnk.energy_given_on_birth;
                        break;
                    case abilities:
                        break;
                    case dnk_move_ability_sea:
                        o_val = boots[t_val]->dnk.move_ability_sea;
                        break;
                    case dnk_move_ability_earth:
                        o_val = boots[t_val]->dnk.move_ability_earth;
                        break;
                }
            }
            max_val = std::max(max_val, o_val);

            switch (buf_n) {
                case 0:
                    tmp_buf0draw[x][y] = o_val;
                    break;
                case 1:
                    tmp_buf1draw[x][y] = o_val;
                    break;
                case 2:
                    tmp_buf2draw[x][y] = o_val;
                    break;
            }

        }
    return max_val;
}

// TODO prpbably add organic on photosynthes
// TODO prpbably add for hunidity