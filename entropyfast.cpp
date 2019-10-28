#include "entropyfast.hpp"

#include <QtConcurrent>

#include <algorithm>
#include <array>
#include <cstdint>

namespace
{
struct ColorPlane
{
    int width = 0;
    int height = 0;
    uint8_t * pixels = nullptr;

    explicit ColorPlane() = delete;
    explicit ColorPlane( const ColorPlane & copy ) = delete;

    ColorPlane( int width, int height )
        : width{std::max(0, width)}
        , height{std::max(0, height)}
        , pixels{ new uint8_t[ static_cast<unsigned>(std::max(0, width) * std::max(0, height)) ]} {}


    ColorPlane( ColorPlane && move )
        : width{ std::exchange( move.width, 0 )}
        , height{ std::exchange( move.height, 0 )}
        , pixels{ std::exchange( move.pixels, nullptr )} {}

    ~ColorPlane() { delete[] pixels; }

    ColorPlane & operator=( ColorPlane && move )
    {
        std::swap( width, move.width );
        std::swap( height, move.height );
        std::swap( pixels, move.pixels );
        return *this;
    }

    uint8_t * row(int index) const { return pixels + width * std::clamp( index, 0, height - 1); }
};

ColorPlane calculateEntropyPlaneFrom( const ColorPlane &plane )
{
    constexpr int radius_x = 5;
    constexpr int radius_y = 5;

    const int width = plane.width;
    const int height = plane.height;

    ColorPlane entropyPlane( width, height );

    for( int y = 0; y < height; ++y )
    {
        uint8_t * p_output_row = entropyPlane.row( y );

        for( int x = 0; x < width; ++x )
        {
            int count = 0;
            int counts[256] = {0};

            for( int kernel_y = y - radius_y; kernel_y < y + radius_y; ++kernel_y )
                if( kernel_y >= 0 && kernel_y < height )
                {
                    uint8_t * p_input_row = plane.row( kernel_y );

                    for( int kernel_x = x - radius_x; kernel_x < x + radius_x; ++kernel_x )
                        if( kernel_x >= 0 && kernel_x < width )
                        {
                            uint8_t value = p_input_row[ kernel_x ];
                            ++counts[ value ];
                            ++count;
                        }
                }

            float rcount = 1.f / static_cast<float>(count);
            float entropy = 0.f;

            for( int c : counts )
                if( c != 0 )
                {
                    float prob = static_cast<float>(c) * rcount;
                    entropy -= prob * log2f( prob );
                }

            float entropy_limit = -log2f( rcount );
            float proportional_entropy = entropy / entropy_limit;

            // EXPERIMENT
            proportional_entropy *= proportional_entropy;

            p_output_row[ x ] = static_cast< uint8_t >( 255.f * proportional_entropy );
        }
    }

    return entropyPlane;
}

QImage makeImageFromGrayscalePlane( const ColorPlane &plane )
{
    const int width = plane.width;
    const int height = plane.height;

    QImage image( width, height, QImage::Format_Grayscale8 );

    for( int y = 0; y < height; ++y )
    {
        uint8_t *p_gray = plane.row( y );
        QRgb *p_argb = reinterpret_cast< QRgb * >( image.scanLine( y ));

        for( int x = 0; x < width; ++x )
        {
            QRgb val = static_cast< QRgb >( p_gray[ x ]);
            p_argb[ x ] = 0xff000000 | (val << 16) | (val << 8) | val;
        }
    }

    return image;
}

QImage makeImageFromColorPlanes( const std::array< ColorPlane, 3 > &planes )
{
    const int width = planes[0].width;
    const int height = planes[0].height;

    QImage image( width, height, QImage::Format_RGB32 );

    for( int y = 0; y < height; ++y )
    {
        uint8_t *p_red = planes[0].row( y );
        uint8_t *p_green = planes[1].row( y );
        uint8_t *p_blue = planes[2].row( y );
        QRgb *p_argb = reinterpret_cast< QRgb * >( image.scanLine( y ));

        for( int x = 0; x < width; ++x )
        {
            QRgb red = static_cast< QRgb >( p_red[ x ]);
            QRgb green = static_cast< QRgb >( p_green[ x ]);
            QRgb blue = static_cast< QRgb >( p_blue[ x ]);
            p_argb[ x ] = 0xff000000 | (red << 16) | (green << 8) | blue;
        }
    }

    return image;
}

ColorPlane extractGrayscalePlaneFrom( const QImage &image )
{
    const int width = image.width();
    const int height = image.height();

    ColorPlane plane( width, height );

    for( int y = 0; y < height; ++y )
    {
        const QRgb *p_argb = reinterpret_cast< const QRgb * >( image.scanLine( y ));
        uint8_t *p_gray = plane.row( y );

        for( int x = 0; x < width; ++x )
            p_gray[ x ] = static_cast< uint8_t >( p_argb[ x ] & 0xff );
    }

    return plane;
}

std::array< ColorPlane, 3 > extractColorPlanesFrom( const QImage &image )
{
    const int width = image.width();
    const int height = image.height();

    ColorPlane red( width, height );
    ColorPlane green( width, height );
    ColorPlane blue( width, height );

    for( int y = 0; y < height; ++y )
    {
        const QRgb *p_argb = reinterpret_cast< const QRgb * >( image.scanLine( y ));
        uint8_t *p_red = red.row( y );
        uint8_t *p_green = green.row( y );
        uint8_t *p_blue = blue.row( y );

        for( int x = 0; x < width; ++x )
        {
            QRgb rgb = p_argb[ x ];
            p_red[ x ] = static_cast< uint8_t >( (rgb >> 16) & 0xff );
            p_green[ x ] = static_cast< uint8_t >( (rgb >> 8) & 0xff );
            p_blue[ x ] = static_cast< uint8_t >( rgb & 0xff );
        }
    }

    return { std::move( red ), std::move( green ), std::move( blue )};
}

QImage calculateEntropyImageFromGrayscale( const QImage &image )
{
    const QImage grayImage = (image.format() == QImage::Format_Grayscale8) ? image : image.convertToFormat( QImage::Format_Grayscale8 );
    ColorPlane grayPlane = extractGrayscalePlaneFrom( grayImage );
    ColorPlane entropyPlane = calculateEntropyPlaneFrom( grayPlane );
    return makeImageFromGrayscalePlane( entropyPlane );
}

QImage calculateEntropyImageFromRgb( const QImage &image )
{
    const QImage rgbImage = (image.format() == QImage::Format_RGB32) or (image.format() == QImage::Format_ARGB32) ? image : image.convertToFormat( QImage::Format_RGB32 );
    std::array< ColorPlane, 3 > r_g_b = extractColorPlanesFrom( rgbImage );
    QtConcurrent::map( r_g_b, [](ColorPlane &plane){ plane = calculateEntropyPlaneFrom(plane); })
    .waitForFinished();
    return makeImageFromColorPlanes( r_g_b );
}

bool isGrayscale( const QImage &image )
{
    return image.format() == QImage::Format_Grayscale8 || image.format() == QImage::Format_Grayscale16;
}
}

namespace EntropyFast
{
QImage calculateEntropyImageFrom( const QImage &image )
{
    if( isGrayscale( image ))
        return calculateEntropyImageFromGrayscale( image );

    return calculateEntropyImageFromRgb( image );
}
}
