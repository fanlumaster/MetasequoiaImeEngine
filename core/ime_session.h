#pragma once

#include "composition_state.h"
#include "scheme_type.h"
#include "../providers/provider_registry.h"
#include "../schemes/input_scheme.h"
#include <memory>

class ImeSession
{
  public:
    explicit ImeSession(SchemeType scheme_type = SchemeType::Shuangpin);

    void handle_key(UINT vk, UINT modifiers_down = 0, WCHAR wch = 0);
    void switch_scheme(SchemeType scheme_type);
    void reset();

    SchemeType current_scheme_type() const;
    const std::string &get_preedit() const;
    const QueryRequest &get_request() const;
    const std::vector<WordItem> &get_candidates() const;

  private:
    void refresh_candidates();
    std::unique_ptr<IInputScheme> create_scheme(SchemeType scheme_type) const;

  private:
    ProviderRegistry provider_registry_;
    std::unique_ptr<IInputScheme> scheme_;
    CompositionState state_;
};
