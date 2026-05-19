#pragma once

#include "pinyin_candidate_provider.h"
#include "../core/scheme_type.h"

class ProviderRegistry
{
  public:
    ICandidateProvider &resolve(SchemeType scheme_type);

  private:
    PinyinCandidateProvider pinyin_provider_;
};
