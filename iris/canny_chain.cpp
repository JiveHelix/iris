#include "iris/canny_chain.h"
#include "iris/default.h"


namespace iris
{


std::shared_ptr<draw::Pixels> CannyChainResults::Display(
    ThreadsafeColor<int32_t> &color) const
{
    if (!this->gaussian)
    {
        // There's nothing to display if the first filter in the chain has no
        // result.
        return {};
    }

    if (this->canny)
    {
        return std::make_shared<draw::Pixels>(this->canny->Colorize());
    }

    // Canny didn't return a result.
    if (this->gradient)
    {
        return std::make_shared<draw::Pixels>(this->gradient->Colorize());
    }

    return std::make_shared<draw::Pixels>(color.Filter(*this->gaussian));
}


template class CannyChain<DefaultLevelAdjustNode>;


} // end namespace iris
