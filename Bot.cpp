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
void Bot::SlightlyMutate() {
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
        brain.allValues[0][0] = (float) (1000 * energy / MaxPossibleEnergyForABot) / 1000.f;

        //Sight
        brain.allValues[0][1] = input.vision;

        //Is relative
        brain.allValues[0][2] = input.isRelative;

        //Rotation
        brain.allValues[0][3] = (direction * 1.0f) / 7.0f;

        //Height
        brain.allValues[0][4] = (y * 1.0f) / (FieldCellsHeight * 1.0f);
    }

    //Compute
    brain.Process();

    BrainOutput toRet = brain.GetOutput();

    //Cannot multipy if not ready
    if (fertilityDelay) {
        --fertilityDelay;
        toRet.divide = 0;
    } else if (toRet.divide) {
        fertilityDelay = FertilityDelay;
    }

    //if(toRet.attack == 1)
    //return toRet;

    return toRet;
}

int Bot::tick() {
    lifetime++;
    energy -= EveryTickEnergyPenalty;
    // penalty from DNK
    const int pen_factor = 1;
    energy -= dnk.protetction_front * pen_factor;
    energy -= dnk.protetction_others * pen_factor;
    energy -= dnk.minerals_ability * pen_factor;
    energy -= dnk.atack_ability * pen_factor;
    energy -= dnk.ps_ability * pen_factor;
    energy -= std::max(0, dnk.max_energy - MaxPossibleEnergyForABot) / 20;
    if (((energy) <= 0) || (lifetime >= MaxBotLifetime))
        return 1;

    return 0;
}


void Bot::draw(frame_type &image) {

    auto pt1 = cv::Point(FieldX + x * FieldCellSize, FieldY + y * FieldCellSize);
    auto pt2 = cv::Point(FieldX + x * FieldCellSize + FieldCellSize, FieldY + y * FieldCellSize + FieldCellSize);
    cv::rectangle(image,
                  pt1,
                  pt2,
                  color,
                  -1,
                  cv::LINE_8, 0);


    //Draw outlines
    drawOutlineAndHead(image, pt1, pt2);
}


void Bot::drawEnergy(frame_type &image) {
    auto c_ = energy * 255 / MaxPossibleEnergyForABot;
    cv::rectangle(image,
                  cv::Point(FieldX + x * FieldCellSize, FieldY + y * FieldCellSize),
                  cv::Point(FieldX + x * FieldCellSize + FieldCellSize, FieldY + y * FieldCellSize + FieldCellSize),
                  cv::Scalar(c_, c_, c_),
                  -1,
                  cv::LINE_8, 0);

    //Draw outlines
    auto pt1 = cv::Point(FieldX + x * FieldCellSize, FieldY + y * FieldCellSize);
    auto pt2 = cv::Point(FieldX + x * FieldCellSize + FieldCellSize, FieldY + y * FieldCellSize + FieldCellSize);
    drawOutlineAndHead(image, pt1, pt2);
}


void Bot::drawPredators(frame_type &image) {
    //Draw body
    auto energySum = energyFromKills + energyFromPS + energyFromMinerals;
    cv::Scalar color_{0, 0, 0};
//    if (energyFromMinerals > 0)
//        color_ = {255, 255, 255};
//    else if (energySum < 20)
//        color_ = {180, 180, 180};
//    else
    color_ = {(double) energyFromMinerals * 255.f / energySum,
              (double) energyFromPS * 255.f / energySum,
              (double) energyFromKills * 255.f / energySum};

    cv::rectangle(image,
                  cv::Point(FieldX + x * FieldCellSize, FieldY + y * FieldCellSize),
                  cv::Point(FieldX + x * FieldCellSize + FieldCellSize, FieldY + y * FieldCellSize + FieldCellSize),
                  color_,
                  -1,
                  cv::LINE_8, 0);

//Draw outlines
    auto pt1 = cv::Point(FieldX + x * FieldCellSize, FieldY + y * FieldCellSize);
    auto pt2 = cv::Point(FieldX + x * FieldCellSize + FieldCellSize, FieldY + y * FieldCellSize + FieldCellSize);
    drawOutlineAndHead(image, pt1, pt2);
}


void Bot::Rotate(int dir) {
    if (dir > 0)
        ++direction;
    else
        direction += 7;

    direction = direction % 8;
}


void Bot::GiveEnergy(int num, EnergySource src) {
    energy += num;

    if (energy > dnk.max_energy) {
        energy = dnk.max_energy;
    }

    if (src == PS) {
        energyFromPS += num;
    } else if (src == kills) {
        energyFromKills += num;
    } else if (src == mineral) {
        energyFromMinerals += num;
    }
}

bool Bot::TakeEnergy(int val) {
    energy -= val;

    return energy <= 0;
}


oPoint Bot::GetDirection() const {
    return Rotations[direction];
}

/*Get neuron summary(info)
Format: (all integers)
-simple neurons
-radial basis neurons
-random neurons
-memory neurons (if any)
-total connections
-dead end neurons
-total neurons
*/
summary_return Bot::GetNeuronSummary() {
    int toRet[6] = {0, 0, 0, 0, 0, 0};
    Neuron *n;

    for (unt xi = 1; xi < NumNeuronLayers; ++xi) {
        for (unt yi = 0; yi < NeuronsInLayer; ++yi) {
            n = &brain.allNeurons[xi][yi];

            switch (n->type) {
                case basic:
                    toRet[0]++;
                    break;
                case radialbasis:
                    toRet[1]++;
                    break;
                case randomizer:
                    toRet[2]++;
                    break;
                case memory:
                    toRet[3]++;
                    break;
                case input:
                    break;
                case output:
                    break;
            }

            toRet[4] += n->numConnections;

            if (n->numConnections == 0)
                toRet[5]++;
        }
    }

    return {toRet[0], toRet[1], toRet[2], toRet[3], toRet[4], toRet[5], NumNeuronLayers * NeuronsInLayer};
}


//int Bot::FindKinship(t_object &stranger) {
//    int numMarkers = 0;
//
//    for (unt i = 0; i < NumberOfMutationMarkers; ++i) {
//        if (mutationMarkers[i] == stranger->mutationMarkers[i])
//            ++numMarkers;
//    }
//
//#ifdef OneMarkerDifferenceCantBeTold
//if (numMarkers == NumberOfMutationMarkers - 1)
//numMarkers = NumberOfMutationMarkers;
//#endif
//
//return
//numMarkers;
//}



Bot::Bot(int X, int Y, int Energy, t_object &prototype) : x(X), y(Y), brain(&prototype->brain),
                                                          weight(0) {

    energy = Energy;
    stunned = StunAfterBirth;
    fertilityDelay = FertilityDelay;
    energyFromPS = 0;
    energyFromKills = 0;
    dnk = prototype->dnk;
    color = prototype->color;
    if (rand() % 2 == 0) return;
//    std::ostringstream mm;
//    mm << " " << dnk.descript();
    for (auto i = 0; i <= dnk.mutability_body; i++) dnk.mutate(1);
    for (int s = 0; s <= dnk.mutability_brain; s++) brain.MutateSlightly();
    auto diff = FindKinship(prototype);
//    mm << " |> " << dnk.descript() << "  K:" <<  diff;
//    std::cout << mm.str() << std::endl;
    ChangeColor((int) 255 * diff);
}


Bot::Bot(int X, int Y, int Energy) : x(X), y(Y), weight(0) {
    energy = Energy;
    stunned = StunAfterBirth;
    fertilityDelay = FertilityDelay;
    energyFromPS = 0;
    energyFromKills = 0;
    //Randomize bot brain
    brain.Randomize();
    //Set brain to dummy brain.SetDummy();
    //Random color
    RandomizeColor();
    //for (int m = 0; m < NeuronsInLayer*NumNeuronLayers; ++m)
//    Mutate();
    dnk.mutate(1); // TODO set sun dependency
    dnk.mutate(1);
    dnk.mutate(1);
    dnk.mutate(1);
    dnk.mutate(1);
    dnk.mutate(1);
    dnk.mutate(1);
    Mutate();
    RandomizeColor();
}


void Bot::GiveInitialEnergyAndMarkers() {
//    RandomizeMarkers();

    energy = MaxPossibleEnergyForABot;
    stunned = StunAfterBirth;
    fertilityDelay = FertilityDelay;
    energyFromPS = 0;
    energyFromKills = 0;
    direction = 0;
    RandomizeColor(); // Temporary
}


Neuron *Bot::GetNeuralNet() {
    return (Neuron *) brain.allNeurons;
}

//BotNeuralNet *Bot::GetBrain() {
//    return &brain;
//}

int Bot::coord() const {
    return XY(x, y);
}

float Bot::FindKinship(t_object &stranger) const {
    return dnk.distance(stranger->dnk);
}


DNK &DNK::operator=(const DNK &dnk2) {
    max_energy = dnk2.max_energy;
    protetction_front = dnk2.protetction_front;
    protetction_others = dnk2.protetction_others;
    minerals_ability = dnk2.minerals_ability;
    atack_ability = dnk2.atack_ability;
    ps_ability = dnk2.ps_ability;
}

void DNK::mutate(int d) {
    int ix = rand() % 9;
    int diff = rand() % (d + 1);
    bool sign = rand() % 2;
    diff = (sign ? -1 : 1) * diff;
    switch (ix) {
        case 0: {
            max_energy = std::max(0, max_energy + diff * 10);
            return;
        }
        case 1: {
            protetction_front = std::max(0, protetction_front + diff);
            return;
        }
        case 02: {
            protetction_others = std::max(0, protetction_others + diff);
            return;
        }
        case 3: {
            atack_ability = std::max(0, atack_ability + diff);
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
    }


}

std::string DNK::descript() const {
    std::ostringstream out;
    out << "ME:" << max_energy << " PF:" << protetction_front
        << " PO:" << protetction_others << " A:" << atack_ability
        << " MA" << minerals_ability << " PS:" << ps_ability
        << " M:" << mutability_body << "/" << mutability_brain;
    return out.str();
}
