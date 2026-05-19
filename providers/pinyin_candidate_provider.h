#pragma once

#include "candidate_provider.h"
#include "../shuangpin/dictionary.h"

class PinyinCandidateProvider : public ICandidateProvider
{
  public:
    std::vector<WordItem> query(const QueryRequest &request) override;
    void reset_cache() override;

  private:
    DictionaryUlPb shuangpin_engine_;
};
