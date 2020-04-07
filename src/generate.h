#ifndef __GENERATE_H__
#define __GENERATE_H__

#include <stdint.h>
#include <stdlib.h>

struct wave {
	float* samples;
	size_t size;
    size_t capacity;
};

struct wave* newWave(size_t const samples);
void deleteWave(struct wave* const wave);
void add_sample(struct wave* const wave, float const sample);

#endif
