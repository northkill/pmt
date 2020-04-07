#include <string.h>

#include "./generate.h"

struct wave* newWave(size_t const samples) {
	struct wave* const wave = malloc(sizeof(struct wave));
	memset(wave, 0, sizeof(*wave));

	wave->samples = malloc(sizeof(*wave->samples) * samples);
	wave->capacity = samples;

	return wave;
}

void deleteWave(struct wave* const wave) {
	free(wave->samples);
	free(wave);
}

void add_sample(struct wave* const wave, float const sample) {
	if (wave->size >= wave->capacity)
		return;

	size_t index = wave->size;
	wave->samples[index] = sample;

	wave->size += 1;
}
