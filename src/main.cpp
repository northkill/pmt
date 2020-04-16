#include <cmath>
#include <cstdio>

#include <array>
#include <exception>
#include <functional>
#include <iomanip>
#include <sstream>
#include <vector>

#include <fftw3.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <SDL2/SDL_opengl.h>

#include "./sdl2.h"

namespace {

auto constexpr DEVICE_FREQUENCY = 48000;
auto constexpr DEVICE_FORMAT = AUDIO_F32;
auto constexpr DEVICE_CHANNELS = 2;
auto constexpr DEVICE_SAMPLES = 8192;

}

auto constexpr squ(float x) -> float {
    if (x > 2.0 * M_PI)
        x = atan2(sin(x), cos(x));
    if (x < 0)
        return 0;
    return 1;
}

template < float (&generator)(float) >
auto gen_sample(float const freq, float const sample_rate, float const x) -> float
{
    return generator(2 * freq * M_PI / sample_rate * x);
}

std::function const gen_sine_sample(gen_sample<sin>);
std::function const gen_square_sample(gen_sample<squ>);

auto emit_end_timer_event(Uint32 const interval, void* const param) -> Uint32
{
    SDL_UserEvent userevent = { 0, };
    userevent.type = reinterpret_cast< uintptr_t >(param);

    SDL_Event event = { 0, };
    event.type = SDL_USEREVENT;
    event.user = userevent;

    if (SDL_PushEvent(&event) != 1) {
        spdlog::warn("failed to push event, retrying in 1 second: {}", SDL_GetError());
        return 1000;
    }

    spdlog::debug("timer callback emitted end timer event");
    return 0;
}

auto main(void) -> int
{
    spdlog::set_level(spdlog::level::debug);

    nk::SDL2_uptr sdl2;
    nk::SDL2AudioDevice_uptr audio_device;
    nk::SDL2Window_uptr window;
    nk::SDL2GLContext_uptr gl;

    try {
        sdl2.reset(new nk::SDL2);
        audio_device.reset(
                new nk::SDL2AudioDevice(
                        sdl2,
                        DEVICE_FREQUENCY,
                        DEVICE_FORMAT,
                        DEVICE_CHANNELS,
                        DEVICE_SAMPLES));
        window.reset(
                new nk::SDL2Window(
                        "oh no",
                        SDL_WINDOWPOS_UNDEFINED,
                        SDL_WINDOWPOS_UNDEFINED,
                        1280,
                        768));
        gl.reset(new nk::SDL2GLContext(window));
    } catch (std::exception const& error) {
        spdlog::error("program failed to initialize: {}", error.what());
        return EXIT_FAILURE;
    }

    auto constexpr duration = 1;
    auto constexpr sample_number = duration * DEVICE_FREQUENCY * DEVICE_CHANNELS;

    std::array< std::vector< float >, 3 > samples;
    for (unsigned int i = 0; i < samples.size(); i += 1) {
        samples[i].reserve(sample_number);
        for (unsigned int j = 0; j < sample_number / 2; j += 1) {
            samples[i].emplace_back(gen_sine_sample(440.0f * (i + 1), DEVICE_FREQUENCY, j));
            samples[i].emplace_back(gen_sine_sample(440.0f * (i + 1), DEVICE_FREQUENCY, j));
        }
    }

    window->show();
    audio_device->unpause();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    bool running = true;
    while (running) {
        SDL_Event event = { 0, };
        if (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;

            case SDL_KEYDOWN:
                if (event.key.repeat != 0)
                    break;
                switch (event.key.keysym.scancode) {
                case SDL_SCANCODE_Z:
                    spdlog::debug("z");
                    audio_device->queue(samples[0]);
                    break;

                case SDL_SCANCODE_X:
                    spdlog::debug("x");
                    audio_device->queue(samples[1]);
                    break;

                case SDL_SCANCODE_C:
                    spdlog::debug("c");
                    audio_device->queue(samples[2]);
                    break;

                default:
                    break;
                }
                break;

            case SDL_KEYUP:
                switch (event.key.keysym.scancode) {
                case SDL_SCANCODE_Q:
                    spdlog::debug("SDL keyup q event dispatched");
                    running = false;
                    break;

                case SDL_SCANCODE_Z:
                    break;

                case SDL_SCANCODE_X:
                    break;

                case SDL_SCANCODE_C:
                    break;

                default:
                    break;
                }
                break;
            }
        }

        glClear(GL_COLOR_BUFFER_BIT);
        SDL_GL_SwapWindow(*window);
    }

    audio_device->pause();
    window->hide();
}
