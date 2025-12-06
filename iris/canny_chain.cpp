#include "iris/canny_chain.h"
#include "iris/default.h"


namespace iris
{


std::shared_ptr<draw::Pixels> CannyChainResults::Display(
    const tau::Margins &margins,
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
        return this->canny->Colorize(margins);
    }

    // Canny didn't return a result.
    if (this->gradient)
    {
        return this->gradient->Colorize(margins);
    }

    return color.Filter(*this->gaussian);
}


template class CannyChain<DefaultLevelAdjustNode>;


} // end namespace iris
