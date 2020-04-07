#ifndef __NK_SDL2_H__
#define __NK_SDL2_H__

extern "C" {
#include <SDL2/SDL.h>
}

#include <memory>
#include <iterator>

#include "./wave.h"

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

};

#endif
