#ifndef __MICROPHONE_H__
#define __MICROPHONE_H__


#define MICROPHONE_PIN 34
#define SAMPLE_WINDOW_MS 50

#ifdef __cplusplus
extern "C"
{
    
#endif

void init_microphone();
float get_microphone();

#ifdef __cplusplus
}
#endif

#endif //__MICROPHONE_H__
