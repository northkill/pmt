#define __STDC_WANT_LIB_EXT1__ 1

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <alsa/asoundlib.h>

#include <SDL2/SDL.h>

#define TEST_WAV_PATH "/home/asdf/project/pmt/test.wav"

typedef struct {
	SDL_AudioDeviceID audio_device;
	struct {
		Uint8 *buffer;
		SDL_AudioSpec spec;
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

static bool init_audio_device(PMTContext *const context, const char* const audio_device_name) {
	SDL_AudioSpec want;
	SDL_memset(&want, 0, sizeof(want));
	want.freq = 48000;
	want.format = AUDIO_F32;
	want.channels = 2;
	want.samples = 4096;
	want.callback = NULL;

	context->audio_device = SDL_OpenAudioDevice(audio_device_name, 0, &want, NULL, SDL_AUDIO_ALLOW_ANY_CHANGE);
	if (g_context.audio_device == 0) {
		fprintf(stderr, "failed to open audio device: %s\n", SDL_GetError());
		return false;
	}
	fprintf(stdout, "opened audio device\n");

	return true;
}

static bool init_test_wav(PMTContext *const context, char const *const path) {
	Uint32 wav_length = 0;
	bool result = SDL_LoadWAV(
			path,
			&context->test_wav.spec,
			&context->test_wav.buffer,
			&wav_length) != NULL;
	if (!result) {
		fprintf(stderr, "failed to open test wav file: %s\n", SDL_GetError());
		return false;
	}
	fprintf(stdout, "loaded test wav file\n");
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

	list_audio_device();
	for (int i = 0; i < SDL_GetNumAudioDrivers(); i += 1)
		fprintf(stdout, "audio driver %d: %s\n", i, SDL_GetAudioDriver(i));
	char const* const audio_device_name = select_audio_device();

	if (!init_audio_device(&g_context, audio_device_name))
		return false;

	if (!init_test_wav(&g_context, TEST_WAV_PATH))
		return false;

	fprintf(stdout, "program initialized\n");
	return true;
}

static void quit() {
	if (g_context.test_wav.buffer != NULL) {
		SDL_FreeWAV(g_context.test_wav.buffer);
		g_context.test_wav.buffer = NULL;
		g_context.test_wav.len = 0;
		SDL_memset(&g_context.test_wav.spec, 0, sizeof(g_context.test_wav.spec));
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

int main(int argc, char const *argv[]) {
	if (!init()) {
		fprintf(stderr, "failed to initialize\n");
		quit();
		return EXIT_FAILURE;
	}
	atexit(quit);

	list_audio_device();

	int const queue_result = SDL_QueueAudio(
			g_context.audio_device,
			g_context.test_wav.buffer,
			g_context.test_wav.len);
	if (queue_result != 0) {
		fprintf(stderr, "failed to queue audio: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	SDL_PauseAudioDevice(g_context.audio_device, 0);
	SDL_Delay(5000);
	SDL_PauseAudioDevice(g_context.audio_device, 1);

	return EXIT_SUCCESS;
}
