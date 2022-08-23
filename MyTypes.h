#pragma once
#include "Systems.h"


typedef unsigned int unt;


//Get random number (0 to max-1)
#define RandomVal(max) (rand() % (max))

//Roll for probability (chance in percents)
#define RandomPercent(val) ((rand()%1000)>=(1000 - ((val) * 10)))

//Simple cycle
#define repeat(times) for(int i=0;i<(times);++i)
