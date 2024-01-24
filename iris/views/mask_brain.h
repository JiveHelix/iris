#pragma once


#include <pex/endpoint.h>
#include <tau/vector2d.h>
#include <draw/polygon_brain.h>
#include <draw/views/pixel_view_settings.h>
#include "iris/mask_settings.h"


namespace iris
{


struct CreateMaskPolygon
{
    std::optional<MaskShapeValue> operator()(
        const draw::Drag &drag,
        const tau::Point2d<int> position)
    {
        auto size = drag.GetSize(position);

        if (size.GetArea() < 1)
        {
            return {};
        }

        auto lines = draw::PolygonLines(size);
        auto polygon = draw::Polygon(lines.GetPoints());
        polygon.center = drag.GetDragCenter(position);

        // Show the mask outline in magenta.
        auto look = draw::Look::Default();
        look.strokeEnable = true;
        look.fillEnable = false;
        look.strokeColor.hue = 300.0;
        look.strokeColor.saturation = 1.0;
        look.strokeColor.value = 1.0;
        look.antialias = true;

        return MaskShapeValue{
            {0, polygon, look, draw::NodeSettings::Default()}};
    }
};


using MaskShapesControl = decltype(MaskControl::polygons);


using DragReplaceMaskPolygon =
    draw::DragReplaceShape<MaskShapesControl, CreateMaskPolygon>;


class MaskBrain
{
public:
    MaskBrain(
        MaskControl maskControl,
        draw::PixelViewControl pixelViewControl);

    void UpdateDisplay();

private:
    void OnMask_(const MaskSettings &);

    draw::ShapesId shapesId_;

    using MaskShapesBrain = draw::ShapeBrain
    <
        MaskShapesControl,
        DragReplaceMaskPolygon
    >;

    MaskShapesBrain maskShapesBrain_;

    pex::Endpoint<MaskBrain, MaskControl> maskEndpoint_;
    draw::PixelViewControl pixelViewControl_;
};


} // end namespace iris
