#include "iris/gaussian_settings.h"


namespace iris
{


std::map<Partials, std::string_view> partialsStringsById{
    {Partials::none, "none"},
    {Partials::rows, "rows"},
    {Partials::columns, "columns"},
    {Partials::both, "both"}};


std::unordered_map<std::string_view, Partials> GetPartialsByString()
{
    std::unordered_map<std::string_view, Partials> result;

    for (auto [key, value]: partialsStringsById)
    {
        result[value] = key;
    }

    return result;
}


std::string ToString(Partials partials)
{
    return std::string(partialsStringsById.at(partials));
}


Partials ToValue(fields::Tag<Partials>, std::string_view asString)
{
    static const auto partialsByString = GetPartialsByString();

    return partialsByString.at(asString);
}


std::string PartialsConverter::ToString(Partials partials)
{
    return iris::ToString(partials);
}


Partials PartialsConverter::ToValue(const std::string &asString)
{
    return ::iris::ToValue(fields::Tag<Partials>{}, asString);
}


std::vector<Partials> PartialsChoices::GetChoices()
{
    return {
        Partials::none,
        Partials::rows,
        Partials::columns,
        Partials::both};
}


std::ostream & operator<<(std::ostream &outputStream, Partials partials)
{
    return outputStream << iris::ToString(partials);
}


template struct GaussianSettings<int32_t>;


} // end namespace iris


template struct pex::Group
<
    iris::GaussianFields,
    iris::GaussianTemplate<int32_t>::template Template,
    pex::PlainT<iris::GaussianSettings<int32_t>>
>;
