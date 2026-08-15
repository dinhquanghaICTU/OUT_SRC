#ifndef __UV_H__
#define __UV_H__

#define SIG_PIN 34

#ifdef __cplusplus
extern "C"
{
#endif

    void init_uv(void);
    // int getdata_uv();

    float readUvVoltage();
    float voltageToUvIndex(float voltage);
    float fakeUvVoltage();
    float fakeUvIndex();

#ifdef __cplusplus
}
#endif

#endif //__UV_H__