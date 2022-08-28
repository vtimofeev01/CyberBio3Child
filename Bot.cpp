#include "Bot.h"
#include "MyTypes.h"

void Bot::RandomizeColor() {
    auto v = rand();
    int i = v % 3;
    int pm = v % 2;
    if (pm == 0) {
        color[i] = std::min(color[i] + 10, 255.);
    } else color[i] = std::max(color[i] - 10, 0.);
    ChangeColor(0, dnk.minerals_ability, dnk.ps_ability, dnk.kill_ability);
}

void Bot::ChangeColor(const int str, int pref_b, int pref_g, int pref_r) {
    int i = rand() % 3;
    auto p_sum = pref_b + pref_g + pref_r + 1;
    ab_color = cv::Scalar(static_cast<double>(255 * pref_b / p_sum),
                          static_cast<double>(255 * pref_g / p_sum),
                          static_cast<double>(255 * pref_r / p_sum));

    int changeColor = rand() % ((2 * str) + 1) - str;
    color[i] += changeColor;

    if (color[i] < 0)
        color[i] = 0;
    else if (color[i] > 255)
        color[i] = 255;
}


//Experimental
[[maybe_unused]] void Bot::SlightlyMutate() {
    brain.MutateSlightly();

//#ifdef ChangeColorSlightly
//    ChangeColor(10, 0, 0, 0);
//#endif
}


void Bot::Mutate() {
    //Change next mutation marker
//    ChangeMutationMarker();

    //Mutate brain
    for (int i = 0; i < (1 + RandomVal(MutateNeuronsMaximum + 1)); ++i)
        brain.Mutate();

    for (int s = 0; s < (1 + RandomVal(MutateNeuronsSlightlyMaximum + 1)); ++s)
        brain.MutateSlightly();

    //Change color
    ChangeColor(20, 0, 0, 0);

}


void Bot::drawOutlineAndHead(frame_type &image, cv::Point &p1, cv::Point &p2) const {
    //Draw outline
    auto head = cv::Scalar(0, 0, 0);
    int hx, hy;
    if (direction == 3 || direction == 4 || direction == 5) hy = p2.y;
    else if (direction == 0 || direction == 1 || direction == 7) hy = p1.y;
    else if (direction == 2 || direction == 6) hy = (p2.y + p1.y) / 2;

    if (direction == 1 || direction == 2 || direction == 3) hx = p1.x;
    else if (direction == 5 || direction == 6 || direction == 7) hx = p2.x;
    else if (direction == 0 || direction == 4) hx = (p2.x + p1.x) / 2;

    cv::circle(image, cv::Point(hx, hy), 1, head, -1);


}


BrainOutput Bot::think(BrainInput input) {
    //Stunned means the creature can not act
    if (stunned) {
        --stunned;
        return {0, 0, 0, 0};
    }

    //Clear all neuron values
    brain.Clear();

    //Input data
    {
        //Energy
//        brain.allValues[0][0] = static_cast<float>(1000 * energy / dnk.max_energy) / 1000.f;
        brain.allValues[0][0] = input.energy;

        //Sight
        brain.allValues[0][1] = input.vision;

        //Is relative
        brain.allValues[0][2] = input.isRelative;

        //Rotation
        brain.allValues[0][3] = input.rotation / 7.0f;

        //Height
        brain.allValues[0][4] = input.goal_energy;
        brain.allValues[0][4] = input.local_terrain;
        brain.allValues[0][4] = input.direct_terrain;
    }

    //Compute
    brain.Process();

    BrainOutput toRet = brain.GetOutput();

    //Cannot multipy if not ready
    if (fertilityDelay) {
        --fertilityDelay;
        toRet.divide = 0;
    } else if (toRet.divide) {
        fertilityDelay = dnk.fertilityDelay;
    }
    return toRet;
}

int Bot::tick(Terrain terr) {
    assert(energy >= 0);
    assert(energy < 50000 );
    step_energyBirth       = 0;
    step_energyFromPS      = 0;
    step_energyFromKills    = 0;
    step_energyFromMinerals = 0;
    step_energyFromOrganic = 0;
    step_spend_front_def   = 0;
    step_spend_front_all   = 0;
    step_spend_mineral_ab  = 0;
    step_spend_kill_ab     = 0;
    step_spend_ps_ab       = 0;
    step_spend_max_en      = 0;
    step_spend_birth       = 0;
    step_spend_attack      = 0;
    step_spend_rotate      = 0;
    step_spend_move        = 0;


    unsigned long loss{0};
    const int pen_factor = 1;
    lifetime++;
    step_spend_front_def = dnk.def_front * pen_factor / 2;
    step_spend_front_all = dnk.def_all * pen_factor;
    step_spend_mineral_ab = dnk.minerals_ability * pen_factor;
    step_spend_kill_ab = dnk.kill_ability * pen_factor;
    step_spend_ps_ab = dnk.ps_ability * pen_factor;
    step_spend_max_en = dnk.max_energy / 100;
    step_move_ability_earth = dnk.move_ability_earth;
    step_move_ability_sea = dnk.move_ability_sea;

    if (terr == Terrain::sea) {
        step_spend_front_def /= 2;
        step_spend_front_all /= 2;
        step_spend_mineral_ab /= 2;
        step_spend_kill_ab /= 2;
        step_spend_ps_ab /= 2;
        step_spend_max_en /= 2;
        step_move_ability_earth /=2;
        step_move_ability_sea /= 2;
    }
    loss = step_spend_front_def + step_spend_front_all + step_spend_mineral_ab + step_spend_kill_ab +
            step_spend_ps_ab + step_spend_max_en + step_move_ability_earth + step_move_ability_sea;
    energy -= static_cast<int>(loss);
    if (((energy) <= 0) || (lifetime >= MaxBotLifetime))
        return 1;
    assert(energy >= 0);
    assert(energy < 50000 );
    return 0;
}


void Bot::draw(frame_type &image, int _xy, bool use_own_color) {
    auto [xx, yy] = XYr(_xy);
    auto pt1 = cv::Point(FieldX + xx * FieldCellSize, FieldY + yy * FieldCellSize);
    auto pt2 = cv::Point(FieldX + xx * FieldCellSize + FieldCellSize, FieldY + yy * FieldCellSize + FieldCellSize);
    cv::rectangle(image,
                  pt1,
                  pt2,
                  use_own_color ? color : ab_color,
                  -1,
                  cv::LINE_8, 0);


    //Draw outlines
    drawOutlineAndHead(image, pt1, pt2);
}


void Bot::drawEnergy(frame_type &image, int _xy) const {
    auto c_ = energy * 255 / (dnk.max_energy + 1);
    auto [xx, yy] = XYr(_xy);
    auto pt1 = cv::Point(FieldX + xx * FieldCellSize, FieldY + yy * FieldCellSize);
    auto pt2 = cv::Point(FieldX + xx * FieldCellSize + FieldCellSize, FieldY + yy * FieldCellSize + FieldCellSize);

    cv::rectangle(image, pt1, pt2,
                  cv::Scalar(c_, c_, c_),
                  -1,
                  cv::LINE_8, 0);

    //Draw outlines
//    drawOutlineAndHead(image, pt1, pt2);
}


void Bot::drawPredators(frame_type &image, int _xy) {
    //Draw body
    auto [xx, yy] = XYr(_xy);
    auto energySum = energyFromKills + energyFromPS + energyFromMinerals;
    auto pt1 = cv::Point(FieldX + xx * FieldCellSize, FieldY + yy * FieldCellSize);
    auto pt2 = cv::Point(FieldX + xx * FieldCellSize + FieldCellSize, FieldY + yy * FieldCellSize + FieldCellSize);

    cv::Scalar color_ = {(double) energyFromMinerals * 255.f / energySum,
                         (double) energyFromPS * 255.f / energySum,
                         (double) energyFromKills * 255.f / energySum};
    cv::rectangle(image, pt1, pt2,
                  color_,
                  -1,
                  cv::LINE_8, 0);
    //Draw outlines
    drawOutlineAndHead(image, pt1, pt2);
}


void Bot::Rotate(int dir) {
    if (dir > 0)
        ++direction;
    else
        direction += 7;

    direction = direction % 8;
}


int Bot::GiveEnergy(int in_energy, EnergySource src) {
    auto n_num{in_energy};
    energy += in_energy;
    int out{0};

    if (energy > dnk.max_energy) {
        out = energy - dnk.max_energy;
        energy = dnk.max_energy;
        n_num -= out;
    }

    if (src == PS) {
        energyFromPS += n_num;
        step_energyFromPS = n_num;
    } else if (src == kills) {
        energyFromKills += n_num;
        step_energyFromKills = n_num;
    } else if (src == mineral) {
        energyFromMinerals += n_num;
        step_energyFromMinerals = n_num;
    } else if (src == ES_garbage) {
        energyFromOrganic += n_num;
        step_energyFromOrganic = n_num;
    } else if (src == birth) {
        step_energyBirth = in_energy;
        energy = in_energy;
    }
    assert(energy >= 0);
    assert(energy < 50000 );
    return out;
}

void Bot::TakeEnergy(int val) {
    energy -= val;
    if (energy <= 0) {
        energy = 0;
    }
}


oPoint Bot::GetDirection() const {
    return Rotations[direction];
}


Bot::Bot(int Energy, t_object &prototype) : energy(0),brain(&prototype->brain),
                                            weight(0) {
    assert(energy >= 0);
    assert(energy < 50000 );
    energy = GiveEnergy(Energy, EnergySource::birth);
    stunned = StunAfterBirth;
    fertilityDelay = prototype->dnk.fertilityDelay;
    energyFromPS = 0;
    energyFromKills = 0;
    dnk = prototype->dnk;
    color = prototype->color;
    direction = rand() % 8;
    if (rand() % 5 == 0) return;
    for (int s = 0; s <= (rand() % (dnk.mutability_brain + 1)); s++) brain.MutateSlightly();
    if (rand() % 40 == 0) return;
    for (auto i = 0; i <= dnk.mutability_body; i++) dnk.mutate(1);
    Mutate();
    auto diff = FindKinship(prototype);
    ChangeColor((int) (255.f * diff), dnk.minerals_ability, dnk.ps_ability, dnk.kill_ability);
    assert(energy >= 0);
    assert(energy < 50000 );
}


Bot::Bot(int Energy) : weight(0), energy(0) {
    assert(energy >= 0);
    assert(energy < 50000 );
    energy = GiveEnergy(Energy, EnergySource::birth);
    stunned = StunAfterBirth;
    fertilityDelay = FertilityDelay;
    energyFromPS = 0;
    energyFromKills = 0;
    direction = rand() % 8;
    //Randomize bot brain
    brain.Randomize();
    RandomizeColor();
    for (auto i = 0; i < 10; i++) {
        dnk.mutate(i);
        dnk.mutate(i);
        dnk.mutate(i);
        Mutate();
    }
    RandomizeColor();
}

float Bot::FindKinship(t_object &stranger) const {
    return dnk.distance(stranger->dnk);
}


DNK &DNK::operator=(const DNK &dnk2) {
    max_energy = dnk2.max_energy;
    def_front = dnk2.def_front;
    def_all = dnk2.def_all;
    minerals_ability = dnk2.minerals_ability;
    kill_ability = dnk2.kill_ability;
    ps_ability = dnk2.ps_ability;
    mutability_body = dnk2.mutability_body;
    mutability_brain = dnk2.mutability_brain;
    max_life_time = dnk2.max_life_time;
    fertilityDelay = dnk2.fertilityDelay;
    energy_given_on_birth = dnk2.energy_given_on_birth;
    move_ability_earth = dnk2.move_ability_earth;
    move_ability_sea = dnk2.move_ability_sea;
}

void DNK::mutate(int d) {
    int ix = rand() % 13;
    int diff = rand() % (d + 1);
    bool sign = rand() % 2;
    diff = (sign ? -1 : 1) * diff;
    switch (ix) {
        case 0: {
            max_energy = std::max(0, max_energy + diff * 100);
            return;
        }
        case 1: {
            def_front = std::max(0, def_front + diff);
            return;
        }
        case 02: {
            def_all = std::max(0, def_all + diff);
            return;
        }
        case 3: {
            kill_ability = std::max(0, kill_ability + diff);
            return;
        }
        case 4: {
            minerals_ability = std::max(0, minerals_ability + diff);
            return;
        }
        case 5: {
            ps_ability = std::max(0, ps_ability + diff);
            return;
        }
        case 6: {
            mutability_body = std::max(0, mutability_body + diff);
            return;
        }
        case 7: {
            mutability_brain = std::max(0, mutability_brain + diff);
            return;
        }
        case 8: {
            max_life_time = std::max(0, max_life_time + diff * 10);
            return;
        }
        case 9: {
            fertilityDelay = std::max(0, fertilityDelay + diff);
            return;
        }
        case 10: {
            energy_given_on_birth = std::max(0, energy_given_on_birth + diff);
            return;
        }
        case 11: {
            move_ability_earth = std::max(0, move_ability_earth + diff);
            return;
        }
        case 12: {
            move_ability_sea = std::max(0, move_ability_sea + diff);
            return;
        }
    }


}

[[maybe_unused]] std::string DNK::descript() const {
    std::ostringstream out;
    out << "ME:" << max_energy << " PF:" << def_front
        << " PO:" << def_all << " A:" << kill_ability
        << " MA" << minerals_ability << " PS:" << ps_ability
        << " M:" << mutability_body << "/" << mutability_brain;
    return out.str();
}
