#include "shuangpin_scheme.h"
#include "../shuangpin/pinyin_utils.h"
#include <boost/algorithm/string/replace.hpp>

namespace
{
bool is_alpha_vk(UINT vk)
{
    return vk >= 'A' && vk <= 'Z';
}
} // namespace

void ShuangpinScheme::reset()
{
    raw_input_.clear();
    key_strokes_.clear();
}

void ShuangpinScheme::handle_key(UINT vk, UINT modifiers_down, WCHAR wch)
{
    if (vk == VK_BACK)
    {
        if (!raw_input_.empty())
        {
            raw_input_.pop_back();
        }
        if (!key_strokes_.empty())
        {
            key_strokes_.pop_back();
        }
        return;
    }

    if (vk == VK_ESCAPE || vk == VK_RETURN || vk == VK_SPACE)
    {
        reset();
        return;
    }

    if (!is_alpha_vk(vk))
    {
        return;
    }

    key_strokes_.push_back(KeyStroke{vk, modifiers_down, wch});
    if (wch >= L'A' && wch <= L'Z')
    {
        raw_input_.push_back(static_cast<char>(wch));
    }
    else if (wch >= L'a' && wch <= L'z')
    {
        raw_input_.push_back(static_cast<char>(wch));
    }
    else
    {
        raw_input_.push_back(static_cast<char>(vk + ('a' - 'A')));
    }
}

QueryRequest ShuangpinScheme::build_request() const
{
    QueryRequest request;
    request.scheme = type();
    request.raw_input = raw_input_;
    request.key_strokes = key_strokes_;
    request.valid = !raw_input_.empty();

    if (!request.valid)
    {
        return request;
    }

    const std::string segmentation = PinyinUtil::pinyin_segmentation(raw_input_);
    request.segmentation = PinyinUtil::convert_seg_shuangpin_to_seg_complete_pinyin(segmentation);
    request.normalized_input = boost::replace_all_copy(request.segmentation, "'", "");
    return request;
}

std::string ShuangpinScheme::get_preedit() const
{
    return raw_input_;
}

SchemeType ShuangpinScheme::type() const
{
    return SchemeType::Shuangpin;
}
