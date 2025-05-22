#include <fmt/core.h>
#include <sqlite3.h>
#include "fmt/base.h"
#include "shuangpin/dictionary.h"
#include <chrono>

void testGenerate() {
    DictionaryUlPb dict;
    std::vector<DictionaryUlPb::WordItem> result = dict.generate("ceui");
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    result = dict.generate("keui");
    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    fmt::println("Time: {} us", duration.count());
    fmt::println("Word count: {}", result.size());
    for (const auto &[pinyin, word, weight] : result)
    {
        fmt::println("Word: {}", word);
    }
}

int main(int argc, char *argv[])
{
    testGenerate();
    return 0;
}
