#include "iris/mask.h"


#include <wxpex/graphics.h>
#include <draw/polygon_shape.h>
#include <draw/shapes.h>
#include <draw/planar.h>


namespace iris
{


template class Mask<int32_t>;
template class Node<Source<ProcessMatrix>, Mask<int32_t>, MaskControl>;


MaskMatrix CreateMask(const MaskSettings &maskSettings)
{
    draw::Polygon polygon;

    if (maskSettings.polygons.empty())
    {
        return MaskMatrix::Zero(
            maskSettings.imageSize.height,
            maskSettings.imageSize.width).array() + 1.0;
    }

    const auto &value = maskSettings.polygons[0];

    auto valueBase = value.GetValueBase();
    auto shape = dynamic_cast<const PolygonShape *>(valueBase.get());

    if (!shape)
    {
        throw std::logic_error("Expected only polygons");
    }

    polygon = shape->shape;

    if (polygon.points.size() < 3)
    {
        return MaskMatrix::Zero(
            maskSettings.imageSize.height,
            maskSettings.imageSize.width).array() + 1.0;
    }

    wxBitmap bitmap(wxpex::ToWxSize(maskSettings.imageSize));
    wxMemoryDC dc(bitmap);

    auto look = draw::Look::Default();
    look.strokeEnable = false;
    look.fillEnable = true;

    // Use the graphics context to fill the polygon with red pixels.
    look.fillColor.hue = 0.0;
    look.fillColor.saturation = 1.0;
    look.fillColor.value = 1.0;
    look.antialias = true;

    {
        wxpex::GraphicsContext context(dc);
        draw::ConfigureLook(context, look);
        draw::DrawSegments(context, polygon.GetPoints());
    }

    dc.SelectObject(wxNullBitmap);

    auto updated = bitmap.ConvertToImage();

    assert(!updated.HasAlpha());

    using Interleaved =
        Eigen::Matrix<uint8_t, Eigen::Dynamic, 3, Eigen::RowMajor>;

    Eigen::Map<Interleaved> interleaved(
        updated.GetData(),
        maskSettings.imageSize.GetArea(),
        3);

    auto planarRgb = draw::PlanarRgb<uint8_t>::FromInterleaved(
        interleaved,
        maskSettings.imageSize.height,
        maskSettings.imageSize.width);

    auto red = tau::GetRed(planarRgb).template cast<double>().eval();
    red.array() /= 255.0;

    if (!maskSettings.feather.enable)
    {
        return red;
    }

    auto gaussian = Gaussian<double, 0>(maskSettings.feather);
    auto feather = gaussian.Filter(red);

    // We've already checked that feather is enabled
    assert(feather);
    return *feather;
}




}// end namespace iris
