#include "BotNeuralNet.h"
#include "MyTypes.h"


float BotNeuralNet::Activation(float value) {
    return (value >= 0.5f) ? 1.0f : 0.0f;
}

float BotNeuralNet::PlusMinusActivation(float value) {
    if (value >= 0.5f)
        return 1.0f;
    else if (value <= -0.5f)
        return -1.0f;

    return 0.0f;
}

float BotNeuralNet::RadialBasisActivation(float value) {
    return ((value >= 0.45f) && (value <= 0.55f)) ? 1.0f : 0.0f;
}

void BotNeuralNet::ClearMemory() {
    for (auto i = 0; i < NumNeuronLayers; i++)
        for (auto j = 0; j < NeuronsInLayer; j++) allMemory[i][j] = 0;
}


BotNeuralNet::BotNeuralNet() {
    for (auto yi = 0; yi < NeuronsInLayer; ++yi) {
        allNeurons[NeuronOutputLayerIndex][yi].type = output;
        allNeurons[NeuronInputLayerIndex][yi].type = input;
    }

    ClearMemory();
}

BotNeuralNet::BotNeuralNet(BotNeuralNet *prototype) {
    BotNeuralNet();
    ClearMemory();
    for (auto i = 0; i < NumNeuronLayers; i++)
        for (auto j = 0; j < NeuronsInLayer; j++) allNeurons[i][j] = prototype->allNeurons[i][j];

}

void BotNeuralNet::Clear() {
    for (auto i = 0; i < NumNeuronLayers; i++)
        for (auto j = 0; j < NeuronsInLayer; j++) allValues[i][j] = 0;
}

void BotNeuralNet::Process() {
    float *value;
    float *m;
    Neuron *n;
    int t, v;

    for (auto xi = 0; xi < NumNeuronLayers - 1; ++xi) {
        for (auto yi = 0; yi < NeuronsInLayer; ++yi) {
            value = &allValues[xi][yi];
            m = &allMemory[xi][yi];
            n = &allNeurons[xi][yi];

            //Skip calculation if neuron has no further connections
            if (n->numConnections == 0) {
                continue;
            }

            switch (n->type) {
                case basic:
                    *value = Activation(*value + n->bias);
                    break;
                case randomizer:
                    t = RandomVal(1000);
                    v = (int) ((*value + n->bias) * 1000.0f);
                    *value = (t <= v) ? 1.0f : 0.0f;
                    break;

                case input:
                    *value = *value + n->bias;
                    break;

                case radialbasis:
                    *value = RadialBasisActivation(*value + n->bias);
                    break;

                case memory:
                    if ((*value + n->bias) <= -.5f) {
                        //Wipe memory
                        *m = 0.0f;
                    } else if ((*value + n->bias) >= .5f) {
                        //Write in memory
                        *m += Activation(*value + n->bias);

                        //Maximal value
                        if (*m > 5.0f)
                            *m = 5.0f;
                    }
                    *value = *m;
                    break;
            }

            for (unt i = 0; i < n->numConnections; ++i) {
                allValues[xi + 1][n->allConnections[i].num] += *value * n->allConnections[i].weight;
            }
        }
    }


    //Output layer

    //Rotation
    value = &allValues[NeuronOutputLayerIndex][0];
    n = &allNeurons[NeuronOutputLayerIndex][0];

    *value = PlusMinusActivation(*value + n->bias);

    //All the rest
    for (unt oi = 1; oi < NeuronsInLayer; ++oi) {
        value = &allValues[NeuronOutputLayerIndex][oi];
        n = &allNeurons[NeuronOutputLayerIndex][oi];

        *value = Activation(*value + n->bias);
    }
}


void BotNeuralNet::MutateSlightly() {
    unt randomNeuronIndex = RandomVal(NumNeuronLayers * NeuronsInLayer);
    unt counter = 0;

    for (unt xi = 0; xi < NumNeuronLayers; ++xi) {
        for (unt yi = 0; yi < NeuronsInLayer; ++yi) {

            if (counter++ == randomNeuronIndex) {
                Neuron *n = &allNeurons[xi][yi];

                n->SlightlyChange();
            }

        }
    }
}


void BotNeuralNet::Mutate() {
    unt randomNeuronIndex = RandomVal(NumNeuronLayers * NeuronsInLayer);
    unt counter = 0;

    for (unt xi = 0; xi < NumNeuronLayers; ++xi) {
        for (unt yi = 0; yi < NeuronsInLayer; ++yi) {
            if (counter++ == randomNeuronIndex) {
                Neuron *n = &allNeurons[xi][yi];

                n->SetRandom();
            }
        }
    }
}


void BotNeuralNet::MutateHarsh() {
    for (unt xi = 0; xi < NumNeuronLayers; ++xi) {
        for (unt yi = 0; yi < NeuronsInLayer; ++yi) {

            if (RandomPercent(50))
                continue;

            Neuron *n = &allNeurons[xi][yi];

            n->SetRandom();

        }
    }
}


void BotNeuralNet::Randomize() {
    for (unt y = 0; y < NeuronsInLayer; ++y) {
        allNeurons[NeuronInputLayerIndex][y].SetRandomConnections();
        allNeurons[NeuronInputLayerIndex][y].SetRandomBias();
    }

    for (unt xi = 1; xi < NumNeuronLayers - 1; ++xi) {
        for (unt yi = 0; yi < NeuronsInLayer; ++yi) {
            allNeurons[xi][yi].SetRandom();
        }
    }

    for (unt y = 0; y < NeuronsInLayer; ++y) {
        allNeurons[NeuronOutputLayerIndex][y].SetRandomBias();
    }
}


BrainOutput BotNeuralNet::GetOutput() {
    BrainOutput toRet;

    repeat(BrainOutput::numFields) toRet.fields[i] = (int) allValues[NeuronOutputLayerIndex][i];

    return toRet;
}


void BotNeuralNet::SetDummy() {
    allNeurons[NeuronOutputLayerIndex][2].bias = 1.0f;
    allNeurons[NeuronInputLayerIndex][0].SetTunnel(3);
    allNeurons[1][3].SetTunnel(3);
    allNeurons[2][3].SetTunnel(3);
    allNeurons[3][3].SetTunnel(3);
    allNeurons[4][3].SetTunnel(3);
}