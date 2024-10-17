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

    std::optional<Result> Filter(const Input &data)
    {
        if (!this->maskSettings_.enable)
        {
            return data;
        }

        if (!this->mask_)
        {
            this->mask_ = CreateMask(this->maskSettings_);
        }

        assert(data.rows() == this->mask_->rows());
        assert(data.cols() == this->mask_->cols());

        MaskMatrix result =
            data.template cast<double>().array() * this->mask_->array();

        return result.template cast<Value>();
    }


private:
    MaskSettings maskSettings_;
    std::optional<MaskMatrix> mask_;
};


extern template class Mask<int32_t>;

extern template class Node<DefaultSource, Mask<int32_t>, MaskControl>;

using DefaultMaskNode = Node<DefaultSource, Mask<int32_t>, MaskControl>;


} // end namespace iris
