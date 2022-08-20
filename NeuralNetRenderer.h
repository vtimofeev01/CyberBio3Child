#pragma once

#include "BotNeuralNet.h"
#include "Systems.h"

#define FONT1 cv::FONT_HERSHEY_DUPLEX


struct NeuralNetRenderer
{
	NeuralNetRenderer();
	~NeuralNetRenderer();

	void Render(BotNeuralNet &brain, frame_type &image);

	bool MouseClick(oPoint &p);

	Neuron* selectedNeuron = nullptr;

private:
	oPoint MouseXY;

	cv::Scalar White = { 255, 255, 255 };
	cv::Rect TextRect;

	cv::Rect bg_rect = { 100 - 20, 670 - 20, Render_LayerDistance * (NumNeuronLayers - 1) + Render_NeuronSize + 40, Render_VerticalDistance * (NeuronsInLayer - 1) + Render_NeuronSize + 40 };

};