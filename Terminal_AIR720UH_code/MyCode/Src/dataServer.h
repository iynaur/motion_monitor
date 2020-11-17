#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void initDataServer(const char* filename);

bool getData(int* dat);

void putData(float a, float b);
void dump();

#ifdef __cplusplus
}
#endif
