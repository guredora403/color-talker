#pragma once

/*
    RGBと色表現が録音されたファイル名の対応表
*/

typedef struct _colorVoice {
    unsigned int r;
    unsigned int g;
    unsigned int b;
    char* fileName;
} ColorVoice;

//参考: https://note.cman.jp/color/base_color.cgi
ColorVoice colorVoices[] = {
    {0, 0, 0, "/blac.wav"},
    {0, 0, 255, "/blue.wav"},
    {255, 0, 0, "/red.wav"},
    {0, 255, 0, "/lime_green.wav"},
    {0, 128, 0, "/green.wav"},
    {0, 255, 255, "/aqua.wav"},
    {128, 128, 128, "/grey.wav"},
    {192, 192, 192, "/silver.wav"},
    {255, 255, 255, "/white.wav"}
};

