#ifndef HCSR_04_H
#define HCSR_04_H

#ifdef __cplusplus
extern "C" {
#endif

void hcsr04_init(void);
float hcsr04_read_distance_cm(void);
float hcsr04_get_last_distance_cm(void);

#ifdef __cplusplus
}
#endif

#endif // HCSR_04_H
