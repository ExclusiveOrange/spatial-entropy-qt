#include "entropy.hpp"

#include <array>

namespace Entropy
{
std::array<QImage, 3> splitRgbFrom(const QImage &image)
{
    const int width = image.width();
    const int height = image.height();

    QImage r(width, height, QImage::Format_Grayscale8);
    QImage g(width, height, QImage::Format_Grayscale8);
    QImage b(width, height, QImage::Format_Grayscale8);

    auto toQRgb = [](const QRgb mono){ return 0xff000000 | (mono << 16) | (mono << 8) | mono; };

    for( int y = 0; y < height; ++y )
    {
        for( int x = 0; x < width; ++x )
        {
            QRgb inRgb = image.pixel(x, y);

            r.setPixel(x, y, toQRgb((inRgb >> 16) & 0xff));
            g.setPixel(x, y, toQRgb((inRgb >> 8) & 0xff));
            b.setPixel(x, y, toQRgb(inRgb & 0xff));
        }
    }

    return {r, g, b};
}

QImage joinRgbFrom(const std::array<QImage, 3> &r_g_b)
{
    const int width = r_g_b[0].width();
    const int height = r_g_b[0].height();

    QImage rgb(width, height, QImage::Format_RGB32);

    for( int y = 0; y < height; ++y )
    {
        for( int x = 0; x < width; ++x )
        {
            QRgb r = r_g_b[0].pixel(x, y) & 0xff;
            QRgb g = r_g_b[1].pixel(x, y) & 0xff;
            QRgb b = r_g_b[2].pixel(x, y) & 0xff;

            QRgb outRgb =
                    0xff000000 |
                    (r << 16) |
                    (g << 8) |
                    b;

            rgb.setPixel(x, y, outRgb);
        }
    }

    return rgb;
}

QImage calculateEntropyImageFrom( const QImage &image )
{
    // TEST: split and join
    return joinRgbFrom( splitRgbFrom( image ));
}
}
