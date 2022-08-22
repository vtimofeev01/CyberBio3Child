#pragma once


#include "Field.h"
#include <iostream>
#include "NeuralNetRenderer.h"






//Window

bool simulate = true;	//Set false to pause

//Population chart
float populationChartData[ChartNumValues];
int chart_numValues = 0;
int chart_currentPosition = 0;


void LoadFilenames();
void CreateNewFile();


