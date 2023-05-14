#include <iostream>
#include <iris/level_settings.h>
#include <pex/traits.h>

using LevelSettings = iris::LevelSettings<int>;
static_assert(pex::HasDefault<LevelSettings>);

int main()
{
    LevelSettings settings = LevelSettings::Default();
    std::cout << settings << std::endl;

    return 0;
}
