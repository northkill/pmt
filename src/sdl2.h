#ifndef __NK_SDL2_H__
#define __NK_SDL2_H__

extern "C" {
#include <SDL2/SDL.h>
}

#include <memory>
#include <iterator>

namespace nk {

class SDL2 {
    public:
        SDL2(void);
        ~SDL2(void);

    private:
        static unsigned int s_count_init;
};

using SDL2_uptr = std::unique_ptr< SDL2 >;

class SDL2AudioDevice {
public:
    SDL2AudioDevice(
            SDL2_uptr const& sdl2,
            int const freq,
            SDL_AudioFormat const format,
            Uint8 const channels,
            Uint16 const samples);
    ~SDL2AudioDevice(void);
    SDL2AudioDevice(SDL2AudioDevice const& other) = delete;
    SDL2AudioDevice& operator = (SDL2AudioDevice const& other) = delete;

    auto pause(void) noexcept -> void;
    auto unpause(void) noexcept -> void;

    template < typename Buffer >
    auto queue(Buffer const& buffer) noexcept -> void
    {
        auto const result = SDL_QueueAudio(
                m_device_id,
                buffer.data(),
                buffer.size() * sizeof(typename Buffer::value_type));
        check_queue_result(result);
    }

private:
    auto check_queue_result(int const result) noexcept -> void;
    SDL_AudioDeviceID const m_device_id;

};

using SDL2AudioDevice_uptr = std::unique_ptr< SDL2AudioDevice >;

class SDL2Window {
public:
    SDL2Window(
            const char* title,
            int const x,
            int const y,
            int const w,
            int const h);
    ~SDL2Window(void);
    SDL2Window(SDL2Window const& other) = delete;
    SDL2Window& operator = (SDL2Window const& other) = delete;

    constexpr operator SDL_Window* (void) noexcept { return m_window; }
    constexpr operator SDL_Window* (void) const noexcept { return m_window; } 

    auto show(void) noexcept -> void;
    auto hide(void) noexcept -> void;

private:
    SDL_Window* m_window;
};

using SDL2Window_uptr = std::unique_ptr< SDL2Window >;

class SDL2GLContext {
public:
    SDL2GLContext(SDL2Window_uptr& window);
    ~SDL2GLContext(void);
    SDL2GLContext(SDL2GLContext const& other) = delete;
    SDL2GLContext& operator = (SDL2GLContext const& other) = delete;

private:
    SDL_GLContext m_context;
};

using SDL2GLContext_uptr = std::unique_ptr< SDL2GLContext >;

class SDL2UserEvent {
public:
    SDL2UserEvent(SDL2_uptr const& sdl2);
    constexpr auto get_type(void) noexcept -> Uint32 { return m_id; }

private:
    Uint32 const m_id;
};

using SDL2UserEvent_uptr = std::unique_ptr< SDL2UserEvent >;

};

#endif
