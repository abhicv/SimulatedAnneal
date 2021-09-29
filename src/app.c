#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//for windowing, input and rendering
#include <SDL2/SDL.h>

//for windows file IO
#include <Windows.h>

//to set DPI awareness on windows
#include <shellscalingapi.h>

//types used
#include "types.h"

#include "font.c"
#include "render.c"
#include "plot.c"
#include "MicroUI.c"
#include "functions.c"

#define APP_NAME "Simulated Annealing\0"
#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 900

#define MUI_ORIGIN_ID 2021
#include "ui_style.h"
global MUI ui;
global MUI_Input uiInput;
global f32 coolRate = 0.01f;
global u32 index = 0;

f32 ObjectiveFunction(f32 x)
{
    return sin(x) + sin((10.0/3.0)*x);
}

f32 Temperature(f32 x)
{
    return 30.0f*exp(-x*coolRate);
}

f32 HillClimb(f32 x, f32 stepSize)
{
    f32 currentY = ObjectiveFunction(x);
    
    f32 yLeft = ObjectiveFunction(x - stepSize);
    f32 yRight = ObjectiveFunction(x + stepSize);
    
    if(yLeft > currentY)
    {
        x -= stepSize;
    }
    else if(yRight > currentY)
    {
        x += stepSize;
    }
    
    return x;
}

f32 RandomF32(f32 min, f32 max, f32 resolution)
{
    i32 minInt = min * resolution;
    i32 maxInt = max * resolution;
    i32 r = (rand() % (maxInt - minInt + 1)) + minInt;
    f32 random = r / resolution;
    return random;
}

typedef struct SimulationData
{
    Function objFunc;
    Function tempFunc;
    
    f32 x;
    f32 xMin;
    f32 xMax;
    
    f32 temp;
    f32 probability;
    u32 iteration;
    
} SimulationData;

void StepSimulation(SimulationData *data)
{
    //get new neightbouring value
    f32 xNew = RandomF32(data->xMin, data->xMax, 1000.0f);
    
    f32 newCost = 0.0f;
    f32 oldCost = 0.0f;
    
    if(data->objFunc != NULL)
    {
        newCost = data->objFunc(xNew);
        oldCost = data->objFunc(data->x);
    }
    else
    {
        printf("Missing objective function!\n");
        return;
    }
    
    f32 cost_diff = newCost - oldCost;
    f32 probability = 1.0f;
    f32 random = RandomF32(0.0f, 1.0f, 1000.0f);
    f32 temp = 0.0f;
    
    if(data->tempFunc != NULL)
    {
        temp = data->tempFunc(data->iteration);
        data->temp = temp;
    }
    else
    {
        printf("Missing temperature function!\n");
        return;
    }
    
    if(newCost > oldCost)
    {
        //probability of rejection
        probability = exp(-cost_diff / temp);
        data->probability = probability;
        
        if(random < probability)
        {
            data->x = xNew;
        }
    }
    else
    {
        data->x = xNew;
    }
    
    data->iteration++;
    
    printf("%d > x: %0.5f, x_new: %0.4f, r : %0.4f, prob: %0.4f, temp: %0.4f, y: %0.5f\n", data->iteration, data->x, xNew, random, probability, temp, data->objFunc(data->x));
    
    return;
}

int main(int argc, char **argv)
{
    SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
    
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        SDL_Log("Failed to initialize SDL : %s\n", SDL_GetError());
        return 1;
    }
    
    SDL_Window *window = SDL_CreateWindow(APP_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
    
    if(window == NULL)
    {
        SDL_Log("Failed to create SDL window : %s\n", SDL_GetError());
        return 1;
    }
    
    SDL_Renderer *renderer = SDL_CreateRenderer(window, 3, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    
    if(renderer == NULL)
    {
        SDL_Log("Failed to create SDL renderer : %s\n", SDL_GetError());
        return 1;
    }
    
    SDL_Rect displayBounRect = {0};
    SDL_GetDisplayBounds(SDL_GetWindowDisplayIndex(window), &displayBounRect);
    
    //printf("w: %d, h: %d\n", displayBounRect.w, displayBounRect.h);
    
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    
    if(texture == NULL)
    {
        SDL_Log("Failed to create SDL texture : %s\n", SDL_GetError());
        return 1;
    }
    
    FontData *fontData = LoadFont("font/Inconsolata.ttf", 22.0f);
    FontData *fontDataPlot = LoadFont("font/Inconsolata.ttf", 16.0f);
    
    if(fontData == NULL)
    {
        printf("Failed to load font file!!\n");
    }
    
    Buffer renderBuffer = {0};
    renderBuffer.data = (u32*)malloc(sizeof(u32) * displayBounRect.w * displayBounRect.h);
    renderBuffer.width = SCREEN_WIDTH;
    renderBuffer.height = SCREEN_HEIGHT;
    
    PlotStatic tempPlot = {
        .xMin = 0,
        .xMax = 1000.0f,
        .yMin = 0,
        .yMax = 50.0f,
        .title = "temperature\0"
    };
    
    PlotStatic probPlot = {
        .xMin = 0,
        .xMax = 1000.0f,
        .yMin = 0,
        .yMax = 1.0f,
        .title = "probability\0"
    };
    
    PlotStatic objFuncPlot = {
        .dataLength = 500.0f,
        .xMin = functionDatas[index].xMin,
        .xMax = functionDatas[index].xMax,
        .yMin = functionDatas[index].yMin,
        .yMax = functionDatas[index].yMax,
        .title = "objective function\0"
    };
    
    PlotFunction(&objFuncPlot, functionDatas[index].func);
    
    SimulationData data = {0};
    data.x = RandomF32(functionDatas[index].xMin, functionDatas[index].xMax, 100.0f);
    data.xMin = functionDatas[index].xMin;
    data.xMax = functionDatas[index].xMax;
    data.objFunc = functionDatas[index].func;
    data.tempFunc = Temperature;
    data.iteration = 0;
    
    //text edit box data
    coolRate = 0.01f;
    TextEdit textEdit = {0};
    textEdit.text[0] = '0';
    textEdit.text[1] = '.';
    textEdit.text[2] = '0';
    textEdit.text[3] = '1';
    textEdit.cursorPos = 4;
    
    b32 quitApp = false;
    SDL_Event event = {0};
    
    bool run = false;
    
    while(!quitApp)
    {
        uiInput.bTextInput = false;
        uiInput.backSpaceDown = false;
        
        while(SDL_PollEvent(&event))
        {
            MUI_GetInput(&uiInput, &event);
            
            switch(event.type)
            {
                case SDL_QUIT:
                quitApp = true;
                break;
                
                case SDL_KEYDOWN:
                if(event.key.keysym.sym == SDLK_ESCAPE)
                {
                    quitApp = true;
                }
                else if(event.key.keysym.sym == SDLK_SPACE)
                {
                    run = false;
                    u32 count = sizeof(functionDatas) / sizeof(functionDatas[0]);
                    
                    index++;
                    
                    if(index > count - 1)
                    {
                        index = 0;
                    }
                    
                    objFuncPlot.dataLength = 500.0f,
                    objFuncPlot.xMin = functionDatas[index].xMin,
                    objFuncPlot.xMax = functionDatas[index].xMax,
                    objFuncPlot.yMin = functionDatas[index].yMin,
                    objFuncPlot.yMax = functionDatas[index].yMax,
                    
                    PlotFunction(&objFuncPlot, functionDatas[index].func);
                    
                    data.x = RandomF32(functionDatas[index].xMin, functionDatas[index].xMax, 100.0f);
                    data.xMin = functionDatas[index].xMin;
                    data.xMax = functionDatas[index].xMax;
                    data.objFunc = functionDatas[index].func;
                    data.iteration = 0;
                    
                    tempPlot.dataLength = 0;
                    probPlot.dataLength = 0;
                }
                break;
                
                case SDL_WINDOWEVENT:
                if((event.window.event == SDL_WINDOWEVENT_RESIZED) || (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED))
                {
                    u32 w = 0, h = 0;
                    SDL_GetWindowSize(window, &w, &h);
                    //printf("Window size changed to width : %d, height : %d\n", w, h);
                    
                    //resizing display buffer and SDL_Texture to fit to new window dimensions
                    SDL_DestroyTexture(texture);
                    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, w, h);
                    
                    renderBuffer.width = w;
                    renderBuffer.height = h;
                }
                break;
            }
        }
        
        if(run)
        {
            StepSimulation(&data);
            PlotPushDataPoint(&tempPlot, data.iteration, data.temp);
            PlotPushDataPoint(&probPlot, data.iteration, data.probability);
            
            // NOTE(abhicv): simulaion stop citeria
            if((data.temp - 0.0f) < 0.001f)
            {
                run = false;
            }
            //SDL_Delay(20);
        }
        
        MUI_BeginFrame(&ui, &uiInput);
        
        Rect panelRect = {
            .x = 5,
            .y = 5,
            .width = 0.2 * (renderBuffer.width - 15),
            .height = 0.5 * (renderBuffer.height - 15),
        };
        
        Rect objFuncPlotRect = {
            .x = panelRect.x + panelRect.width + 5,
            .y = panelRect.y,
            .width = 0.8 * (renderBuffer.width - 15),
            .height = panelRect.height,
        };
        
        Rect tempPlotRect = {
            .x = panelRect.x,
            .y = panelRect.y + panelRect.height + 5,
            .width = 0.5 * (renderBuffer.width - 15),
            .height = 0.5 * (renderBuffer.height - 15),
        };
        
        Rect probPlotRect = {
            .x = tempPlotRect.x + tempPlotRect.width + 5,
            .y = panelRect.y + panelRect.height + 5,
            .width = 0.5 * (renderBuffer.width - 15),
            .height = 0.5 * (renderBuffer.height - 15),
        };
        
        MUI_Rect uiRect = {0};
        uiRect.x = panelRect.x + 10;
        uiRect.y = panelRect.y + 10;
        uiRect.width = panelRect.width - 20;
        uiRect.height = 30;
        
        MUI_PushColumnLayout(&ui, uiRect, 5);
        {
            //MUI_TextA(&ui, GEN_MUI_ID(), "Simulated Annealing !!\0", 20, textStyle);
            
            if(MUI_ButtonA(&ui, GEN_MUI_ID(), "run", buttonStyle))
            {
                run = true;
            }
            
            if(MUI_ButtonA(&ui, GEN_MUI_ID(), "step", buttonStyle))
            {
                StepSimulation(&data);
                PlotPushDataPoint(&tempPlot, data.iteration, data.temp);
                PlotPushDataPoint(&probPlot, data.iteration, data.probability);
            }
            
            if(MUI_ButtonA(&ui, GEN_MUI_ID(), "stop", buttonStyle))
            {
                run = false;
            }
            
            if(MUI_ButtonA(&ui, GEN_MUI_ID(), "reset", buttonStyle))
            {
                data.iteration = 0;
                tempPlot.dataLength = 0;
                probPlot.dataLength = 0;
            }
            
            char iterString[256];
            sprintf(iterString, "Iteration:%d\0", data.iteration);
            MUI_TextA(&ui, GEN_MUI_ID(), iterString, 20, textStyle);
            
            //coolRate = MUI_SliderA(&ui, GEN_MUI_ID(), v, sliderStyle);
            
            MUI_TextA(&ui, GEN_MUI_ID(), "Cooling Rate:", 20, textStyle);
            
            sscanf(textEdit.text, "%f", &coolRate);
            MUI_TextEditA(&ui, GEN_MUI_ID(), buttonStyle, &textEdit);
        }
        MUI_PopLayout(&ui);
        
        // NOTE(abhicv): RENDERING
        //ClearBuffer(&renderBuffer, (Color){4, 35, 40, 255});
        ClearBuffer(&renderBuffer, (Color){18, 18, 18, 255});
        
        //temp function plot
        tempPlot.rect = tempPlotRect;
        DrawPlotStatic(&renderBuffer, fontDataPlot, &tempPlot, (Color){0, 255, 0, 255});
        DrawRectWire(&renderBuffer, &tempPlotRect, (Color){255, 255, 255, 255});
        
        //probability plot
        probPlot.rect = probPlotRect;
        DrawPlotStatic(&renderBuffer, fontDataPlot, &probPlot, (Color){255, 255, 0, 255});
        DrawRectWire(&renderBuffer, &probPlotRect, (Color){255, 255, 255, 255});
        
        // obj function plot
        objFuncPlot.rect = objFuncPlotRect;
        DrawPlotStatic(&renderBuffer, fontDataPlot, &objFuncPlot, (Color){255, 255, 255, 255});
        PlotAnnotate(&renderBuffer, &objFuncPlot, data.x, data.objFunc(data.x));
        DrawRectWire(&renderBuffer, &objFuncPlotRect, (Color){255, 255, 255, 255});
        
        //ui panel
        DrawRectWire(&renderBuffer, &panelRect, (Color){255, 255, 255, 255});
        MUI_EndFrame(&ui, &renderBuffer, fontData);
        
        SDL_UpdateTexture(texture, NULL, renderBuffer.data, 4 * renderBuffer.width);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }
    
    SDL_Quit();
    return 0;
}
