#include "Bot.h"
#include "MyTypes.h"

void Bot::ChangeMutationMarker() {
    mutationMarkers[nextMarker++] = rand();

    if (nextMarker >= NumberOfMutationMarkers) {
        nextMarker = 0;

#ifdef RandomColorEveryNewMarkersSet
        RandomizeColor();
#endif
    }
}


void Bot::RandomizeMarkers() {
    for (int &mutationMarker: mutationMarkers) {
        mutationMarker = rand();
    }
    nextMarker = 0;
}

void Bot::RandomizeColor() {
    auto v = rand();
    int i = v % 3;
    int pm = v % 2;
    if (pm == 0) {
        color[i] = std::min(color[i] + 10, 255.);
    } else    color[i] = std::max(color[i] - 10, 0.);
}


void Bot::TotalMutation() {
    //RandomizeMarkers();
    repeat(3) ChangeMutationMarker();

    brain.MutateHarsh();

    RandomizeColor();
}


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
    ChangeMutationMarker();

    //Mutate brain
    for (int i = 0; i < (1 + RandomVal(MutateNeuronsMaximum + 1)); ++i)
        brain.Mutate();

    for (int s = 0; s < (1 + RandomVal(MutateNeuronsSlightlyMaximum + 1)); ++s)
        brain.MutateSlightly();

    //Change color
#ifdef ChangeColorSlightly
    ChangeColor(20);
#endif

    /*if (RandomPercentX10(RandomColorChancePercentX100))
    {
        RandomizeColor();
    }*/

}


void Bot::drawOutlineAndHead(frame_type &image, cv::Point &p1, cv::Point &p2) {
    //Draw outline
    auto head = cv::Scalar(0, 0, 0);
    int hx, hy;
    if (direction == 3 || direction == 4 || direction == 5) hy = p2.y;
    else if (direction == 0 || direction == 1 || direction == 7) hy = p1.y;
    else if (direction == 2 || direction == 6 ) hy = (p2.y + p1.y) / 2;

    if (direction == 1 || direction == 2 || direction == 3) hx = p1.x;
    else if (direction == 5 || direction == 6 || direction == 7) hx = p2.x;
    else if (direction == 0 || direction == 4 ) hx = (p2.x + p1.x) / 2;

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


void Bot::Radiation() {
    Mutate();
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
        brain.allValues[0][0] = ((energy * 1.0f) / (MaxPossibleEnergyForABot * 1.0f));

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

int Bot::selectionStep = 0;

bool Bot::SelectionWatcher() {
    if (y >= FieldCellsHeight - OceanHeight) {
        if (selection_lastX < x) {
            ++selection_numRightSteps;

        }

        selection_lastX = x;

        if (selection_numTicks++ == 2) {
            if (selection_numRightSteps == 0) {
                if (RandomPercent(selectionStep))
                    return true;
            }

            selection_numRightSteps = 0;
            selection_numTicks = 0;
        }
    }

    return false;
}


int Bot::tick() {

    energy -= EveryTickEnergyPenalty;

    if (SelectionWatcher())
        energy = 0;

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
    auto energySumm = energyFromKills + energyFromPS;
    cv::Scalar color_{0, 0, 0};
    if (energyFromMinerals > 0)
        color_ = {255, 255, 255};
    else if (energySumm < 20)
        color_ = {180, 180, 180};
    else
        color_ = {0,
                  (double) energyFromPS * 255.f / energySumm,
                  (double) energyFromKills * 255.f / energySumm};

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

    if (energy > MaxPossibleEnergyForABot) {
        energy = MaxPossibleEnergyForABot;
    }

    if (src == PS) {
        energyFromPS += num;
    } else if (src == kills) {
        energyFromKills += num;
    } else if (src == mineral) {
        energyFromMinerals += num;
    }
}


int Bot::GetEnergy() const {
    return energy;
}

int Bot::GetEnergyFromPS() const {
    return energyFromPS;
}

int Bot::GetEnergyFromKills() const {
    return energyFromKills;
}


int *Bot::GetMarkers() {
    return mutationMarkers;
}


cv::Scalar Bot::GetColor() {
    return color;
}


bool Bot::TakeEnergy(int val) {
    energy -= val;

    return energy <= 0;
}


oPoint Bot::GetDirection() {
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
            }

            toRet[4] += n->numConnections;

            if (n->numConnections == 0)
                toRet[5]++;
        }
    }

    return {toRet[0], toRet[1], toRet[2], toRet[3], toRet[4], toRet[5], NumNeuronLayers * NeuronsInLayer};
}


int Bot::FindKinship(t_object &stranger) {
    int numMarkers = 0;

    for (unt i = 0; i < NumberOfMutationMarkers; ++i) {
        if (mutationMarkers[i] == stranger->mutationMarkers[i])
            ++numMarkers;
    }

#ifdef OneMarkerDifferenceCantBeTold
    if (numMarkers == NumberOfMutationMarkers - 1)
        numMarkers = NumberOfMutationMarkers;
#endif

    return numMarkers;
}

void Bot::Repaint(cv::Scalar &newColor) {
    color = newColor;
}


Bot::Bot(int X, int Y, int Energy, std::shared_ptr<Bot> prototype, bool mutate) : x(X), y(Y), brain(&prototype->brain) {

    energy = Energy;
    stunned = StunAfterBirth;
    fertilityDelay = FertilityDelay;
    energyFromPS = 0;
    energyFromKills = 0;

    //Copy parent's markers and color
    memcpy(mutationMarkers, prototype->mutationMarkers, sizeof(mutationMarkers));
    nextMarker = prototype->nextMarker;

//	memcpy(color, prototype->color, sizeof(color));
    color = prototype->color;

    //Now mutate
#ifndef ForbidMutations
    if (mutate) {
#ifdef UseTotalMutation
        if (RandomPercentX10(TotalMutationChancePercentX10))
            TotalMutation();
        else
#endif
        {
            Mutate();
        }
    }
#endif

    selection_lastX = X;
}


Bot::Bot(int X, int Y, int Energy) : x(X), y(Y) {

    //Ignore
    //brain.allNeurons[1][1].type = random;
    //brain.allNeurons[1][1].bias = 0.5f;
    //brain.allNeurons[1][1].AddConnection(2, -1.0f);

    //brain.allNeurons[2][2].bias = 1.0f;

    //brain.allNeurons[4][1].bias = 1.0f;

    RandomizeMarkers();

    energy = Energy;
    stunned = StunAfterBirth;
    fertilityDelay = FertilityDelay;
    energyFromPS = 0;
    energyFromKills = 0;

    //Randomize bot brain
    brain.Randomize();

    //Set brain to dummy brain
    //brain.SetDummy();

    //Random color
    RandomizeColor();

    //for (int m = 0; m < NeuronsInLayer*NumNeuronLayers; ++m)
    //Mutate();

    selection_lastX = X;
}


void Bot::GiveInitialEnergyAndMarkers() {
    RandomizeMarkers();

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

BotNeuralNet *Bot::GetBrain() {
    return &brain;
}


