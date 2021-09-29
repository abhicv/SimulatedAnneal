#include "plot.h"
#include "types.h"

typedef struct FunctionData
{
    Function func;
    f32 xMin;
    f32 xMax;
    f32 yMin;
    f32 yMax;
    
} FunctionData;

f32 F1(f32 x)
{
    return sin(x) + sin((10.0/3.0)*x);
}

f32 F2(f32 x)
{
    return -(1.4 - 3.0 * x) * sin(18.0 * x);
}

f32 F3(f32 x)
{
    return -(x+sin(x))*exp(-x*x);
}

f32 F4(f32 x)
{
    return F1(x) + log(x) - (0.84*x) + 3;
}

f32 F5(f32 x)
{
    return -exp(-x)*sin(2 * 3.14 * x);
}

global FunctionData functionDatas[] = {
    [0] = {
        .func = F1,
        .xMin = 0.0f,
        .xMax = 9.0f,
        .yMin = -2.5f,
        .yMax = 2.5f,
    },
    [1] = {
        .func = F2,
        .xMin = 0.0f,
        .xMax = 1.2f,
        .yMin = -1.6f,
        .yMax = 2.5f,
    },
    [2] = {
        .func = F3,
        .xMin = -10.0f,
        .xMax = 10.0f,
        .yMin = -1.0f,
        .yMax = 1.0f,
    },
    [3] = {
        .func = F4,
        .xMin = 1.0f,
        .xMax = 7.5f,
        .yMin = -2.0f,
        .yMax = 4.0f,
    },
    [4] = {
        .func = F5,
        .xMin = 0.0f,
        .xMax = 4.0f,
        .yMin = -0.9f,
        .yMax = 0.6f,
    },
};