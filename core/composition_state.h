#pragma once

#include "query_request.h"
#include "word_item.h"
#include <string>
#include <vector>

struct CompositionState
{
    std::string preedit;
    QueryRequest request;
    std::vector<WordItem> candidates;
};
