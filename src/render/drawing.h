#pragma once

#include "types.h"
#include "render/font.h"
#include "math/vec2.h"

#include <cstdint>
#include <cmath>

namespace hop { namespace draw {

template <typename T>
struct Buffer
{
    T* buffer;
    uint32 pitch;
    uint32 width;
    uint32 height;
};

template <typename T>
void clear(const Buffer<T>& buffer, Vec2u p1, Vec2u p2, T color)
{
    if (!buffer.buffer) return;
    for (uint32 i = p1.y; i < p2.y; ++i)
        for (uint32 j = p1.x; j < p2.x; ++j)
            buffer.buffer[i * buffer.pitch + j] = color;
}

template <typename T>
void line(const Buffer<T>& buffer, Vec2f p1, Vec2f p2, T color)
{
    if (!buffer.buffer) return;
    if (p1.x < 0) p1.x = 0;
    if (p2.x < 0) p2.x = 0;
    if (p1.y < 0) p1.y = 0;
    if (p2.y < 0) p2.y = 0;
    if (p1.x >= buffer.width) p1.x = buffer.width;
    if (p2.x >= buffer.width) p2.x = buffer.width;
    if (p1.y >= buffer.height) p1.y = buffer.height;
    if (p2.y >= buffer.height) p2.y = buffer.height;

    float w = p2.x - p1.x;
    float h = p2.y - p1.y;
    float l = std::abs(w);
    if (std::abs(h) > l) l = std::abs(h);
    int il = (int)l;
    float dx = w / l;
    float dy = h / l;

    for (int i = 0; i <= il; ++i)
    {
        buffer.buffer[(int)p1.x + (int)p1.y * buffer.pitch] = color;
        p1.x += dx;
        p1.y += dy;
    }
}

template <typename T>
void box(const Buffer<T>& buffer, Vec2u p1, Vec2u p2, T color)
{
    if (!buffer.buffer) return;
    line(buffer, { (float)p1.x, (float)p1.y }, { (float)p2.x, (float)p1.y }, color);
    line(buffer, { (float)p2.x, (float)p1.y }, { (float)p2.x, (float)p2.y }, color);
    line(buffer, { (float)p1.x, (float)p2.y }, { (float)p2.x, (float)p2.y }, color);
    line(buffer, { (float)p1.x, (float)p1.y }, { (float)p1.x, (float)p2.y }, color);
}

template <typename T>
void bar(const Buffer<T>& buffer, Vec2u p1, Vec2u p2, T color)
{
    if (!buffer.buffer) return;
    if (p1.x > buffer.width) p1.x = buffer.width;
    if (p2.x > buffer.width) p2.x = buffer.width;
    if (p1.y > buffer.height) p1.y = buffer.height;
    if (p2.y > buffer.height) p2.y = buffer.height;

    T* ptr = buffer.buffer + p1.y * buffer.pitch + p1.x;

    for (uint32 y = p1.y; y < p2.y; ++y)
    {
        for (uint32 x = 0; x < (p2.x - p1.x); ++x)
            ptr[x] = color;
        ptr += buffer.pitch;
    }
}

template <typename T>
void print(const Buffer<T>& buffer, const char* str, Vec2u pos, T color)
{
    if (!buffer.buffer)
        return;
    if (pos.y + (uint32)FONT_HEIGHT > buffer.height)
        return;

    while (*str != '\0')
    {
        if (pos.x + (uint32)FONT_WIDTH > buffer.width)
            return;

        uint32 idx = pos.y * buffer.pitch + pos.x;
        const uint8 * const font = default_font[(uint8)*str];

        for (uint32 h = 0; h < FONT_HEIGHT; ++h)
        {
            for (uint32 w = 0; w < FONT_WIDTH; ++w)
            {
                if (font[FONT_HEIGHT - h - 1] & (1 << w))
                    buffer.buffer[idx] = color;
                ++idx;
            }
            idx += buffer.pitch - FONT_WIDTH;
        }
        pos.x += FONT_WIDTH + FONT_SPACE;
        ++str;
    }
}

} } // namespace hop::draw
