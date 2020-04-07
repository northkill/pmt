#include <exception>

#include <spdlog/spdlog.h>

#include "./sdl2.h"

namespace {

auto constexpr SDL2InitErrorMessage = "failed to initialize SDL2";
class SDL2InitError : public std::exception {
public:
    char const* what(void) const noexcept override {
        return SDL2InitErrorMessage;
    }
};

auto constexpr SDL2AudioDeviceInitErrorMessage = "failed to initialize SDL2 audio device";
class SDL2AudioDeviceInitError : public std::exception {
public:
    char const* what(void) const noexcept override {
        return SDL2AudioDeviceInitErrorMessage;
    }
};

auto init_audio_device(
        nk::SDL2_uptr const& sdl2,
        int const freq,
        SDL_AudioFormat const format,
        Uint8 const channels,
        Uint16 const samples)
    -> SDL_AudioDeviceID
{
    spdlog::trace("init_audio_device");
    SDL_AudioSpec const want = {
        freq,
        format,
        channels,
        0, // silence
        samples,
    };
    auto const device_id = SDL_OpenAudioDevice(nullptr, 0, &want, nullptr, 0);
    if (device_id == 0) {
        spdlog::error("`SDL_AudioSpec` failed: {}", SDL_GetError());
        throw SDL2AudioDeviceInitError();
    }
    spdlog::debug("opened audio device: {}", device_id);
    spdlog::trace("init_audio_device finished");
    return device_id;
}

}

namespace nk {

unsigned int SDL2::s_count_init = 0;

SDL2::SDL2(void)
{
    spdlog::trace("SDL2::SDL2");
    spdlog::trace("SDL2::s_count_init: {}", s_count_init);

    if (s_count_init == 0) {
        if (SDL_Init(SDL_INIT_AUDIO) != 0) {
            spdlog::error("`SDL_Init` failed: {}", SDL_GetError());
            throw SDL2InitError();
        }
        spdlog::debug("SDL2 initialized");
    } else {
        spdlog::debug("SDL2 skipped initializing");
    }

    s_count_init += 1;

    spdlog::trace("SDL2::SDL2 finished");
}

SDL2::~SDL2(void)
{
    spdlog::trace("SDL2::~SDL2");
    spdlog::trace("SDL2::s_count_init: {}", s_count_init);

    if (s_count_init > 0) {
        s_count_init -= 1;
        if (s_count_init == 0) {
            SDL_Quit();
            spdlog::debug("SDL2 quitted");
        } else {
            spdlog::debug("SDL2 skipped quitting");
        }
    } else {
        spdlog::warn("SDL2::Quit called but SDL2 has not been initialized");
    }

    spdlog::trace("SDL2::~SDL2 finished");
}

SDL2AudioDevice::SDL2AudioDevice(
        SDL2_uptr const& sdl2,
        int const freq,
        SDL_AudioFormat const format,
        Uint8 const channels,
        Uint16 const samples)
    : m_device_id(init_audio_device(sdl2, freq, format, channels, samples))
{
    spdlog::trace("SDL2AudioDevice::SDL2AudioDevice");
    spdlog::trace("SDL2AudioDevice::SDL2AudioDevice finished");
}

SDL2AudioDevice::~SDL2AudioDevice(void)
{
    spdlog::trace("SDL2AudioDevice::~SDL2AudioDevice");
    SDL_CloseAudioDevice(m_device_id);
    spdlog::debug("closed audio device: {}", m_device_id);
    spdlog::trace("SDL2AudioDevice::~SDL2AudioDevice finished");
}

auto SDL2AudioDevice::pause(void) noexcept -> void
{
    SDL_PauseAudioDevice(m_device_id, 1);
}

auto SDL2AudioDevice::unpause(void) noexcept -> void
{
    SDL_PauseAudioDevice(m_device_id, 0);
}

auto SDL2AudioDevice::check_queue_result(int const result) noexcept -> void
{
    if (result != 0)
        spdlog::warn("`SDL_QueueAudio` failed: {}", SDL_GetError());
}

}
