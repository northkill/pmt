#include <cmath>

#include <exception>
#include <functional>
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

auto squ(float x) -> float {
    if (x > 2.0 * M_PI)
        x = atan2(sin(x), cos(x));
    if (x < 0)
        return 0;
    return 1;
}

auto gen_sine_sample(float const freq, float const sample_rate, float const x) -> float
{
    return sin(2 * freq * M_PI / sample_rate * x);
}

auto between(float const x, float const a, float const b) -> bool {
    return x > a and x < b;
}

template <unsigned int F1, unsigned int F2, unsigned int T = 100>
auto get_formant_coef(float const freq) -> float {
    if (between(freq, F1 - T, F1 + T))
        return 1.0;
    else if (between(freq, F2 - T, F2 + T))
        return 0.9;
    return 0.3;
}

auto const get_formant_coef_i = std::function (get_formant_coef< 240, 2400 >);
auto const get_formant_coef_ae = std::function (get_formant_coef< 585, 1710 >);
auto const get_formant_coef_e = std::function (get_formant_coef< 390, 2300 >);
auto const get_formant_coef_3 = std::function (get_formant_coef< 610, 1900 >);
auto const get_formant_coef_u = std::function (get_formant_coef< 250, 595 >);

auto gen_harmonics_sample(
        float const freq,
        float const x,
        std::function<float(float const)> const get_formant_coef,
        unsigned int const i = 1)
    -> float
{
    float const target_freq = freq * i;
    if (target_freq > 4200)
        return 0;
    return 2.0 * get_formant_coef(target_freq) * gen_sine_sample(target_freq, DEVICE_FREQUENCY, x)
        + gen_harmonics_sample(freq, x, get_formant_coef, i + 1);
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

    std::vector< float > samples1(duration / 1000 * DEVICE_CHANNELS * DEVICE_FREQUENCY);
    for (auto i = 0; i < samples1.size() / 2; i += 1) {
        float const sample = 0.01 * gen_harmonics_sample(120, i, get_formant_coef_i);
        samples1[i * 2 + 0] = sample;
        samples1[i * 2 + 1] = sample;
    }

    std::vector< float > samples2(duration / 1000 * DEVICE_CHANNELS * DEVICE_FREQUENCY);
    for (auto i = 0; i < samples2.size() / 2; i += 1) {
        float const sample = 0.01 * gen_harmonics_sample(120, i, get_formant_coef_ae);
        samples2[i * 2 + 0] = sample;
        samples2[i * 2 + 1] = sample;
    }

    std::vector< float > samples3(duration / 1000 * DEVICE_CHANNELS * DEVICE_FREQUENCY);
    for (auto i = 0; i < samples3.size() / 2; i += 1) {
        float const sample = 0.01 * gen_harmonics_sample(120, i, get_formant_coef_u);
        samples3[i * 2 + 0] = sample;
        samples3[i * 2 + 1] = sample;
    }

    std::vector< float > samples4(duration / 1000 * DEVICE_CHANNELS * DEVICE_FREQUENCY);
    for (auto i = 0; i < samples4.size() / 2; i += 1) {
        float const sample = 0.01 * gen_harmonics_sample(120, i, get_formant_coef_3);
        samples4[i * 2 + 0] = sample;
        samples4[i * 2 + 1] = sample;
    }

    audio_device->pause();
    audio_device->queue(samples1);
    audio_device->queue(samples2);
    audio_device->queue(samples3);
    audio_device->queue(samples4);
    audio_device->unpause();
    SDL_Delay(duration * 4);
}
