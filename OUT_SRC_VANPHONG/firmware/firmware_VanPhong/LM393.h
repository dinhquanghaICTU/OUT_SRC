#ifndef __LM393_H__
#define __LM393_H__

#define LM393_PIN 26

#ifdef __cplusplus
extern "C"
{
#endif

    void LM393_init();
    float get_data_LM393();

#ifdef __cplusplus
}
#endif
#endif //__LM393_H__