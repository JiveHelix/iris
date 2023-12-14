#pragma once


#include <pex/endpoint.h>
#include <tau/vector2d.h>
#include <draw/polygon_brain.h>
#include <draw/views/pixel_view_settings.h>
#include "iris/mask_settings.h"


namespace iris
{


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
    draw::PolygonBrain polygonBrain_;
    pex::Endpoint<MaskBrain, MaskControl> maskEndpoint_;
    draw::PixelViewControl pixelViewControl_;
};


} // end namespace iris
