#include "pinyin_candidate_provider.h"
#include "../core/scheme_type.h"
#include <fmt/core.h>
#include <fmt/base.h>

std::vector<WordItem> PinyinCandidateProvider::query(const QueryRequest &request)
{
    if (!request.valid)
    {
        return {};
    }

    if (request.scheme == SchemeType::Shuangpin)
    {
        shuangpin_engine_.reset_state();
        for (const auto &key_stroke : request.key_strokes)
        {
            fmt::println("Handling keystroke: vk={}", key_stroke.vk);
            shuangpin_engine_.handleVkCode(key_stroke.vk, key_stroke.modifiers_down, key_stroke.wch);
        }
        fmt::println("length: {}", shuangpin_engine_.get_cur_candiate_list().size());
        return shuangpin_engine_.get_cur_candiate_list();
    }

    if (request.scheme == SchemeType::Quanpin)
    {
        std::vector<WordItem> result;
        const std::string sentence = shuangpin_engine_.search_sentence_from_ime_engine(request.normalized_input);
        if (!sentence.empty())
        {
            result.emplace_back(request.normalized_input, sentence, 1);
        }
        return result;
    }

    return {};
}

void PinyinCandidateProvider::reset_cache()
{
    shuangpin_engine_.reset_cache();
}
