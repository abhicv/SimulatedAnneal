#include "plot.h"

void PlotFunction(PlotStatic *plot, Function func)
{
    f32 step = (plot->xMax - plot->xMin) / plot->dataLength;
    
    for(int n = 0; n < plot->dataLength; n++)
    {
        plot->xData[n] = plot->xMin + (n * step);
        plot->yData[n] = func(plot->xData[n]);
    }
    
    return;
    
}

void PlotPushDataPoint(PlotStatic *plot, f32 x, f32 y)
{
    if(plot->dataLength < PLOT_POINT_LIMIT)
    {
        u32 index = plot->dataLength;
        plot->xData[index] = x;
        plot->yData[index] = y;
        plot->dataLength++;
    }
    return;
}

void PlotAnnotate(Buffer *buffer, PlotStatic *plot, f32 xValue, f32 yValue)
{
    if(xValue >= plot->xMin && xValue <= plot->xMax && yValue >= plot->yMin && yValue <= plot->yMax)
    {
        Rect drawRect = plot->rect;
        drawRect.width -= (0.2 * plot->rect.height);
        drawRect.height -= (0.2 * plot->rect.height);
        drawRect.x += (0.2 * plot->rect.height) / 2;
        drawRect.y += (0.2 * plot->rect.height) / 2;
        
        i32 yLength = drawRect.height;
        i32 xLength = drawRect.width;
        
        f32 yStep =  yLength / (plot->yMax - plot->yMin); //value of one pixel in y axis
        f32 xStep = xLength / (plot->xMax - plot->xMin);  //value of one pixel in x axis
        
        i32 y = (yValue - plot->yMin) * yStep;
        i32 x =(xValue - plot->xMin) * xStep;
        
        y = drawRect.y + drawRect.height - y;
        x = drawRect.x + x;
        
        if(x >= drawRect.x && y >= drawRect.y && x <= (drawRect.x + drawRect.width) && y <= (drawRect.y + drawRect.height))
        {
            DrawCircle(buffer, x, y, 12, (Color){255, 0, 0, 255});
        }
    }
}

void DrawPlotStatic(Buffer *buffer, FontData *fontData, PlotStatic *plot, Color color)
{
    Rect drawRect = plot->rect;
    drawRect.width -= (0.2 * plot->rect.height);
    drawRect.height -= (0.2 * plot->rect.height);
    drawRect.x += (0.2 * plot->rect.height) / 2;
    drawRect.y += (0.2 * plot->rect.height) / 2;
    
    DrawRectWire(buffer, &drawRect, (Color){255, 255, 255, 255});
    
    i32 yLength = drawRect.height;
    i32 xLength = drawRect.width;
    
    f32 yStep =  yLength / (plot->yMax - plot->yMin); //value of one pixel in y axis
    f32 xStep = xLength / (plot->xMax - plot->xMin);  //value of one pixel in x axis
    
    //plotting data points
    for(int n = 0; n < plot->dataLength; n++)
    {
        i32 y = (plot->yData[n] - plot->yMin) * yStep;
        i32 x =(plot->xData[n] - plot->xMin) * xStep;
        
        y = drawRect.y + drawRect.height - y;
        x = drawRect.x + x;
        
        if(x >= drawRect.x && y >= drawRect.y && x <= (drawRect.x + drawRect.width) && y <= (drawRect.y + drawRect.height))
        {
            DrawCircle(buffer, x, y, 2, color);
        }
    }
    
    f32 xMark = (plot->xMax - plot->xMin) / 5.0f;
    f32 yMark = (plot->yMax - plot->yMin) / 5.0f;
    
    f32 xValue = plot->xMin;
    f32 yValue = plot->yMin;
    
    char str[25];
    
    // marking x axis
    for(int n = 0; n <= 5; n++)
    {
        sprintf(str, "%0.2f\0", xValue);
        xValue += xMark;
        
        i32 x = drawRect.x + (n * (xLength) / 5);
        i32 y = drawRect.y + drawRect.height + 4;
        
        RenderText(buffer, str, strlen(str), fontData, x, y, (Color){255, 255, 255 ,255}, plot->rect);
    }
    
    // marking y axis
    for(int n = 0; n <= 5; n++)
    {
        sprintf(str, "%0.2f\0", yValue);
        yValue += yMark;
        
        i32 x = drawRect.x - (fontData->charDatas['0'].xadvance * strlen(str)) - 2;
        i32 y = drawRect.y + drawRect.height - (n * (yLength / 5)) - fontData->lineHeight;
        
        RenderText(buffer, str, strlen(str), fontData, x, y, (Color){255, 255, 255 ,255}, plot->rect);
    }
    
    //plot title
    if(plot->title != NULL)
    {
        u32 len = strlen(plot->title);
        u32 width = len * fontData->charDatas['A'].xadvance;
        
        i32 x = plot->rect.x + (plot->rect.width / 2) - (width / 2);
        i32 y = plot->rect.y + 20;
        
        RenderText(buffer, plot->title, len, fontData, x, y, (Color){255, 255, 255 ,255}, plot->rect);
    }
    
    return;
    
}