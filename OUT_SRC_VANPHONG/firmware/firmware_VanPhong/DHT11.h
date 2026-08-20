#ifndef __DHT11_H__
#define __DHT11_H__

#define DHT11_PIN 25
#define DHT_TYPE DHT11

#ifdef __cplusplus
extern "C"
{
#endif

float DHT11_get_temperature();
float DHT11_get_readHumidity();
void DHT11_init();
#ifdef __cplusplus
}
#endif
#endif //__DHT11_H__