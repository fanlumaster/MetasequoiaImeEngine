#include <Windows.h>
#include <fmt/core.h>
#include <sqlite3.h>
#include "fmt/base.h"
#include "shuangpin/dictionary.h"
#include <chrono>
#include "shuangpin/pinyin_utils.h"

using namespace std;

void testGenerate()
{
    DictionaryUlPb dict;
    dict.generate("yi", "yi");
    vector<UINT> sequence{'N', 'I', 'R'};
    sequence = {'Y', 'I', 'R', 'F', 'I'};
    sequence = {'Y', 'I', 'G', 'E'};
    sequence = {'Y', 'I'};

    for (const auto &c : sequence)
    {
        std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
        dict.handleVkCode(c);
        std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        fmt::println("Time: {} us", duration.count());
    }
    std::vector<DictionaryUlPb::WordItem> result = dict.get_cur_candiate_list();

    for (const auto &[pinyin, word, weight] : result)
    {
        // fmt::println("Word: {}", word);
    }
}

void testGetChar()
{
    fmt::println("First char: {}", PinyinUtil::get_first_han_char("𰻝什么东西呢"));
    fmt::println("Last char: {}", PinyinUtil::get_last_han_char("𰻝什么东西呢"));
    fmt::println("First char: {}", PinyinUtil::get_first_han_char("你好呀什么东西呢"));
    fmt::println("Last char: {}", PinyinUtil::get_last_han_char("什么东西呢我的汴梁"));
}

int main(int argc, char *argv[])
{
    testGenerate();
    // testGetChar();
    return 0;
}
