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
    std::optional<draw::ShapeValueWrapper> operator()(
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
        auto look = draw::Look{};
        look.stroke.enable = true;
        look.fill.enable = false;
        look.stroke.color.hue = 300.0;
        look.stroke.color.saturation = 1.0;
        look.stroke.color.value = 1.0;
        look.stroke.antialias = true;

        return draw::ShapeValueWrapper::Create<draw::PolygonShape>(
            0,
            pex::Order{},
            polygon,
            look,
            draw::NodeSettings{});
    }
};


using DragReplaceMaskPolygon = draw::DragReplaceShape<CreateMaskPolygon>;


class MaskBrain
{
public:
    MaskBrain(
        const MaskControl &maskControl,
        const draw::PixelViewControl &pixelViewControl);

    void UpdateDisplay();

private:
    void OnMask_(const MaskSettings &);

    draw::ShapesId shapesId_;

    using MaskShapesBrain = draw::ShapeEditor<DragReplaceMaskPolygon>;

    MaskShapesBrain maskShapesBrain_;

    pex::Endpoint<MaskBrain, MaskControl> maskEndpoint_;
    draw::PixelViewControl pixelViewControl_;
};


} // end namespace iris
