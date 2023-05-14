#pragma once


#include <string>
#include <png.h>
#include <tau/eigen.h>
#include <tau/planar.h>
#include <tau/color.h>
#include "iris/error.h"
#include "iris/size.h"


namespace iris
{


CREATE_EXCEPTION(PngError, IrisError);


template<typename Pixel>
using PlanarRgb =
    tau::Planar<3, Pixel, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

template<typename Pixel>
using PlanarHsv =
    tau::Planar<3, Pixel, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;


PlanarRgb<uint8_t> ReadPng(const std::string &fileName);


void WritePng(const PlanarRgb<uint8_t> &planarRgb, const std::string &fileName);


template<typename Pixel>
class Png
{
public:
    using Rgb = PlanarRgb<uint8_t>;
    using Hsv = PlanarHsv<float>;

    Png() = default;

    Png(const std::string &fileName)
    {
        this->rgb_ = ReadPng(fileName);
    }

    void Write(const std::string &fileName)
    {
        WritePng(*this->rgb_, fileName);
    }

    operator bool ()
    {
        return this->rgb_.has_value();
    }

    Rgb & GetRgb()
    {
        return *this->rgb_;
    }

    const Rgb & GetRgb() const
    {
        return *this->rgb_;
    }

    PlanarRgb<Pixel> GetRgbPixels() const
    {
        return this->rgb_->template Cast<Pixel>();
    }

    Hsv GetHsv() const
    {
        if (!this->rgb_)
        {
            throw std::runtime_error("No data");
        }

        return tau::RgbToHsv<float>(*this->rgb_);
    }

    Eigen::MatrixX<Pixel> GetValue(float scale) const
    {
        auto hsv = this->GetHsv();
        auto value = tau::GetValue(hsv);
        value.array() *= scale;
        return value.template cast<Pixel>();
    }

    Size GetSize() const
    {
        return this->rgb_->GetSize();
    }

    std::optional<Rgb> rgb_;
};



} // end namespace iris
