#pragma once

#include <cfloat>

struct ObjectVisibilityData
{
    bool isVisible = false;
    float distance = FLT_MAX;
    // Можно добавить дополнительные данные: угол, нормаль и т.д.
};
