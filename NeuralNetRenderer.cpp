#include <opencv2/imgproc.hpp>
#include "NeuralNetRenderer.h"




NeuralNetRenderer::NeuralNetRenderer()
= default;


NeuralNetRenderer::~NeuralNetRenderer()
= default;

bool NeuralNetRenderer::MouseClick(oPoint &p)
{
	MouseXY = p;

	return bg_rect.contains(cv::Point (p.x, p.y));
}

void NeuralNetRenderer::Render(BotNeuralNet &brain, frame_type &image)
{ 
	//Draw background
//	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
//	SDL_RenderFillRect(renderer, &bg_rect);

    cv::rectangle(image, bg_rect, FieldBackgroundColor,
                  -1, cv::LINE_8,0);

	//Draw connections
	Neuron* n;
	NeuronConnection* c;
	oPoint drawTo = { bg_rect.x + 20, bg_rect.y + 20};
    cv::Scalar color_;
	for (int xi = 0; xi < NumNeuronLayers; ++xi)
	{
		for (int yi = 0; yi < NeuronsInLayer; ++yi)
		{
			n = &brain.allNeurons[xi][yi];

			for (int ci = 0; ci < n->numConnections; ++ci)
			{
				c = &n->allConnections[ci];

				if (c->weight > Render_GreyThreshold)
				{
					color_ = Render_PositiveWeightColor;
				}
				else if (c->weight < -Render_GreyThreshold)
				{
					color_ = Render_NegativeWeightColor;
				}
				else
				{
					color_ = Render_NeutralWeightColor;
				}
                cv::line(image,
                         cv::Point{drawTo.x + xi * Render_LayerDistance + (Render_NeuronSize/2), drawTo.y + yi * Render_VerticalDistance + (Render_NeuronSize/2)},
                         cv::Point{drawTo.x + xi * Render_LayerDistance + Render_LayerDistance + (Render_NeuronSize/2), drawTo.y + c->num * Render_VerticalDistance + (Render_NeuronSize/2) },
                         color_,
                         2,
                         cv::LINE_8
                         );

			}
		}
	}

	//Draw neurons	
	cv::Rect rect = {0,0,Render_NeuronSize,Render_NeuronSize };

	for (int xi = 0; xi < NumNeuronLayers; ++xi)
	{
		for (int yi = 0; yi < NeuronsInLayer; ++yi)
		{

			n = &brain.allNeurons[xi][yi];

			//Choose color
			switch (n->type)
			{
			//Yellow
			case input: case output:
				color_ = cv::Scalar{235, 235, 0, };
				break;

			//Grey
			case basic:
                color_ = cv::Scalar{150, 150, 150};
				break;

			//White
			case radialbasis:
                color_ = cv::Scalar{255, 255, 255};
				break;

			//Blue
			case randomizer:
                color_ = cv::Scalar{0, 0, 200};
				break;

			//Purple
			case memory:
                color_ = cv::Scalar{216, 191, 216};
				break;

			}

			//Draw
			rect.x = drawTo.x + xi*Render_LayerDistance;
			rect.y = drawTo.y + yi*Render_VerticalDistance;

            cv::rectangle(image, rect, color_,
                          -1, cv::LINE_8,0);

			//if selected
			if (rect.contains(cv::Point(MouseXY.x, MouseXY.y)))
			{
				//Highlight
                cv::rectangle(image, rect, cv::Scalar{255, 0, 0},
                              -1, cv::LINE_8,0);
				selectedNeuron = n;
			}

		}
	}


}