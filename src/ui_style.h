#ifndef UI_STYLE_H
#define UI_STYLE_H

MUI_Style buttonStyle = {
    .buttonStyle = {
        .idleColor = {51, 153, 255, 255},
        .highlightColor = {200, 140, 10, 255},
        .textColor = {51, 153, 255, 255},
        .fontSize = 20,
    },
};

MUI_Style sliderStyle = {
    .sliderStyle = {
        .bgColor = {255, 255, 255, 255},
        .sliderColor = {255, 0, 0, 255},
    }
};

MUI_Style textStyle = {
    .textStyle = {
        .textColor = {255, 255, 255, 255},
    }
};

#endif //UI_STYLE_H
