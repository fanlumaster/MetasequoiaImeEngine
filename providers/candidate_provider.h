#pragma once

#include "../core/query_request.h"
#include "../core/word_item.h"
#include <vector>

class ICandidateProvider
{
  public:
    virtual ~ICandidateProvider() = default;

    virtual std::vector<WordItem> query(const QueryRequest &request) = 0;
    virtual void reset_cache() = 0;
};
