#pragma once

#include "scheme_type.h"
#include <Windows.h>
#include <string>
#include <vector>

struct KeyStroke
{
    UINT vk = 0;
    UINT modifiers_down = 0;
    WCHAR wch = 0;
};

struct QueryRequest
{
    SchemeType scheme = SchemeType::Quanpin;
    std::string raw_input;
    std::string normalized_input;
    std::string segmentation;
    std::vector<KeyStroke> key_strokes;
    bool valid = false;
};
