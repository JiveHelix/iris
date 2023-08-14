#include "iris/projection_settings.h"


namespace iris
{


template struct StereoProjectionModel<float>;
template struct StereoProjectionModel<double>;


} // end namespace iris


template struct pex::Group
    <
        iris::file::FileFields,
        iris::file::IntrinsicsTemplate<float>::template Template
    >;

template struct pex::Group
    <
        iris::file::FileFields,
        iris::file::IntrinsicsTemplate<double>::template Template
    >;

template struct pex::Group
    <
        iris::file::FileFields,
        iris::file::PoseTemplate<float>::template Template
    >;

template struct pex::Group
    <
        iris::file::FileFields,
        iris::file::PoseTemplate<double>::template Template
    >;

template struct pex::Group
    <
        iris::ProjectionFields,
        iris::ProjectionTemplate<float>::template Template
    >;

template struct pex::Group
    <
        iris::ProjectionFields,
        iris::ProjectionTemplate<double>::template Template
    >;


template struct pex::Group
    <
        iris::StereoProjectionFields,
        iris::StereoProjectionTemplate<float>::template Template
    >;

template struct pex::Group
    <
        iris::StereoProjectionFields,
        iris::StereoProjectionTemplate<double>::template Template
    >;
