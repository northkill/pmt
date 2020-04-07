#define __STDC_WANT_LIB_EXT1__ 1

#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#include <alsa/asoundlib.h>

#include <SDL2/SDL.h>

#include "./generate.h"

#define TEST_WAV_PATH "/home/asdf/project/pmt/test.wav"
#define AUDIO_DEVICE_FREQ 48000
#define AUDIO_DEVICE_FORMAT AUDIO_F32
#define AUDIO_DEVICE_CHANNELS 2

typedef struct {
	SDL_AudioDeviceID audio_device;
	struct {
		bool converted;
		Uint8 *buffer;
		Uint32 len;
	} test_wav;
} PMTContext;

static PMTContext g_context = { 0, };

static void list_audio_device() {
	fprintf(stdout, "current audio driver: %s\n", SDL_GetCurrentAudioDriver());

	int const count = SDL_GetNumAudioDevices(0);
	if (count == -1) {
		fprintf(stdout, "couldn't get audio device number\n");
		return;
	}

	for (int i = 0; i < count; i += 1) {
		char const* const name = SDL_GetAudioDeviceName(i, 0);
		fprintf(stdout, "%02d: %s\n", i, name);
	}
}

static int scan_int() {
	char buffer[3] = { 0, };
	for (int i = 0; i < 2; i += 1) {
		char const input = fgetc(stdin);
		if (input == EOF)
			return -1;
		buffer[i] = input;
	}

	int input = -1;
	sscanf(buffer, "%2d", &input);
	return input;
}

static char const* select_audio_device() {
	fprintf(stdout, "select audio device: ");
	int const index = scan_int();
	return SDL_GetAudioDeviceName(index, 0);
}

static bool init_audio_device(PMTContext *const context) {
	SDL_AudioSpec want;
	SDL_memset(&want, 0, sizeof(want));
	want.freq = AUDIO_DEVICE_FREQ;
	want.format = AUDIO_DEVICE_FORMAT;
	want.channels = AUDIO_DEVICE_CHANNELS;
	want.samples = 4096;
	want.callback = NULL;

	context->audio_device = SDL_OpenAudioDevice(NULL, 0, &want, NULL, 0);
	if (g_context.audio_device == 0) {
		fprintf(stderr, "failed to open audio device: %s\n", SDL_GetError());
		return false;
	}
	fprintf(stdout, "opened audio device\n");

	return true;
}

static bool init_test_wav(PMTContext *const context, char const *const path) {
	SDL_AudioSpec spec;
	SDL_memset(&spec, 0, sizeof(spec));
	bool loaded = SDL_LoadWAV(
			path,
			&spec,
			&context->test_wav.buffer,
			&context->test_wav.len) != NULL;
	if (!loaded) {
		fprintf(stderr, "failed to open test wav file: %s\n", SDL_GetError());
		return false;
	}
	fprintf(stdout, "loaded test wav file\n");

	SDL_AudioCVT cvt;
	int wavStatus = SDL_BuildAudioCVT(
			&cvt,
			spec.format,
			spec.channels,
			spec.freq,
			AUDIO_DEVICE_FORMAT,
			AUDIO_DEVICE_CHANNELS,
			AUDIO_DEVICE_FREQ);
	if (wavStatus == 0) {
		g_context.test_wav.converted = false;
		fprintf(stdout, "wav conversion is not needed\n");
		return true;
	} else if (wavStatus == -1) {
		fprintf(stderr, "failed to build wav conversion data: %s\n", SDL_GetError());
		return false;
	}
	fprintf(stdout, "wav conversion is needed\n");

	cvt.len = g_context.test_wav.len;
	size_t const buffer_size = cvt.len * cvt.len_mult;
	cvt.buf = SDL_malloc(buffer_size);
	SDL_memcpy(cvt.buf, g_context.test_wav.buffer, g_context.test_wav.len);
	if (SDL_ConvertAudio(&cvt) != 0) {
		fprintf(stderr, "failed to convert wav: %s\n", SDL_GetError());
		return false;
	}

	SDL_FreeWAV(g_context.test_wav.buffer);
	g_context.test_wav.buffer = cvt.buf;
	g_context.test_wav.len = buffer_size;
	g_context.test_wav.converted = true;
	fprintf(stdout, "converted wav\n");
	return true;
}

static bool init_sdl() {
	if (SDL_Init(SDL_INIT_AUDIO) != 0) {
		fprintf(stderr, "failed to initialize SDL: %s\n", SDL_GetError());
		return false;
	}
	fprintf(stdout, "SDL initialized\n");
	return true;
}

static bool init() {
	if (!init_sdl())
		return false;

	if (!init_audio_device(&g_context))
		return false;

	if (!init_test_wav(&g_context, TEST_WAV_PATH))
		return false;

	fprintf(stdout, "program initialized\n");
	return true;
}

static void quit() {
	if (g_context.test_wav.buffer != NULL) {
		if (g_context.test_wav.converted)
			SDL_free(g_context.test_wav.buffer);
		else
			SDL_FreeWAV(g_context.test_wav.buffer);
		g_context.test_wav.buffer = NULL;
		g_context.test_wav.len = 0;
		fprintf(stdout, "freed test wav\n");
	}

	if (g_context.audio_device != 0) {
		SDL_CloseAudioDevice(g_context.audio_device);
		g_context.audio_device = 0;
		fprintf(stdout, "closed audio device\n");
	}

	if (SDL_WasInit(0)) {
		SDL_Quit();
		fprintf(stdout, "quitted SDL\n");
	}

	fprintf(stdout, "program quitted\n");
}

static float squ(double x) {
	if (x > 2 * M_PI)
		x = atan2(sin(x), cos(x));
	if (x < 0)
		return 0;
	return 1;
}

int main(int argc, char const *argv[]) {
	if (!init()) {
		fprintf(stderr, "failed to initialize\n");
		quit();
		return EXIT_FAILURE;
	}
	atexit(quit);

	SDL_PauseAudioDevice(g_context.audio_device, 1);

	size_t const sampleRate = AUDIO_DEVICE_FREQ;
	size_t const duration = 4;
	size_t const channels = 2;
	double const freq = 440.0;
	struct wave* wave = newWave(channels * sampleRate * duration);

	for (int i = 0; i < sampleRate * duration; i += 1) {
		float const sample = squ((double)2 * M_PI * 440 / sampleRate * i);
		add_sample(wave, sample);
		add_sample(wave, sample);
	}

	bool const queued = SDL_QueueAudio(
			g_context.audio_device,
			wave->samples,
			wave->size * sizeof(*wave->samples)) == 0;
	if (!queued) {
		fprintf(stderr, "failed to queue audio: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	fprintf(stdout, "start\n");
	SDL_PauseAudioDevice(g_context.audio_device, 0);
	SDL_Delay((duration + 1) * 1000);
	fprintf(stdout, "stop\n");

	deleteWave(wave);
	wave = NULL;

	return EXIT_SUCCESS;
}
