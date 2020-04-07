#include <cmath>

#include <exception>
#include <vector>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "./sdl2.h"

namespace {

auto constexpr DEVICE_FREQUENCY = 48000;
auto constexpr DEVICE_FORMAT = AUDIO_F32;
auto constexpr DEVICE_CHANNELS = 2;
auto constexpr DEVICE_SAMPLES = 8192;

}

auto main(void) -> int
{
    spdlog::set_level(spdlog::level::debug);

    nk::SDL2_uptr sdl2;
    nk::SDL2AudioDevice_uptr audio_device;

    try {
        sdl2.reset(new nk::SDL2);
        audio_device.reset(
                new nk::SDL2AudioDevice(
                        sdl2,
                        DEVICE_FREQUENCY,
                        DEVICE_FORMAT,
                        DEVICE_CHANNELS,
                        DEVICE_SAMPLES));
    } catch (std::exception const& error) {
        spdlog::error("program failed to initialize: {}", error.what());
        return EXIT_FAILURE;
    }

    auto constexpr duration = 4000;
    std::vector< float > samples(duration / 1000 * DEVICE_CHANNELS * DEVICE_FREQUENCY);
    for (auto i = 0; i < samples.size() / 2; i += 1) {
        float const sample = sin(2 * 440 * (float)M_PI / DEVICE_FREQUENCY * i);
        samples[i * 2 + 0] = sample;
        samples[i * 2 + 1] = sample;
    }

    audio_device->pause();
    audio_device->queue(samples);
    audio_device->unpause();
    SDL_Delay(duration);
}
