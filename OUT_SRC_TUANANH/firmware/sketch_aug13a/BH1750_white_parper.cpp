#include "BH1750_white_parper.h"
#include <Wire.h>
#include <BH1750.h>

float lux = 0.0;

BH1750 lightMeter;

void BH1750_white_parper_init(){
     Wire.begin(21, 22);
    lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);
}

float BH1750_white_parper_getdata(){
    return lightMeter.readLightLevel();
}