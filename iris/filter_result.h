#pragma once


#include <tau/margins.h>


namespace iris
{


class FilterResult
{
public:
    FilterResult()
        :
        margins_{}
    {

    }

    void SetMargins(const tau::Margins &margins)
    {
        this->margins_ = margins;
    }

    tau::Margins GetMargins() const
    {
        return this->margins_;
    }

protected:
    tau::Margins margins_;
};


} // end namespace iris
