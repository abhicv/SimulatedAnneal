#ifndef PLOT_H
#define PLOT_H

#include "render.h"

#define PLOT_POINT_LIMIT 5000

typedef f32 (*Function)(f32);

typedef struct PlotStatic
{
    Rect rect;
    f32 xData[PLOT_POINT_LIMIT];
    f32 yData[PLOT_POINT_LIMIT];
    
    u32 dataLength;
    
    f32 xMin, xMax;
    f32 yMin, yMax;
    
    char *title;
    
} PlotStatic;

#endif //PLOT_H
