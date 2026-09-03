#ifndef __LM393_H__
#define __LM393_H__

#define LM393_PIN 34

#ifdef __cplusplus
extern "C" {
#endif

void LM393_init();
float get_data_LM393();

float get_soil_moisture_percent();
#ifdef __cplusplus
}
#endif
#endif //__LM393_H__