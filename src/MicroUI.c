#include "MicroUI.h"

MUI_Id MUI_IdInit(u32 primary, u32 secondary)
{
    MUI_Id id = {primary, secondary};
    return id;
}

bool MUI_IdEqual(MUI_Id a, MUI_Id b)
{
    return (a.primary == b.primary && a.secondary == b.secondary);
}

MUI_Id MUI_NullId(void)
{
    MUI_Id id = {0, 0};
    return id;
}

void MUI_BeginFrame(MUI *ui, MUI_Input *input)
{
    ui->widgetCount = 0;
    
    if (input != NULL)
    {
        ui->leftMouseButtonDown = input->leftMouseButtonDown;
        ui->rightMouseButtonDown = input->rightMouseButtonDown;
        ui->mouseX = input->mouseX;
        ui->mouseY = input->mouseY;
        ui->textInputChar = input->textInputChar;
        ui->bTextInput = input->bTextInput;
        ui->backSpaceDown = input->backSpaceDown;
    }
}

void MUI_EndFrame(MUI *ui, Buffer *buffer, FontData *fontData)
{
    Color color = {255, 255, 255, 255};
    Color textColor = {255, 255, 255, 255};
    
    bool highlighted = false;
    
    for (u32 i = 0; i < ui->widgetCount; i++)
    {
        switch (ui->widgets[i].widgetType)
        {
			case MUI_WIDGET_BUTTON:
            {
                highlighted = MUI_IdEqual(ui->hotWidgetId, ui->widgets[i].id);
                color = highlighted ? ui->widgets[i].style.buttonStyle.highlightColor : ui->widgets[i].style.buttonStyle.idleColor;
                
                MUI_Rect rect = ui->widgets[i].rect;
                
                DrawRectWire(buffer, &rect, color);
                
                if(highlighted)
                {
                    Color borderColor = {255, 255, 255, 255};
                    DrawRectWire(buffer, &rect, borderColor);
                }
            }
            break;
            
            case MUI_WIDGET_SLIDER:
            {
                //bg rect
                MUI_Rect rect = ui->widgets[i].rect;
                
                color = ui->widgets[i].style.sliderStyle.bgColor;
                DrawRectWire(buffer, &rect, color);
                
                //sliding rect
                MUI_Rect slideRect = rect;
                slideRect.width = (f32)rect.width * ui->widgets[i].slider.value;
                color = ui->widgets[i].style.sliderStyle.sliderColor;
                DrawRect(buffer, &slideRect, color);
            }
            break;
            
            case MUI_WIDGET_TEXTEDIT:
            {
                MUI_Rect rect = ui->widgets[i].rect;
                
                highlighted = MUI_IdEqual(ui->activeWidgetId, ui->widgets[i].id);
                
                color = highlighted ? ui->widgets[i].style.buttonStyle.highlightColor : ui->widgets[i].style.buttonStyle.idleColor;
                
                DrawRectWire(buffer, &rect, color);
                
                if(highlighted)
                {
                    Color borderColor = {255, 255, 255, 255};
                    DrawRectWire(buffer, &rect, borderColor);
                }
                
                //text
                i32 textX = rect.x + 4;
                i32 textY = (rect.height - fontData->lineHeight) / 2;
                textY = textY + rect.y;
                
                char *text = ui->widgets[i].textInput.textEdit.text;
                
                if (text != NULL)
                {
                    RenderText(buffer, text, strlen(text), fontData, textX, textY, (Color){255, 255, 255, 255}, rect);
                }
                
                //cursor
                if(highlighted)
                {
                    Rect caret = {0};
                    caret.x = textX + (fontData->charDatas['A'].xadvance * strlen(text));
                    caret.y = textY;
                    caret.width = fontData->charDatas['A'].xadvance;
                    caret.height = fontData->lineHeight;
                    
                    if((caret.x +caret.width) <= (rect.x + rect.width))
                    {
                        DrawRect(buffer, &caret, (Color){255, 255, 255, 255});
                    }
                }
            }
            break;
            
            case MUI_WIDGET_TEXT:
            {
                textColor = ui->widgets[i].style.textStyle.textColor;
                char *text = ui->widgets[i].text.text;
                
                //DrawRectWire(buffer, &ui->widgets[i].rect, textColor);
                
                u32 textWidth = strlen(text) *  fontData->charDatas['A'].xadvance;
                u32 textHeight = fontData->lineHeight;
                
                i32 x = (ui->widgets[i].rect.width - textWidth) / 2;
                x = ui->widgets[i].rect.x + x;
                
                i32 y = (ui->widgets[i].rect.height - textHeight) / 2;
                y = ui->widgets[i].rect.y + y;
                
                Rect clipRect = {.x = 0, .y = 0, .width = buffer->width, .height = buffer->height};
                
                if (text != NULL)
                {
                    RenderText(buffer, text, strlen(text), fontData, x, y, textColor, clipRect);
                }
            }
            break;
            
            default:
            break;
        }
    }
}

void MUI_Text(MUI *ui, MUI_Id id, MUI_Rect rect, char *text, u32 fontSize, MUI_Style style)
{
    if(ui->widgetCount < MUI_MAX_WIDGETS)
    {
        MUI_Widget *widget = ui->widgets + ui->widgetCount++;
        widget->widgetType = MUI_WIDGET_TEXT;
        widget->id = id;
        widget->text.text = text;
        widget->text.fontSize = fontSize;
        widget->rect = rect;
        widget->style = style;
    }
    else
    {
		printf("UI widget count out of bound\n");
    }
}

void MUI_TextA(MUI *ui, MUI_Id id, char *text, u32 fontSize, MUI_Style style)
{
    MUI_Text(ui, id, MUI_GetNextAutoLayoutRect(ui), text, fontSize, style);
}

bool MUI_Button(MUI *ui, MUI_Id id, char *text, MUI_Rect rect, MUI_Style style)
{
    bool isTriggered = false;
    
    u32 left = rect.x;
    u32 right = rect.x + rect.width;
    u32 top = rect.y;
    u32 bottom = rect.y + rect.height;
    
    bool isMouseOver = (ui->mouseX > left &&
                        ui->mouseX < right &&
                        ui->mouseY > top &&
                        ui->mouseY < bottom);
    
    if (!MUI_IdEqual(id, ui->hotWidgetId) && isMouseOver)
    {
        ui->hotWidgetId = id;
    }
    else if (MUI_IdEqual(id, ui->hotWidgetId) && !isMouseOver)
    {
        ui->hotWidgetId = MUI_NullId();
    }
    
    if (MUI_IdEqual(id, ui->activeWidgetId))
    {
        if (!ui->leftMouseButtonDown)
        {
            if (MUI_IdEqual(id, ui->hotWidgetId))
            {
                isTriggered = true;
                ui->activeWidgetId = MUI_NullId();
            }
        }
    }
    else
    {
        if (MUI_IdEqual(id, ui->hotWidgetId))
        {
            if (ui->leftMouseButtonDown)
            {
                ui->activeWidgetId = id;
            }
        }
    }
    
    if(ui->widgetCount < MUI_MAX_WIDGETS)
    {
        MUI_Widget *widget = ui->widgets + ui->widgetCount++;
        widget->id = id;
        widget->widgetType = MUI_WIDGET_BUTTON;
        widget->rect = rect;
        widget->style = style;
        
        MUI_Style textStyle = {
            .textStyle = {
                .textColor = style.buttonStyle.textColor,
            },
        };
        
        MUI_Text(ui, MUI_IdInit(id.primary - 10, id.secondary + 10), rect, text, style.buttonStyle.fontSize, textStyle);
        
    }
    else
    {
        printf("MUI widget count out of bound\n");
    }
    
    return isTriggered;
}

bool MUI_ButtonA(MUI *ui, MUI_Id id, char *text, MUI_Style style)
{
    return MUI_Button(ui, id, text, MUI_GetNextAutoLayoutRect(ui), style);
}

f32 MUI_Slider(MUI *ui, MUI_Id id, f32 value, MUI_Rect rect, MUI_Style style)
{
    u32 left = rect.x;
    u32 right = rect.x + rect.width;
    u32 top = rect.y;
    u32 bottom = rect.y + rect.height;
    
    bool isMouseOver = (ui->mouseX > left && ui->mouseX < right && ui->mouseY > top && ui->mouseY < bottom);
    
    if (!MUI_IdEqual(id, ui->hotWidgetId) && isMouseOver)
    {
        ui->hotWidgetId = id;
    }
    else if (MUI_IdEqual(id, ui->hotWidgetId) && !isMouseOver)
    {
        ui->hotWidgetId = MUI_NullId();
    }
    
    if (!MUI_IdEqual(id, ui->activeWidgetId))
    {
        if (MUI_IdEqual(id, ui->hotWidgetId))
        {
            if (ui->leftMouseButtonDown)
            {
                ui->activeWidgetId = id;
            }
        }
    }
    else
    {
        if (ui->leftMouseButtonDown)
        {
            value = ((f32)ui->mouseX - (f32)left) / (f32)rect.width;
        }
        else
        {
            ui->activeWidgetId = MUI_NullId();
        }
    }
    
    if (value <= 0.0f)
    {
        value = 0.0f;
    }
    else if (value > 1.0f)
    {
        value = 1.0f;
    }
    
    if(ui->widgetCount < MUI_MAX_WIDGETS)
    {
        MUI_Widget *widget = ui->widgets + ui->widgetCount++;
        widget->id = id;
        widget->widgetType = MUI_WIDGET_SLIDER;
        widget->rect = rect;
        widget->slider.value = value;
        widget->style = style;
        
        //value text
        MUI_Style textStyle = {
            .textStyle = {
                .textColor = {255, 255, 255, 255},
            },
        };
        
        sprintf(widget->slider.valueStr, "%0.3f\0", value);
        MUI_Text(ui, MUI_IdInit(id.primary - 5, id.secondary + 5), rect, widget->slider.valueStr, 20, textStyle);
        
    }
    else
    {
        printf("UI widget count out of bound\n");
    }
    
    
    return value;
}

f32 MUI_SliderA(MUI *ui, MUI_Id id, f32 value, MUI_Style style)
{
    return MUI_Slider(ui, id, value, MUI_GetNextAutoLayoutRect(ui), style);
}

void MUI_TextEdit(MUI *ui, MUI_Id id, MUI_Rect rect, MUI_Style style, TextEdit *textEdit)
{
    u32 left = rect.x;
    u32 right = rect.x + rect.width;
    u32 top = rect.y;
    u32 bottom = rect.y + rect.height;
    
    bool isMouseOver = (ui->mouseX > left && ui->mouseX < right &&
                        ui->mouseY > top && ui->mouseY < bottom);
    
    if (!MUI_IdEqual(id, ui->hotWidgetId) && isMouseOver)
    {
        ui->hotWidgetId = id;
    }
    else if (MUI_IdEqual(id, ui->hotWidgetId) && !isMouseOver && ui->leftMouseButtonDown)
    {
        ui->hotWidgetId = MUI_NullId();
        ui->activeWidgetId = MUI_NullId();
    }
    
    if (MUI_IdEqual(id, ui->activeWidgetId))
    {
        if (MUI_IdEqual(id, ui->hotWidgetId))
        {
            if(ui->bTextInput)
            {
                if(textEdit->cursorPos < MAX_TEXTEDIT_SIZE)
                {
                    textEdit->text[textEdit->cursorPos] = ui->textInputChar;
                    textEdit->cursorPos++;
                }
            }
            
            if(ui->backSpaceDown && textEdit->cursorPos > 0)
            {
                textEdit->cursorPos--;
                textEdit->text[textEdit->cursorPos] = 0;
            }
        }
    }
    else
    {
        if (MUI_IdEqual(id, ui->hotWidgetId))
        {
            if (ui->leftMouseButtonDown)
            {
                ui->activeWidgetId = id;
            }
        }
    }
    
    if(ui->widgetCount < MUI_MAX_WIDGETS)
    {
        MUI_Widget *widget = ui->widgets + ui->widgetCount++;
        widget->id = id;
        widget->widgetType = MUI_WIDGET_TEXTEDIT;
        widget->rect = rect;
        widget->style = style;
        widget->textInput.textEdit = *textEdit;
    }
    else
    {
        printf("UI widget count out of bound\n");
    }
    
}

void MUI_TextEditA(MUI *ui, MUI_Id id, MUI_Style style, TextEdit *textEdit)
{
    MUI_TextEdit(ui, id, MUI_GetNextAutoLayoutRect(ui), style, textEdit);
}

void MUI_PushColumnLayout(MUI *ui, MUI_Rect rect, u32 offset)
{
    u32 i = ui->autoLayOutIndex++;
    ui->autoLayOutGroup[i].rect = rect;
    ui->autoLayOutGroup[i].progress = 0;
    ui->autoLayOutGroup[i].offset = offset;
    ui->autoLayOutGroup[i].isColumn = true;
}

void MUI_PushRowLayout(MUI *ui, MUI_Rect rect, u32 offset)
{
    u32 i = ui->autoLayOutIndex++;
    ui->autoLayOutGroup[i].rect = rect;
    ui->autoLayOutGroup[i].progress = 0;
    ui->autoLayOutGroup[i].offset = offset;
    ui->autoLayOutGroup[i].isColumn = false;
}

void MUI_PopLayout(MUI *ui)
{
    if (ui->autoLayOutIndex > 0)
    {
        ui->autoLayOutIndex--;
    }
}

MUI_Rect MUI_GetNextAutoLayoutRect(MUI *ui)
{
    MUI_Rect rect = {0};
    
    if (ui->autoLayOutIndex > 0)
    {
        u32 i = ui->autoLayOutIndex - 1;
        
        if (ui->autoLayOutGroup[i].isColumn)
        {
            rect = ui->autoLayOutGroup[i].rect;
            rect.y = ui->autoLayOutGroup[i].progress + rect.y;
            ui->autoLayOutGroup[i].progress += rect.height + ui->autoLayOutGroup[i].offset;
        }
        else
        {
            rect = ui->autoLayOutGroup[i].rect;
            rect.x = ui->autoLayOutGroup[i].progress + rect.x;
            ui->autoLayOutGroup[i].progress += rect.width + ui->autoLayOutGroup[i].offset;
        }
    }
    
    return rect;
}

void MUI_GetInput(MUI_Input *uiInput, SDL_Event *event)
{
    uiInput->mouseX = event->motion.x;
    uiInput->mouseY = event->motion.y;
    
    uiInput->bTextInput = false;
    
    SDL_StartTextInput();
    
    switch(event->type)
    {
        case SDL_MOUSEBUTTONDOWN:
        if (event->button.button == SDL_BUTTON_LEFT)
        {
            uiInput->leftMouseButtonDown = true;
        }
        else if (event->button.button == SDL_BUTTON_RIGHT)
        {
            uiInput->rightMouseButtonDown = true;
        }
        break;
        
        case SDL_MOUSEBUTTONUP:
        if(event->button.button == SDL_BUTTON_LEFT)
        {
            uiInput->leftMouseButtonDown = false;
        }
        else if(event->button.button == SDL_BUTTON_RIGHT)
        {
            uiInput->rightMouseButtonDown = false;
        }
        break;
        
        case SDL_KEYDOWN:
        if (event->key.keysym.sym == SDLK_BACKSPACE)
        {
            uiInput->backSpaceDown = true;
        }
        break;
        
        case SDL_KEYUP:
        if (event->key.keysym.sym == SDLK_BACKSPACE)
        {
            uiInput->backSpaceDown = false;
        }
        break;
        
        case SDL_TEXTINPUT:
        uiInput->textInputChar = event->text.text[0];
        uiInput->bTextInput = true;
        break;
    }
}

