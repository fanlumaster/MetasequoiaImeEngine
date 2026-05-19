#include "quanpin_scheme.h"
#include <cctype>

namespace
{
bool is_alpha_vk(UINT vk)
{
    return vk >= 'A' && vk <= 'Z';
}
} // namespace

void QuanpinScheme::reset()
{
    raw_input_.clear();
}

void QuanpinScheme::handle_key(UINT vk, UINT modifiers_down, WCHAR wch)
{
    (void)modifiers_down;
    if (vk == VK_BACK)
    {
        if (!raw_input_.empty())
        {
            raw_input_.pop_back();
        }
        return;
    }

    if (vk == VK_ESCAPE || vk == VK_RETURN || vk == VK_SPACE)
    {
        reset();
        return;
    }

    if (vk == VK_OEM_7)
    {
        raw_input_.push_back('\'');
        return;
    }

    if (!is_alpha_vk(vk))
    {
        return;
    }

    if (wch >= L'A' && wch <= L'Z')
    {
        raw_input_.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(wch))));
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

QueryRequest QuanpinScheme::build_request() const
{
    QueryRequest request;
    request.scheme = type();
    request.raw_input = raw_input_;
    request.segmentation = raw_input_;
    request.normalized_input.reserve(raw_input_.size());
    for (const char ch : raw_input_)
    {
        if (ch != '\'')
        {
            request.normalized_input.push_back(ch);
        }
    }
    request.valid = !request.normalized_input.empty();
    return request;
}

std::string QuanpinScheme::get_preedit() const
{
    return raw_input_;
}

SchemeType QuanpinScheme::type() const
{
    return SchemeType::Quanpin;
}
