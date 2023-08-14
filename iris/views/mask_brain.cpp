#include "iris/views/mask_brain.h"
#include <draw/polygon_shape.h>


namespace iris
{


MaskBrain::MaskBrain(
    MaskControl maskControl,
    draw::PixelViewControl pixelViewControl)
    :
    shapesId_(),
    polygonBrain_(maskControl.polygon, pixelViewControl),
    maskEndpoint_(this, maskControl, &MaskBrain::OnMask_),
    pixelViewControl_(pixelViewControl)
{

}


void MaskBrain::UpdateDisplay()
{
    this->OnMask_(this->maskEndpoint_.control.Get());
}


void MaskBrain::OnMask_(const MaskSettings &maskSettings)
{
    auto shapes = draw::Shapes(this->shapesId_.Get());

    if (!maskSettings.showOutline)
    {
        this->pixelViewControl_.asyncShapes.Set(shapes);
        return;
    }

    // Show the mask outline in magenta.
    auto look = draw::Look::Default();
    look.strokeEnable = true;
    look.fillEnable = false;
    look.strokeColor.hue = 300.0;
    look.strokeColor.saturation = 1.0;
    look.strokeColor.value = 1.0;
    look.antialias = true;

    shapes.EmplaceBack<draw::PolygonShape>(maskSettings.polygon, look);
    this->pixelViewControl_.asyncShapes.Set(shapes);
}



} // end namespace iris
