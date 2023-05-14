#include "iris/png.h"
#include <jive/overflow.h>


namespace iris
{


static png_uint_32 ToPngInt(Eigen::Index value)
{
    if (!jive::CheckConvertible<png_uint_32>(value))
    {
        throw PngError("Value is too large for png format.");
    }

    return static_cast<png_uint_32>(value);
}


PlanarRgb<uint8_t> ReadPng(const std::string &fileName)
{
    using Eigen::Index;

    png_image image;
    memset(&image, 0, sizeof(image));
    image.version = PNG_IMAGE_VERSION;

    png_image_begin_read_from_file(&image, fileName.c_str());

    image.format = PNG_FORMAT_RGB;

    using Interleaved =
        Eigen::Matrix<uint8_t, Eigen::Dynamic, 3, Eigen::RowMajor>;

    Interleaved interleaved(Index(image.height * image.width), 3);

    png_image_finish_read(
        &image,
        nullptr,
        interleaved.data(),
        0,
        nullptr);

    if (image.opaque)
    {
        png_image_free(&image);
    }

    if (image.warning_or_error > 2)
    {
        throw PngError(image.message);
    }

    if (image.warning_or_error == 1)
    {
        std::cerr << "PNG Warning: " << image.message << std::endl;
    }

    return PlanarRgb<uint8_t>::FromInterleaved(
        interleaved,
        Index(image.height),
        Index(image.width));
}


void WritePng(const PlanarRgb<uint8_t> &planarRgb, const std::string &fileName)
{
    using Eigen::Index;

    png_image image;
    memset(&image, 0, sizeof(image));
    image.version = PNG_IMAGE_VERSION;
    image.format = PNG_FORMAT_RGB;
    image.width = ToPngInt(planarRgb.GetColumnCount());
    image.height = ToPngInt(planarRgb.GetRowCount());

    using Interleaved =
        Eigen::Matrix<uint8_t, Eigen::Dynamic, 3, Eigen::RowMajor>;

    Interleaved interleaved = planarRgb.GetInterleaved<Eigen::RowMajor>();

    std::cout << "image.width = " << image.width << std::endl;
    std::cout << "image.height = " << image.height << std::endl;
    std::cout << "interleaved.rows() = " << interleaved.rows() << std::endl;
    std::cout << "interleaved.cols() = " << interleaved.cols() << std::endl;

    int pngResult = png_image_write_to_file(
        &image,
        fileName.c_str(),
        0,
        interleaved.data(),
        0,
        nullptr);

    std::cout << "FYI: pngResult=" << pngResult << std::endl;

    if (image.opaque)
    {
        png_image_free(&image);
    }

    if (image.warning_or_error > 2)
    {
        throw PngError(image.message);
    }

    if (image.warning_or_error == 1)
    {
        std::cerr << "PNG Warning: " << image.message << std::endl;
    }
}


} // end namespace iris
