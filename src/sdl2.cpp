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

auto constexpr SDL2WindowInitErrorMessage = "failed to initialize SDL2 window";
class SDL2WindowInitError : public std::exception {
public:
    char const* what(void) const noexcept override {
        return SDL2WindowInitErrorMessage;
    }
};

auto constexpr SDL2OpenGLAttributeInitErrorMessage = "failed to set SDL2 OpenGL attribute";
class SDL2OpenGLAttributeInitError : public std::exception {
public:
    char const* what(void) const noexcept override {
        return SDL2OpenGLAttributeInitErrorMessage;
    }
};

auto constexpr SDL2GLContextInitErrorMessage = "failed to create SDL2 OpenGL context";
class SDL2GLContextInitError : public std::exception {
public:
    char const* what(void) const noexcept override {
        return SDL2GLContextInitErrorMessage;
    }
};

auto constexpr SDL2UserEventInitErrorMessage = "failed to register user event";
class SDL2UserEventInitError : public std::exception {
public:
    char const* what(void) const noexcept override {
        return SDL2UserEventInitErrorMessage;
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

auto set_sdl2_gl_attribute(SDL_GLattr const attr, int const value) {
    if (SDL_GL_SetAttribute(attr, value) != 0) {
        spdlog::error("`SDL_GL_SetAttribute` failed: {}", SDL_GetError());
        throw SDL2OpenGLAttributeInitError();
    }
}

}

namespace nk {

unsigned int SDL2::s_count_init = 0;

SDL2::SDL2(void)
{
    spdlog::trace("SDL2::SDL2");
    spdlog::trace("SDL2::s_count_init: {}", s_count_init);

    if (s_count_init == 0) {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
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
    spdlog::debug("closed audio device");
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

SDL2Window::SDL2Window(
        const char* title,
        int const x,
        int const y,
        int const w,
        int const h)
    : m_window(nullptr)
{
    spdlog::trace("SDL2Window::SDL2Window");
    m_window = SDL_CreateWindow(title, x, y, w, h, SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL);
    if (m_window == nullptr) {
        spdlog::error("`SDL_CreateWindow` failed: {}", SDL_GetError());
        throw SDL2WindowInitError();
    }

    set_sdl2_gl_attribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    set_sdl2_gl_attribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    spdlog::trace("SDL2Window::SDL2Window finished");
}

SDL2Window::~SDL2Window(void)
{
    spdlog::trace("SDL2Window::~SDL2Window");
    if (m_window != nullptr) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr; 
        spdlog::debug("window destroyed");
    }
    spdlog::trace("SDL2Window::~SDL2Window finished");
}

auto SDL2Window::show(void) noexcept -> void
{
    SDL_ShowWindow(m_window);
}

auto SDL2Window::hide(void) noexcept -> void
{
    SDL_HideWindow(m_window);
}

SDL2GLContext::SDL2GLContext(SDL2Window_uptr& window)
    : m_context(SDL_GL_CreateContext(*window))
{
    spdlog::trace("SDL2GLContext::SDL2GLContext");
    if (m_context == nullptr) {
        spdlog::error("`SDL_GL_CreateContext` failed: {}", SDL_GetError());
        throw SDL2GLContextInitError();
    }
    spdlog::trace("SDL2GLContext::SDL2GLContext finished");
}

SDL2GLContext::~SDL2GLContext(void)
{
    spdlog::trace("SDL2GLContext::~SDL2GLContext");
    if (m_context != nullptr) {
        SDL_GL_DeleteContext(m_context);
        m_context = nullptr;
        spdlog::debug("SDL OpenGL context deleted");
    }
    spdlog::trace("SDL2GLContext::~SDL2GLContext finished");
}

SDL2UserEvent::SDL2UserEvent(SDL2_uptr const& sdl2)
    : m_id(SDL_RegisterEvents(1))
{
    spdlog::trace("SDL2UserEvent::SDL2UserEvent");
    if (m_id == static_cast< Uint32 >(-1)) {
        spdlog::error("`SDL_RegisterEvents` failed: {}", SDL_GetError());
        throw SDL2UserEventInitError();
    }
    spdlog::trace("SDL2UserEvent::SDL2UserEvent finished");
}

}
