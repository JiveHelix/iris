#include "iris/canny_chain.h"
#include "iris/default.h"


namespace iris
{


std::shared_ptr<draw::Pixels> CannyChainResults::Display(
    ThreadsafeColorMap<int32_t> &color) const
{
    if (!this->gaussian)
    {
        // There's nothing to display if the first filter in the chain has no
        // result.
        return {};
    }

    if (this->canny)
    {
        return this->canny->Colorize();
    }

    // Canny didn't return a result.
    if (this->gradient)
    {
        return this->gradient->Colorize();
    }

    return color.Filter(this->margins_.RemoveMargin(*this->gaussian));
}


template class CannyChain<DefaultLevelAdjustNode>;


} // end namespace iris
