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

    tau::Margins ComputeRequiredMargins() const
    {
        return {0, 0};
    }

    bool Filter(Eigen::Ref<const Input> input, Eigen::Ref<Result> result)
    {
        if (!this->maskSettings_.enable)
        {
            return false;
        }

        auto &imageSize = this->maskSettings_.imageSize;

        if (imageSize.height != input.rows() || imageSize.width != input.cols())
        {
            imageSize = draw::GetMatrixSize(input);
            this->mask_.reset();
        }

        if (!this->mask_)
        {
            this->mask_ = CreateMask(this->maskSettings_);
        }

        assert(result.rows() == input.rows());
        assert(result.cols() == input.cols());
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
