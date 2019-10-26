#include "entropy.hpp"

#include <QtConcurrent>

#include <array>
#include <cmath>

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
            QRgb r = r_g_b[0].pixel(x, y);
            QRgb g = r_g_b[1].pixel(x, y);
            QRgb b = r_g_b[2].pixel(x, y);

            QRgb outRgb = 0xff000000 | (r & 0xff0000) | (g & 0xff00) | (b & 0xff);

            rgb.setPixel(x, y, outRgb);
        }
    }

    return rgb;
}

bool isGrayscale( const QImage &image )
{
    return image.format() == QImage::Format_Grayscale8 || image.format() == QImage::Format_Grayscale16;
}

QImage calculateGrayscaleEntropyImageFrom( const QImage &image )
{
    constexpr int radius_x = 5;
    constexpr int radius_y = 5;

    const int width = image.width();
    const int height = image.height();

    QImage out(width, height, QImage::Format_Grayscale8);

    for( int y = 0; y < height; ++y )
        for( int x = 0; x < width; ++x )
        {
            int count = 0;
            int counts[256] = {0};

            for( int kernel_y = y - radius_y; kernel_y < y + radius_y; ++kernel_y )
                if( kernel_y >= 0 && kernel_y < height )
                    for( int kernel_x = x - radius_x; kernel_x < x + radius_x; ++kernel_x )
                        if( kernel_x >= 0 && kernel_x < width )
                        {
                            QRgb value = image.pixel( kernel_x, kernel_y ) & 0xff;
                            ++counts[ value ];
                            ++count;
                        }

            QRgb out_pixel = 0xff000000;

            if( count > 0 )
            {
                float rcount = 1.f / (float)count;
                float entropy = 0.f;

                for( int c : counts )
                    if( c != 0 )
                    {
                        float prob = (float)c * rcount;
                        entropy -= prob * log2f( prob );
                    }

                float entropy_limit = -log2f( rcount );
                float proportional_entropy = entropy / entropy_limit;

                QRgb mono = (QRgb)(255.f * proportional_entropy);

                out_pixel |= (mono << 16) | (mono << 8) | mono;
            }

            out.setPixel( x, y, out_pixel );
        }

    return out;
}

QImage calculateEntropyImageFrom( const QImage &image )
{
    if( isGrayscale( image ))
        return calculateGrayscaleEntropyImageFrom( image );
    // else assume rgb

    std::array<QImage, 3> r_g_b = splitRgbFrom( image );

    QtConcurrent::map( r_g_b, [](QImage &channel){ channel = calculateGrayscaleEntropyImageFrom( channel ); })
    .waitForFinished();

    return joinRgbFrom( r_g_b );
}
}
