#include "iris/views/mask_brain.h"
#include <draw/polygon_shape.h>


namespace iris
{


MaskBrain::MaskBrain(
    MaskControl maskControl,
    draw::PixelViewControl pixelViewControl)
    :
    shapesId_(),
    maskShapesBrain_(maskControl.polygons, pixelViewControl),
    maskEndpoint_(this, maskControl, &MaskBrain::OnMask_),
    pixelViewControl_(pixelViewControl)
{

}


void MaskBrain::UpdateDisplay()
{
    this->OnMask_(this->maskEndpoint_.Get());
}


void MaskBrain::OnMask_(const MaskSettings &maskSettings)
{
    auto shapes = draw::Shapes(this->shapesId_.Get());

    if (!maskSettings.showOutline)
    {
        this->pixelViewControl_.asyncShapes.Set(shapes);
        return;
    }

    for (const auto &shapeValue: maskSettings.polygons)
    {
        shapes.Append(shapeValue.GetValueBase()->Copy());
    }

    this->pixelViewControl_.asyncShapes.Set(shapes);
}


} // end namespace iris
