#include <iostream>
#include <iris/level_settings.h>
#include <pex/traits.h>

using LevelSettings = iris::LevelSettings<int>;

int main()
{
    LevelSettings settings = LevelSettings{};
    std::cout << settings << std::endl;

    return 0;
}
