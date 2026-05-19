//
// 测试拼音输入法的核心逻辑，包括双拼和全拼方案，以及动态切换输入方案的功能。
//
#include <Windows.h>
#include <fmt/core.h>
#include "fmt/base.h"
#include <chrono>
#include "core/ime_session.h"

using namespace std;

void print_candidates(const std::vector<WordItem> &result)
{
    for (const auto &[code, word, weight] : result)
    {
        fmt::println("Candidate: {} [{}] ({})", word, code, weight);
    }
}

void feed_sequence(ImeSession &session, const vector<UINT> &sequence, const vector<WCHAR> &wch_sequence = {})
{
    for (int i = 0; i < sequence.size(); ++i)
    {
        std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
        session.handle_key(sequence[i], 0, i < wch_sequence.size() ? wch_sequence[i] : 0);
        std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        fmt::println("Preedit: {}", session.get_preedit());
        fmt::println("Time: {} us", duration.count());
    }
}

void test_shuangpin_session()
{
    ImeSession session(SchemeType::Shuangpin);
    const vector<UINT> sequence{'C', 'L', 'S'};      // 按键的 vk 码
    const vector<WCHAR> wch_sequence{'c', 'l', 's'}; // 实际的字符，区分大小写输入

    fmt::println("==== Shuangpin ====");
    feed_sequence(session, sequence, wch_sequence);
    print_candidates(session.get_candidates());
}

void test_shuangpin_session02()
{
    // ImeSession session(SchemeType::Quanpin);
    ImeSession session(SchemeType::Shuangpin);
    // const vector<UINT> sequence{'C', 'E', 'L', 'I', 'S', 'H', 'I'};
    const vector<UINT> sequence{'C', 'E', 'L', 'I', 'U', 'I'};
    const vector<WCHAR> wch_sequence{'c', 'e', 'l', 'i', 'u', 'i'};

    fmt::println("==== Shuangpin ====");
    feed_sequence(session, sequence, wch_sequence);
    print_candidates(session.get_candidates());
}

void test_quanpin_session()
{
    // ImeSession session(SchemeType::Quanpin);
    ImeSession session(SchemeType::Shuangpin);
    // const vector<UINT> sequence{'C', 'E', 'L', 'I', 'S', 'H', 'I'};
    const vector<UINT> sequence{'C', 'E', 'L', 'I', 'U', 'I'};
    const vector<WCHAR> wch_sequence{'c', 'e', 'l', 'i', 'u', 'i'};

    fmt::println("==== Quanpin ====");
    feed_sequence(session, sequence, wch_sequence);
    print_candidates(session.get_candidates());
}

void test_dynamic_switch()
{
    ImeSession session(SchemeType::Shuangpin);

    fmt::println("==== Switch Scheme ====");
    feed_sequence(session, {'N', 'I'}, {'n', 'i'});
    fmt::println("Before switch preedit: {}", session.get_preedit());

    session.switch_scheme(SchemeType::Quanpin);
    fmt::println("After switch preedit: {}", session.get_preedit());

    feed_sequence(session, {'N', 'I', 'H', 'A', 'O'}, {'n', 'i', 'h', 'a', 'o'});
    print_candidates(session.get_candidates());
}

int main(int argc, char *argv[])
{
    test_shuangpin_session();
    test_shuangpin_session02();
    // test_quanpin_session();
    test_dynamic_switch();
    return 0;
}
