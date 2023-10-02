#include "demo_brain.h"


std::shared_ptr<draw::Pixels> DemoBrain::Process()
{
    this->userControl_.pixelView.asyncShapes.Set(
        draw::Shapes::MakeResetter());

    auto chainResult = this->filters_.chess.GetChainResults();

    this->maskBrain_.UpdateDisplay();

    if (!chainResult)
    {
        return {};
    }

    return chainResult->Display(
        this->userControl_.pixelView.asyncShapes,
        this->demoModel_.chess.linesShape.Get(),
        this->demoModel_.chess.verticesShape.Get(),
        this->demoModel_.chessShape.Get(),
        this->filters_.color,
        {});
}

