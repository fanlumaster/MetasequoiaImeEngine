#pragma once

#include "input_scheme.h"
#include <string>
#include <vector>

class ShuangpinScheme : public IInputScheme
{
  public:
    void reset() override;
    void handle_key(UINT vk, UINT modifiers_down, WCHAR wch) override;
    QueryRequest build_request() const override;
    std::string get_preedit() const override;
    SchemeType type() const override;

  private:
    std::string raw_input_;
    std::vector<KeyStroke> key_strokes_;
};
