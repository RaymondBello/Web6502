#include "2DEngine.h"

static const Pixel
    GREY(192, 192, 192),
    DARK_GREY(128, 128, 128), VERY_DARK_GREY(64, 64, 64),
    RED(255, 0, 0), DARK_RED(128, 0, 0), VERY_DARK_RED(64, 0, 0),
    YELLOW(255, 255, 0), DARK_YELLOW(128, 128, 0), VERY_DARK_YELLOW(64, 64, 0),
    GREEN(0, 255, 0), DARK_GREEN(0, 128, 0), VERY_DARK_GREEN(0, 64, 0),
    CYAN(0, 255, 255), DARK_CYAN(0, 128, 128), VERY_DARK_CYAN(0, 64, 64),
    BLUE(0, 0, 255), DARK_BLUE(0, 0, 128), VERY_DARK_BLUE(0, 0, 64),
    MAGENTA(255, 0, 255), DARK_MAGENTA(128, 0, 128), VERY_DARK_MAGENTA(64, 0, 64),
    WHITE(255, 255, 255), BLACK(0, 0, 0), BLANK(0, 0, 0, 0);

// O------------------------------------------------------------------------------O
// | Pixel IMPLEMENTATION                                                         |
// O------------------------------------------------------------------------------O
Pixel::Pixel()
{
    r = 0;
    g = 150;
    b = 0;
    a = nDefaultAlpha;
}

Pixel::Pixel(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
    n = red | (green << 8) | (blue << 16) | (alpha << 24);
} // Thanks jarekpelczar

Pixel::Pixel(uint32_t p)
{
    n = p;
}

bool Pixel::operator==(const Pixel &p) const
{
    return n == p.n;
}

bool Pixel::operator!=(const Pixel &p) const
{
    return n != p.n;
}

Pixel Pixel::operator*(const float i) const
{
    float fR = std::min(255.0f, std::max(0.0f, float(r) * i));
    float fG = std::min(255.0f, std::max(0.0f, float(g) * i));
    float fB = std::min(255.0f, std::max(0.0f, float(b) * i));
    return Pixel(uint8_t(fR), uint8_t(fG), uint8_t(fB), a);
}

Pixel Pixel::operator/(const float i) const
{
    float fR = std::min(255.0f, std::max(0.0f, float(r) / i));
    float fG = std::min(255.0f, std::max(0.0f, float(g) / i));
    float fB = std::min(255.0f, std::max(0.0f, float(b) / i));
    return Pixel(uint8_t(fR), uint8_t(fG), uint8_t(fB), a);
}

Pixel &Pixel::operator*=(const float i)
{
    this->r = uint8_t(std::min(255.0f, std::max(0.0f, float(r) * i)));
    this->g = uint8_t(std::min(255.0f, std::max(0.0f, float(g) * i)));
    this->b = uint8_t(std::min(255.0f, std::max(0.0f, float(b) * i)));
    return *this;
}

Pixel &Pixel::operator/=(const float i)
{
    this->r = uint8_t(std::min(255.0f, std::max(0.0f, float(r) / i)));
    this->g = uint8_t(std::min(255.0f, std::max(0.0f, float(g) / i)));
    this->b = uint8_t(std::min(255.0f, std::max(0.0f, float(b) / i)));
    return *this;
}

Pixel Pixel::operator+(const Pixel &p) const
{
    uint8_t nR = uint8_t(std::min(255, std::max(0, int(r) + int(p.r))));
    uint8_t nG = uint8_t(std::min(255, std::max(0, int(g) + int(p.g))));
    uint8_t nB = uint8_t(std::min(255, std::max(0, int(b) + int(p.b))));
    return Pixel(nR, nG, nB, a);
}

Pixel Pixel::operator-(const Pixel &p) const
{
    uint8_t nR = uint8_t(std::min(255, std::max(0, int(r) - int(p.r))));
    uint8_t nG = uint8_t(std::min(255, std::max(0, int(g) - int(p.g))));
    uint8_t nB = uint8_t(std::min(255, std::max(0, int(b) - int(p.b))));
    return Pixel(nR, nG, nB, a);
}

Pixel &Pixel::operator+=(const Pixel &p)
{
    this->r = uint8_t(std::min(255, std::max(0, int(r) + int(p.r))));
    this->g = uint8_t(std::min(255, std::max(0, int(g) + int(p.g))));
    this->b = uint8_t(std::min(255, std::max(0, int(b) + int(p.b))));
    return *this;
}

Pixel &Pixel::operator-=(const Pixel &p) // Thanks Au Lit
{
    this->r = uint8_t(std::min(255, std::max(0, int(r) - int(p.r))));
    this->g = uint8_t(std::min(255, std::max(0, int(g) - int(p.g))));
    this->b = uint8_t(std::min(255, std::max(0, int(b) - int(p.b))));
    return *this;
}

Pixel Pixel::inv() const
{
    uint8_t nR = uint8_t(std::min(255, std::max(0, 255 - int(r))));
    uint8_t nG = uint8_t(std::min(255, std::max(0, 255 - int(g))));
    uint8_t nB = uint8_t(std::min(255, std::max(0, 255 - int(b))));
    return Pixel(nR, nG, nB, a);
}

// O------------------------------------------------------------------------------O
// | Sprite IMPLEMENTATION                                                        |
// O------------------------------------------------------------------------------O
Sprite::Sprite()
{
    width = 0;
    height = 0;
}

Sprite::Sprite(int32_t w, int32_t h)
{
    width = w;
    height = h;
    pColData.resize(width * height);
    pColData.resize(width * height, nDefaultPixel);
}

Sprite::~Sprite()
{
    pColData.clear();
}

void Sprite::SetSampleMode(Sprite::Mode mode)
{
    modeSample = mode;
}

Pixel Sprite::GetPixel(const vi2d &a) const
{
    return GetPixel(a.x, a.y);
}

bool Sprite::SetPixel(const vi2d &a, Pixel p)
{
    return SetPixel(a.x, a.y, p);
}

Pixel Sprite::GetPixel(int32_t x, int32_t y) const
{
    if (modeSample == Sprite::Mode::NORMAL)
    {
        if (x >= 0 && x < width && y >= 0 && y < height)
            return pColData[y * width + x];
        else
            return Pixel(0, 0, 0, 0);
    }
    else
    {
        return pColData[abs(y % height) * width + abs(x % width)];
    }
}

bool Sprite::SetPixel(int32_t x, int32_t y, Pixel p)
{
    if (x >= 0 && x < width && y >= 0 && y < height)
    {
        pColData[y * width + x] = p;
        return true;
    }
    else
        return false;
}

Pixel Sprite::Sample(float x, float y) const
{
    int32_t sx = std::min((int32_t)((x * (float)width)), width - 1);
    int32_t sy = std::min((int32_t)((y * (float)height)), height - 1);
    return GetPixel(sx, sy);
}

Pixel Sprite::SampleBL(float u, float v) const
{
    u = u * width - 0.5f;
    v = v * height - 0.5f;
    int x = (int)floor(u); // cast to int rounds toward zero, not downward
    int y = (int)floor(v); // Thanks @joshinils
    float u_ratio = u - x;
    float v_ratio = v - y;
    float u_opposite = 1 - u_ratio;
    float v_opposite = 1 - v_ratio;

    Pixel p1 = GetPixel(std::max(x, 0), std::max(y, 0));
    Pixel p2 = GetPixel(std::min(x + 1, (int)width - 1), std::max(y, 0));
    Pixel p3 = GetPixel(std::max(x, 0), std::min(y + 1, (int)height - 1));
    Pixel p4 = GetPixel(std::min(x + 1, (int)width - 1), std::min(y + 1, (int)height - 1));

    return Pixel(
        (uint8_t)((p1.r * u_opposite + p2.r * u_ratio) * v_opposite + (p3.r * u_opposite + p4.r * u_ratio) * v_ratio),
        (uint8_t)((p1.g * u_opposite + p2.g * u_ratio) * v_opposite + (p3.g * u_opposite + p4.g * u_ratio) * v_ratio),
        (uint8_t)((p1.b * u_opposite + p2.b * u_ratio) * v_opposite + (p3.b * u_opposite + p4.b * u_ratio) * v_ratio));
}

Pixel *Sprite::GetData()
{
    return pColData.data();
}

Sprite *Sprite::Duplicate()
{
    Sprite *spr = new Sprite(width, height);
    std::memcpy(spr->GetData(), GetData(), width * height * sizeof(Pixel));
    spr->modeSample = modeSample;
    return spr;
}

Sprite *Sprite::Duplicate(const vi2d &vPos, const vi2d &vSize)
{
    Sprite *spr = new Sprite(vSize.x, vSize.y);
    for (int y = 0; y < vSize.y; y++)
        for (int x = 0; x < vSize.x; x++)
            spr->SetPixel(x, y, GetPixel(vPos.x + x, vPos.y + y));
    return spr;
}