#pragma once


#include "iris/mask_settings.h"
#include "iris/node.h"


namespace iris
{


using MaskMatrix = Eigen::MatrixX<double>;

MaskMatrix CreateMask(const MaskSettings &maskSettings);


template<typename Value>
class Mask
{
public:

    using Input =
        Eigen::Matrix<Value, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

    using Result = Input;

    Mask()
        :
        maskSettings_(MaskSettings{}),
        mask_()
    {

    }

    Mask(const Mask &) = delete;

    Mask & operator=(const Mask &other)
    {
        this->maskSettings_ = other.maskSettings_;
        this->mask_ = other.mask_;
        return *this;
    }

    Mask(const MaskSettings &maskSettings)
        :
        maskSettings_(maskSettings),
        mask_()
    {
        assert(!this->mask_);
    }

    bool Filter(const Input &input, Result &result)
    {
        if (!this->maskSettings_.enable)
        {
            return false;
        }

        if (!this->mask_)
        {
            this->mask_ = CreateMask(this->maskSettings_);
        }

        assert(input.rows() == this->mask_->rows());
        assert(input.cols() == this->mask_->cols());

        MaskMatrix resultAsFloat =
            input.template cast<double>().array() * this->mask_->array();

        result = resultAsFloat.template cast<Value>();

        return true;
    }


private:
    MaskSettings maskSettings_;
    std::optional<MaskMatrix> mask_;
};


extern template class Mask<int32_t>;

extern template class Node<DefaultSource, Mask<int32_t>, MaskControl>;

using DefaultMaskNode = Node<DefaultSource, Mask<int32_t>, MaskControl>;


} // end namespace iris
