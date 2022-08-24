#pragma clang diagnostic push
#pragma ide diagnostic ignored "hicpp-multiway-paths-covered"
#include "Bot.h"
#include "MyTypes.h"



//void Bot::RandomizeMarkers() {
//    for (int &mutationMarker: mutationMarkers) {
//        mutationMarker = rand();
//    }
//    nextMarker = 0;
//}

void Bot::RandomizeColor() {
    auto v = rand();
    int i = v % 3;
    int pm = v % 2;
    if (pm == 0) {
        color[i] = std::min(color[i] + 10, 255.);
    } else color[i] = std::max(color[i] - 10, 0.);
}


//void Bot::TotalMutation() {
//    //RandomizeMarkers();
//    repeat(3) ChangeMutationMarker();
//
//    brain.MutateHarsh();
//
//    RandomizeColor();
//}


void Bot::ChangeColor(const int str) {
    int i = rand() % 3;

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

#ifdef ChangeColorSlightly
    ChangeColor(10);
#endif
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
    ChangeColor(20);

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

#ifdef DrawBotOutline
//	SDL_SetRenderDrawColor(renderer, BotOutlineColor);
//	SDL_RenderDrawRect(renderer, &rect);
#endif

    //Draw direction marker
#ifdef DrawBotHead
//	SDL_RenderDrawLine(renderer, FieldX + x * FieldCellSize + FieldCellSizeHalf, FieldY + y * FieldCellSize + FieldCellSizeHalf, FieldX + x * FieldCellSize + FieldCellSizeHalf + Rotations[direction].x * FieldCellSizeHalf, FieldY + y * FieldCellSize + FieldCellSizeHalf + Rotations[direction].y * FieldCellSizeHalf);
#endif
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
        brain.allValues[0][0] = static_cast<float>(1000 * energy / MaxPossibleEnergyForABot) / 1000.f;

        //Sight
        brain.allValues[0][1] = input.vision;

        //Is relative
        brain.allValues[0][2] = input.isRelative;

        //Rotation
        brain.allValues[0][3] = static_cast<float>(direction) / 7.0f;

        //Height
        brain.allValues[0][4] = 0; // TODO find what to use
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
    int loss{0};
    const int pen_factor = 1;
    lifetime++;
    loss += EveryTickEnergyPenalty;
    loss += dnk.def_front * pen_factor / 2;
    loss += dnk.def_all * pen_factor;
    loss += dnk.minerals_ability * pen_factor;
    loss += dnk.kill_ability * pen_factor;
    loss += dnk.ps_ability * pen_factor;
    loss += dnk.max_energy / 100;
    if (terr == Terrain::sea) loss /=2;
    energy -=loss;
    if (((energy) <= 0) || (lifetime >= MaxBotLifetime))
        return 1;

    return 0;
}


void Bot::draw(frame_type &image, int _xy) {
    auto [xx, yy] = XYr(_xy);
    auto pt1 = cv::Point(FieldX + xx * FieldCellSize, FieldY + yy * FieldCellSize);
    auto pt2 = cv::Point(FieldX + xx * FieldCellSize + FieldCellSize, FieldY + yy * FieldCellSize + FieldCellSize);
    cv::rectangle(image,
                  pt1,
                  pt2,
                  color,
                  -1,
                  cv::LINE_8, 0);


    //Draw outlines
    drawOutlineAndHead(image, pt1, pt2);
}


void Bot::drawEnergy(frame_type &image, int _xy) {
    auto c_ = energy * 255 / dnk.max_energy;
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


int Bot::GiveEnergy(int num, EnergySource src) {
    energy += num;
    int out{0};

    if (energy > dnk.max_energy) {
        out = dnk.max_energy - energy;
        energy = dnk.max_energy;

    }

    if (src == PS) {
        energyFromPS += num;
    } else if (src == kills) {
        energyFromKills += num;
    } else if (src == mineral) {
        energyFromMinerals += num;
    } else if (src == ES_garbage) {
        energyFromOrganic += num;
    }
    return out;
}

bool Bot::TakeEnergy(int val) {
    energy -= val;

    return energy <= 0;
}


oPoint Bot::GetDirection() const {
    return Rotations[direction];
}


Bot::Bot(int Energy, t_object &prototype) : brain(&prototype->brain),
                                            weight(0) {

    energy = Energy;
    stunned = StunAfterBirth;
    fertilityDelay = prototype->dnk.fertilityDelay;
    energyFromPS = 0;
    energyFromKills = 0;
    dnk = prototype->dnk;
    color = prototype->color;
    direction = rand() % 8;
    if (rand() % 5 == 0) return;
    for (int s = 0; s <=( rand()%(dnk.mutability_brain + 1)); s++) brain.MutateSlightly();
    if (rand() % 40 == 0) return;
//    std::ostringstream mm;
//    mm << " " << dnk.descript();
    for (auto i = 0; i <= dnk.mutability_body; i++) dnk.mutate(1);
    Mutate();
//    for (int s = 0; s <= dnk.mutability_brain; s++) brain.MutateSlightly();
    auto diff = FindKinship(prototype);
//    mm << " |> " << dnk.descript() << "  K:" <<  diff;
//    std::cout << mm.str() << std::endl;
    ChangeColor((int) (255.f * diff));
}


Bot::Bot(int Energy) : weight(0) {
    energy = Energy;
    stunned = StunAfterBirth;
    fertilityDelay = FertilityDelay;
    energyFromPS = 0;
    energyFromKills = 0;
    direction = rand() % 8;
    //Randomize bot brain
    brain.Randomize();
    //Set brain to dummy brain.SetDummy();
    //Random color
    RandomizeColor();
    //for (int m = 0; m < NeuronsInLayer*NumNeuronLayers; ++m)
//    Mutate();
    for (auto i = 0; i < 10; i++) {
        dnk.mutate(i);
        dnk.mutate(i);
        dnk.mutate(i);
        Mutate();
    }
    RandomizeColor();
}

//BotNeuralNet *Bot::GetBrain() {
//    return &brain;
//}

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
}

void DNK::mutate(int d) {
    int ix = rand() % 11;
    int diff = rand() % (d + 1);
    bool sign = rand() % 2;
    diff = (sign ? -1 : 1) * diff;
    switch (ix) {
        case 0: {
            max_energy = std::max(0, max_energy + diff * 10);
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

#pragma clang diagnostic pop