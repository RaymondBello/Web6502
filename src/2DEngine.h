#include <cmath>
#include <cstdint>
#include <string>
#include <iostream>
#include <streambuf>
#include <sstream>
#include <chrono>
#include <vector>
#include <list>
#include <thread>
#include <atomic>
#include <fstream>
#include <map>
#include <functional>
#include <algorithm>
#include <array>
#include <cstring>
#include <filesystem>

#include "imgui.h"

#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__EMSCRIPTEN__)
#define IMAGE_LIBPNG
#endif

#define UNUSED(x) (void)(x)

template <class T>
struct v2d_generic
{
    T x = 0;
    T y = 0;
    v2d_generic() : x(0), y(0) {}
    v2d_generic(T _x, T _y) : x(_x), y(_y) {}
    v2d_generic(const v2d_generic &v) : x(v.x), y(v.y) {}
    v2d_generic &operator=(const v2d_generic &v) = default;
    T mag() const { return T(std::sqrt(x * x + y * y)); }
    T mag2() const { return x * x + y * y; }
    v2d_generic norm() const
    {
        T r = 1 / mag();
        return v2d_generic(x * r, y * r);
    }
    v2d_generic perp() const { return v2d_generic(-y, x); }
    v2d_generic floor() const { return v2d_generic(std::floor(x), std::floor(y)); }
    v2d_generic ceil() const { return v2d_generic(std::ceil(x), std::ceil(y)); }
    v2d_generic max(const v2d_generic &v) const { return v2d_generic(std::max(x, v.x), std::max(y, v.y)); }
    v2d_generic min(const v2d_generic &v) const { return v2d_generic(std::min(x, v.x), std::min(y, v.y)); }
    T dot(const v2d_generic &rhs) const { return this->x * rhs.x + this->y * rhs.y; }
    T cross(const v2d_generic &rhs) const { return this->x * rhs.y - this->y * rhs.x; }
    v2d_generic operator+(const v2d_generic &rhs) const { return v2d_generic(this->x + rhs.x, this->y + rhs.y); }
    v2d_generic operator-(const v2d_generic &rhs) const { return v2d_generic(this->x - rhs.x, this->y - rhs.y); }
    v2d_generic operator*(const T &rhs) const { return v2d_generic(this->x * rhs, this->y * rhs); }
    v2d_generic operator*(const v2d_generic &rhs) const { return v2d_generic(this->x * rhs.x, this->y * rhs.y); }
    v2d_generic operator/(const T &rhs) const { return v2d_generic(this->x / rhs, this->y / rhs); }
    v2d_generic operator/(const v2d_generic &rhs) const { return v2d_generic(this->x / rhs.x, this->y / rhs.y); }
    v2d_generic &operator+=(const v2d_generic &rhs)
    {
        this->x += rhs.x;
        this->y += rhs.y;
        return *this;
    }
    v2d_generic &operator-=(const v2d_generic &rhs)
    {
        this->x -= rhs.x;
        this->y -= rhs.y;
        return *this;
    }
    v2d_generic &operator*=(const T &rhs)
    {
        this->x *= rhs;
        this->y *= rhs;
        return *this;
    }
    v2d_generic &operator/=(const T &rhs)
    {
        this->x /= rhs;
        this->y /= rhs;
        return *this;
    }
    v2d_generic &operator*=(const v2d_generic &rhs)
    {
        this->x *= rhs.x;
        this->y *= rhs.y;
        return *this;
    }
    v2d_generic &operator/=(const v2d_generic &rhs)
    {
        this->x /= rhs.x;
        this->y /= rhs.y;
        return *this;
    }
    v2d_generic operator+() const { return {+x, +y}; }
    v2d_generic operator-() const { return {-x, -y}; }
    bool operator==(const v2d_generic &rhs) const { return (this->x == rhs.x && this->y == rhs.y); }
    bool operator!=(const v2d_generic &rhs) const { return (this->x != rhs.x || this->y != rhs.y); }
    const std::string str() const { return std::string("(") + std::to_string(this->x) + "," + std::to_string(this->y) + ")"; }
    friend std::ostream &operator<<(std::ostream &os, const v2d_generic &rhs)
    {
        os << rhs.str();
        return os;
    }
    operator v2d_generic<int32_t>() const { return {static_cast<int32_t>(this->x), static_cast<int32_t>(this->y)}; }
    operator v2d_generic<float>() const { return {static_cast<float>(this->x), static_cast<float>(this->y)}; }
    operator v2d_generic<double>() const { return {static_cast<double>(this->x), static_cast<double>(this->y)}; }
};

// Note: joshinils has some good suggestions here, but they are complicated to implement at this moment,
// however they will appear in a future version of PGE
template <class T>
inline v2d_generic<T> operator*(const float &lhs, const v2d_generic<T> &rhs)
{
    return v2d_generic<T>((T)(lhs * (float)rhs.x), (T)(lhs * (float)rhs.y));
}
template <class T>
inline v2d_generic<T> operator*(const double &lhs, const v2d_generic<T> &rhs)
{
    return v2d_generic<T>((T)(lhs * (double)rhs.x), (T)(lhs * (double)rhs.y));
}
template <class T>
inline v2d_generic<T> operator*(const int &lhs, const v2d_generic<T> &rhs)
{
    return v2d_generic<T>((T)(lhs * (int)rhs.x), (T)(lhs * (int)rhs.y));
}
template <class T>
inline v2d_generic<T> operator/(const float &lhs, const v2d_generic<T> &rhs)
{
    return v2d_generic<T>((T)(lhs / (float)rhs.x), (T)(lhs / (float)rhs.y));
}
template <class T>
inline v2d_generic<T> operator/(const double &lhs, const v2d_generic<T> &rhs)
{
    return v2d_generic<T>((T)(lhs / (double)rhs.x), (T)(lhs / (double)rhs.y));
}
template <class T>
inline v2d_generic<T> operator/(const int &lhs, const v2d_generic<T> &rhs)
{
    return v2d_generic<T>((T)(lhs / (int)rhs.x), (T)(lhs / (int)rhs.y));
}

// To stop dandistine crying...
template <class T, class U>
inline bool operator<(const v2d_generic<T> &lhs, const v2d_generic<U> &rhs)
{
    return lhs.y < rhs.y || (lhs.y == rhs.y && lhs.x < rhs.x);
}
template <class T, class U>
inline bool operator>(const v2d_generic<T> &lhs, const v2d_generic<U> &rhs)
{
    return lhs.y > rhs.y || (lhs.y == rhs.y && lhs.x > rhs.x);
}

typedef v2d_generic<int32_t> vi2d;
typedef v2d_generic<uint32_t> vu2d;
typedef v2d_generic<float> vf2d;
typedef v2d_generic<double> vd2d;

namespace _gfs = std::filesystem;

constexpr uint8_t nMouseButtons = 5;
constexpr uint8_t nDefaultAlpha = 0xFF;
constexpr uint32_t nDefaultPixel = (nDefaultAlpha << 24);
enum rcode
{
    FAIL = 0,
    OK = 1,
    NO_FILE = -1
};

struct Pixel
{
    union
    {
        uint32_t n = nDefaultPixel;
        struct
        {
            uint8_t r;
            uint8_t g;
            uint8_t b;
            uint8_t a;
        };
    };

    enum Mode
    {
        NORMAL,
        MASK,
        ALPHA,
        CUSTOM
    };

    Pixel();
    Pixel(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = nDefaultAlpha);
    Pixel(uint32_t p);
    Pixel &operator=(const Pixel &v) = default;
    bool operator==(const Pixel &p) const;
    bool operator!=(const Pixel &p) const;
    Pixel operator*(const float i) const;
    Pixel operator/(const float i) const;
    Pixel &operator*=(const float i);
    Pixel &operator/=(const float i);
    Pixel operator+(const Pixel &p) const;
    Pixel operator-(const Pixel &p) const;
    Pixel &operator+=(const Pixel &p);
    Pixel &operator-=(const Pixel &p);
    Pixel inv() const;
};

class Sprite
{
public:
    Sprite();
    Sprite(int32_t w, int32_t h);
    // Sprite(const Sprite &) = delete;
    ~Sprite();


public:
    int32_t width = 0;
    int32_t height = 0;

    ImTextureID TexID;

    enum Mode
    {
        NORMAL,
        PERIODIC
    };
    enum Flip
    {
        NONE = 0,
        HORIZ = 1,
        VERT = 2
    };

public:
    void SetSampleMode(Sprite::Mode mode = Sprite::Mode::NORMAL);
    Pixel GetPixel(int32_t x, int32_t y) const;
    bool SetPixel(int32_t x, int32_t y, Pixel p);
    Pixel GetPixel(const vi2d &a) const;
    bool SetPixel(const vi2d &a, Pixel p);
    Pixel Sample(float x, float y) const;
    Pixel SampleBL(float u, float v) const;
    Pixel *GetData();
    Sprite *Duplicate();
    Sprite *Duplicate(const vi2d &vPos, const vi2d &vSize);
    std::vector<Pixel> pColData;
    Mode modeSample = Mode::NORMAL;

};



