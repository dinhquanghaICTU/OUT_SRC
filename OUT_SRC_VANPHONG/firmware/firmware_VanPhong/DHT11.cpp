#include "DHT11.h"
#include <DHT.h>

DHT dht(DHT11_PIN, DHT_TYPE);



void DHT11_init()
{
    dht.begin();
}

float DHT11_get_temperature(){
    return dht.readTemperature();
}

float DHT11_get_readHumidity(){
    return dht.readHumidity();
}
