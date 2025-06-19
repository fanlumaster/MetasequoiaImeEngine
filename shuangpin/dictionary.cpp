#include "dictionary.h"
#include <fmt/core.h>
#include <fmt/format.h>
#include "pinyin_utils.h"
#include <mutex>
#include <shared_mutex>
#include <sqlite3.h>
#include <string>
#include <tuple>
#include <unordered_set>
#include <utility>
#include <regex>
#include <cstdlib>
#include "global_ime_vars.h"
#include "../googlepinyinime-rev/src/include/pinyinime.h"
#include "spdlog/spdlog.h"
#include <boost/locale/encoding_utf.hpp>

using namespace std;

vector<string> DictionaryUlPb::alpha_list{
    "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", //
    "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z"  //
};

vector<string> DictionaryUlPb::single_han_list{
    "啊按爱安暗阿案艾傲奥", //
    "把被不本边吧白别部比", //
    "从才此次错曾存草刺层", //
    "的到大地地但得得对多", //
    "嗯嗯而儿二尔饿呃恶耳", //
    "放发法分风飞反非服房", //
    "个过国给高感光果公更", //
    "或好会还后和很话回行", //
    "成长出处常吃场车城传", //
    "就级集家经见间几进将", //
    "看开口快空可刻苦克客", //
    "来里老啦了两力连理脸", //
    "吗没面明门名马美命目", //
    "那年女难内你男哪拿南", //
    "哦噢欧偶呕殴鸥藕区怄", //
    "平怕片跑破旁朋品派皮", //
    "请去起前气其却全轻清", //
    "人然如让日入任认容若", //
    "所三色死四思算虽似斯", //
    "他她天头同听太特它通", //
    "是说上时神深手生事声", //
    "这中只知真长正种主住", //
    "我为无问外王位文望完", //
    "下小想些笑行向学新相", //
    "一有也要以样已又意于", //
    "在子自做走再最怎作总"  //
};

DictionaryUlPb::DictionaryUlPb() : _kb_input_sequence(100), _cached_buffer(25)
{
    ime_pinyin::im_set_max_lens(64, 32);
    bool _res = ime_pinyin::im_open_decoder(                                                                          //
        (fmt::format("{}\\{}\\dict_pinyin.dat", PinyinUtil::get_local_appdata_path(), PinyinUtil::app_name)).c_str(), //
        (fmt::format("{}\\{}\\user_dict.dat", PinyinUtil::get_local_appdata_path(), PinyinUtil::app_name)).c_str()    //
    );
    if (!_res)
    {
        spdlog::error("Failed to open googleime dictionary.");
    }

    db_path = fmt::format(                    //
        "{}\\{}\\cutted_flyciku_with_jp.db",  //
        PinyinUtil::get_local_appdata_path(), //
        PinyinUtil::app_name                  //
    );
    int exit = sqlite3_open(db_path.c_str(), &db);
    if (exit != SQLITE_OK)
    {
        spdlog::error("Failed to open db.");
    }
}

/**
 * @brief Generate candidate list when not in help mode
 *
 * @param pinyin_sequence
 * @param pinyin_segmentation
 * @return vector<DictionaryUlPb::WordItem>
 */
vector<DictionaryUlPb::WordItem> DictionaryUlPb::generate( //
    const string &pinyin_sequence,                         //
    const string &pinyin_segmentation                      //
)
{
    std::shared_lock lock(mutex_);
    vector<DictionaryUlPb::WordItem> candidate_list;
    if (pinyin_sequence.size() == 0)
    {
        return candidate_list;
    }
    vector<string> code_list;
    if (pinyin_sequence.size() == 1)
    {
        generate_for_single_char(candidate_list, pinyin_sequence);
    }
    else
    {
        vector<string> pinyin_list;
        boost::split(pinyin_list, pinyin_segmentation, boost::is_any_of("'"));
        // Build sql for query
        auto sql_pair = build_sql(pinyin_sequence, pinyin_list);
        string sql_str = sql_pair.first;
        if (sql_pair.second) // Need to filter
        {
            auto key_value_weight_list = select_complete_data(sql_str);
            filter_key_value_list(candidate_list, pinyin_list, key_value_weight_list);
        }
        else
        {
            candidate_list = select_complete_data(sql_str);
        }
    }
    return candidate_list;
}

/**
 * @brief Filter with single help code
 *
 * Not only the first Hanzi part, but also the last one that will be considered.
 *   - For single Hanzi, we consider its first and last part
 *   - For Multi Hanzi, we consider first Hanzi's first part and last Hanzi's first part
 *
 * e.g. 阿: 阿's helpcode is ek, when we type aae or aak, 阿 will both be filtered.
 *      阿姨: 阿's helpcode is ek, 姨's helpcode is nr, when we type aayie or aayin, 阿姨 will both be filtered.
 *
 * @param candidate_list
 * @param filtered_list
 * @param help_code
 */
void DictionaryUlPb::filter_with_single_helpcode(                //
    const std::vector<DictionaryUlPb::WordItem> &candidate_list, //
    std::vector<DictionaryUlPb::WordItem> &filtered_list,        //
    const std::string &help_code                                 //
)
{
    if (candidate_list.empty())
        return;
    unordered_set<string> wordSet;
    if (PinyinUtil::count_utf8_chars(std::get<1>(candidate_list[0])) == 1)
    {
        /* Single Hanzi */
        for (const auto &cand : candidate_list)
        {
            string word = std::get<1>(cand);
            string firstHanChar = PinyinUtil::get_first_han_char(word);
            if (PinyinUtil::helpcode_keymap.count(firstHanChar))
            {
                if (PinyinUtil::helpcode_keymap[firstHanChar][0] == help_code[0])
                {
                    if (wordSet.count(word))
                    {
                        continue;
                    }
                    filtered_list.push_back(cand);
                    wordSet.insert(word);
                }
                if (PinyinUtil::helpcode_keymap[firstHanChar][1] == help_code[0])
                {
                    if (wordSet.count(word))
                    {
                        continue;
                    }
                    filtered_list.push_back(cand);
                    wordSet.insert(word);
                }
            }
        }
    }
    else
    {
        /* Multi Hanzi */
        for (const auto &cand : candidate_list)
        {
            string word = std::get<1>(cand);
            string firstHanChar = PinyinUtil::get_first_han_char(word);
            string lastHanChar = PinyinUtil::get_last_han_char(word);
            if (PinyinUtil::helpcode_keymap.count(firstHanChar))
            {
                if (PinyinUtil::helpcode_keymap[firstHanChar][0] == help_code[0])
                {
                    if (wordSet.count(word))
                    {
                        continue;
                    }
                    filtered_list.push_back(cand);
                    wordSet.insert(word);
                }
                if (PinyinUtil::helpcode_keymap[lastHanChar][0] == help_code[0])
                {
                    if (wordSet.count(word))
                    {
                        continue;
                    }
                    filtered_list.push_back(cand);
                    wordSet.insert(word);
                }
            }
        }
    }
}

/**
 * @brief Filter with double help codes
 *
 * Rules:
 *   - For single Hanzi, we consider its first and last part
 *   - For Multi Hanzi, we consider first Hanzi's first part and last Hanzi's first part
 *
 * e.g. 阿: 阿's helpcode is ek, when we type aaek, 阿 will be filtered.
 *      阿姨: 阿's helpcode is ek, 姨's helpcode is nr, when we type aayien, 阿姨 will be filtered.
 *
 * @param candidate_list
 * @param filtered_list
 * @param help_codes
 */
void DictionaryUlPb::filter_with_double_helpcodes(               //
    const std::vector<DictionaryUlPb::WordItem> &candidate_list, //
    std::vector<DictionaryUlPb::WordItem> &filtered_list,        //
    const std::string &help_codes                                //
)
{
    if (candidate_list.empty())
        return;
    if (PinyinUtil::count_utf8_chars(std::get<1>(candidate_list[0])) == 1)
    {
        /* Single Hanzi */
        for (const auto &cand : candidate_list)
        {
            string word = std::get<1>(cand);
            string firstHanChar = PinyinUtil::get_first_han_char(word);
            if (PinyinUtil::helpcode_keymap.count(firstHanChar))
            {
                if (PinyinUtil::helpcode_keymap[firstHanChar][0] == help_codes[0] &&
                    PinyinUtil::helpcode_keymap[firstHanChar][1] == help_codes[1])
                {
                    filtered_list.push_back(cand);
                }
            }
        }
    }
    else
    {
        /* Multi Hanzi */
        for (const auto &cand : candidate_list)
        {
            string word = std::get<1>(cand);
            string firstHanChar = PinyinUtil::get_first_han_char(word);
            string lastHanChar = PinyinUtil::get_last_han_char(word);
            if (PinyinUtil::helpcode_keymap.count(firstHanChar))
            {
                if (PinyinUtil::helpcode_keymap[firstHanChar][0] == help_codes[0] &&
                    PinyinUtil::helpcode_keymap[lastHanChar][0] == help_codes[1])
                {
                    filtered_list.push_back(cand);
                }
            }
        }
    }
}

/**
 * @brief
 *
 * Note: Use the help code only in standard cases—that is, when the shuangpin part is complete.
 *
 * @param pure_pinyin
 * @param pure_pinyin_segmentation
 * @param pinyin_sequence
 * @param help_codes
 * @return vector<DictionaryUlPb::WordItem>
 */
vector<DictionaryUlPb::WordItem> DictionaryUlPb::generate_with_helpcodes( //
    const string &pure_pinyin,                                            //
    const string &pure_pinyin_segmentation,                               //
    const string &pinyin_sequence,                                        //
    const string &help_codes                                              //
)
{
    vector<WordItem> candidate_list = generate(pure_pinyin, pure_pinyin_segmentation);
    vector<WordItem> filtered_list;
    // Filter with help codes
    if (help_codes.size() == 1)
        filter_with_single_helpcode( //
            candidate_list,          //
            filtered_list,           //
            help_codes               //
        );
    else if (help_codes.size() == 2)
        filter_with_double_helpcodes( //
            candidate_list,           //
            filtered_list,            //
            help_codes                //
        );
    return filtered_list;
}

std::string VkCodeToChar(UINT vk)
{
    if (vk >= 'A' && vk <= 'Z')
    {
        return std::string(1, char(vk + ('a' - 'A')));
    }
    if (vk >= '0' && vk <= '9')
    {
        return std::string(1, char(vk));
    }
    switch (vk)
    {
    case VK_SPACE:
        return " ";
    case VK_TAB:
        return "\t";
    case VK_RETURN:
        return "\n";
    default:
        return "";
    }
}

std::string VkSequenceToString(const UINT *vk_codes, size_t count)
{
    std::string result;
    for (size_t i = 0; i < count; ++i)
    {
        result += VkCodeToChar(vk_codes[i]);
    }
    return result;
}

void DictionaryUlPb::generate_for_single_char(vector<DictionaryUlPb::WordItem> &candidate_list, string code)
{
    string s = single_han_list[code[0] - 'a'];
    for (size_t i = 0; i < s.length();)
    {
        size_t cplen = PinyinUtil::get_first_char_size(s.substr(i, s.size() - i));
        candidate_list.push_back(make_tuple(code, s.substr(i, cplen), 1));
        i += cplen;
    }
}

/**
 * @brief
 *
 * @param vk
 * @return int
 */
int DictionaryUlPb::handleVkCode(UINT vk)
{
    _kb_input_sequence.push_back(vk);
    if (vk >= 'A' && vk <= 'Z')
    {
        _pinyin_sequence += char(vk + ('a' - 'A'));
    }
    else if (vk == VK_SPACE || (vk >= '0' && vk <= '9') || vk == VK_RETURN || vk == VK_SHIFT || vk == VK_ESCAPE)
    {
        // Clear state
        reset_state();
        return 0;
    }
    else if (vk == VK_TAB)
    {
        // Toggle help mode
        if (!_is_help_mode)
        {
            _is_help_mode = true;
            _help_mode_raw_pos = _pinyin_sequence.size();
        }
        return 0;
    }
    else if (vk == VK_BACK)
    {
        if (_pinyin_sequence.size() > 0)
        {
            _pinyin_sequence = _pinyin_sequence.substr(0, _pinyin_sequence.size() - 1);
        }
        if (_pinyin_sequence.size() == _help_mode_raw_pos)
        {
            _is_help_mode = false;
            _help_mode_raw_pos = 0;
        }
    }

    //
    // We do not handle other keys currently
    //

    /* Generate candidate list */
    // Real help mode triggered by Tab(comple help mode)
    if (_is_help_mode && _pinyin_sequence.size() > _help_mode_raw_pos)
    {
        // Shuangpin part must be complete
        if (PinyinUtil::is_all_complete_pinyin(_pure_pinyin_sequence, _pinyin_segmentation))
        {
            _pure_pinyin_sequence = _pinyin_sequence.substr(0, _help_mode_raw_pos);
            _pinyin_segmentation = PinyinUtil::pinyin_segmentation(_pure_pinyin_sequence);
            _pinyin_helpcodes = _pinyin_sequence.substr(     //
                _help_mode_raw_pos,                          //
                _pinyin_sequence.size() - _help_mode_raw_pos //
            );
            _cur_candidate_list = generate_with_helpcodes( //
                _pure_pinyin_sequence,                     //
                _pinyin_segmentation,                      //
                _pinyin_sequence,                          //
                _pinyin_helpcodes                          //
            );
        }
        else
        {
            _pinyin_segmentation = PinyinUtil::pinyin_segmentation(_pinyin_sequence);
            _cur_candidate_list = generate(_pinyin_sequence, _pinyin_segmentation);
        }
    }
    else
    {
        // Not in complete help mode:
        //   1. odd length pinyin sequence, and shuangpin part is complete, need to trigger help mode
        //   2. even length pinyin sequence, no need to trigger help mode
        if (_pinyin_sequence.size() % 2 == 1 && _pinyin_sequence.size() > 1)
        {
            _pure_pinyin_sequence = _pinyin_sequence.substr(0, _pinyin_sequence.size() - 1);
            _pinyin_segmentation = PinyinUtil::pinyin_segmentation(_pure_pinyin_sequence);
            if (PinyinUtil::is_all_complete_pinyin(_pure_pinyin_sequence, _pinyin_segmentation))
            {
                // Odd length pinyin sequence, and shuangpin part is complete, need to trigger help mode
                _pure_pinyin_sequence = _pinyin_sequence.substr(0, _pinyin_sequence.size() - 1);
                _pinyin_segmentation = PinyinUtil::pinyin_segmentation(_pure_pinyin_sequence);
                _pinyin_helpcodes = _pinyin_sequence.substr(_pinyin_sequence.size() - 1, 1);
                _cur_candidate_list = generate_with_helpcodes( //
                    _pure_pinyin_sequence,                     //
                    _pinyin_segmentation,                      //
                    _pinyin_sequence,                          //
                    _pinyin_helpcodes                          //
                );

                // Attach original pinyin's candidate list to the end of help mode candidate list
                _pinyin_segmentation = PinyinUtil::pinyin_segmentation(_pinyin_sequence);
                auto original_candidate_list = generate(_pinyin_sequence, _pinyin_segmentation);
                _cur_candidate_list.insert(          //
                    _cur_candidate_list.end(),       //
                    original_candidate_list.begin(), //
                    original_candidate_list.end()    //
                );
            }
            else
            {
                // Still use pure pinyin without triggerring help mode
                _pinyin_segmentation = PinyinUtil::pinyin_segmentation(_pinyin_sequence);
                _cur_candidate_list = generate(_pinyin_sequence, _pinyin_segmentation);
            }
        }
        else
        {
            // Even length pinyin sequence, no need to trigger help mode
            _pinyin_segmentation = PinyinUtil::pinyin_segmentation(_pinyin_sequence);
            _cur_candidate_list = generate(_pinyin_sequence, _pinyin_segmentation);
        }
    }

    _pinyin_segmentation = PinyinUtil::pinyin_segmentation(_pinyin_sequence);

    return 0;
}

void DictionaryUlPb::filter_key_value_list(                       //
    vector<DictionaryUlPb::WordItem> &candidate_list,             //
    const vector<string> &pinyin_list,                            //
    const vector<DictionaryUlPb::WordItem> &key_value_weight_list //
)
{
    string regex_str("");
    for (const auto &each_pinyin : pinyin_list)
    {
        if (each_pinyin.size() == 2)
        {
            regex_str += each_pinyin;
        }
        else
        {
            regex_str = regex_str + each_pinyin + "[a-z]";
        }
    }
    regex pattern(regex_str);
    for (const auto &each_tuple : key_value_weight_list)
    {
        if (regex_match(get<0>(each_tuple), pattern))
        {
            candidate_list.push_back(each_tuple);
        }
    }
}

vector<DictionaryUlPb::WordItem> DictionaryUlPb::generate_for_creating_word(const string code)
{
    return select_complete_data(build_sql_for_creating_word(code));
}

int DictionaryUlPb::create_word(string pinyin, string word)
{
    string jp;
    for (size_t i = 0; i < pinyin.size(); i += 2)
        jp += pinyin[i];
    if (!do_validate(pinyin, jp, word))
        return ERROR_CODE;
    if (check_data(build_sql_for_checking_word(pinyin, jp, word)))
    {
        return OK;
    }
    insert_data(build_sql_for_inserting_word(pinyin, jp, word));
    return OK;
}

string DictionaryUlPb::build_sql_for_updating_word(string word)
{
    int han_cnt = PinyinUtil::cnt_han_chars(word);
    string pinyin = GlobalIME::pinyin.substr(0, han_cnt * 2);
    string jp;
    for (size_t i = 0; i < pinyin.size(); i += 2)
        jp += pinyin[i];
    if (!do_validate(pinyin, jp, word))
        return "";
    string table = choose_tbl(pinyin, jp.size());
    string base_sql = "update {0} set weight = ( select MAX(weight) + 1 from {0} AS sub where sub.key = '{1}') "
                      "where key = '{1}' and value = '{2}';";
    string res_sql = fmt::format(base_sql, table, pinyin, word);
    return res_sql;
}

int DictionaryUlPb::update_data(string sql_str)
{
    sqlite3_stmt *stmt;
    int exit = sqlite3_prepare_v2(db, sql_str.c_str(), -1, &stmt, 0);
    if (exit != SQLITE_OK)
    {
        spdlog::error("sqlite3_prepare_v2 error.");
    }
    exit = sqlite3_step(stmt);
    if (exit != SQLITE_DONE)
    {
        spdlog::error("sqlite3_step error.");
    }
    sqlite3_finalize(stmt);
    return 0;
}

int DictionaryUlPb::update_weight_by_word(string word)
{
    std::unique_lock lock(mutex_);
    update_data(build_sql_for_updating_word(word));
    return OK;
}

// generate_with_seg_pinyin

DictionaryUlPb::~DictionaryUlPb()
{
    if (db)
    {
        sqlite3_close(db);
    }
}

vector<string> DictionaryUlPb::select_data(string sql_str)
{
    vector<string> candidateList;
    sqlite3_stmt *stmt;
    int exit = sqlite3_prepare_v2(db, sql_str.c_str(), -1, &stmt, 0);
    if (exit != SQLITE_OK)
    {
        spdlog::error("sqlite3_prepare_v2 error.");
    }
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        candidateList.push_back(string(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2))));
    }
    sqlite3_finalize(stmt);
    return candidateList;
}

vector<DictionaryUlPb::WordItem> DictionaryUlPb::select_complete_data(string sql_str)
{
    vector<DictionaryUlPb::WordItem> candidateList;
    sqlite3_stmt *stmt;
    int exit = sqlite3_prepare_v2(db, sql_str.c_str(), -1, &stmt, 0);
    if (exit != SQLITE_OK)
    {
        spdlog::error("sqlite3_prepare_v2 error.");
    }
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        candidateList.push_back(make_tuple(                                       //
            string(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0))), // key
            string(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2))), // value
            sqlite3_column_int(stmt, 3))                                          // weight
        );
    }
    sqlite3_finalize(stmt);
    return candidateList;
}

vector<pair<string, string>> DictionaryUlPb::select_key_and_value(string sql_str)
{
    vector<pair<string, string>> candidateList;
    sqlite3_stmt *stmt;
    int exit = sqlite3_prepare_v2(db, sql_str.c_str(), -1, &stmt, 0);
    if (exit != SQLITE_OK)
    {
        spdlog::error("sqlite3_prepare_v2 error.");
    }
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        candidateList.push_back(make_pair(string(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0))),
                                          string(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2)))));
    }
    sqlite3_finalize(stmt);
    return candidateList;
}

/**
 * @brief Check if data exists
 *
 * @param sql_str
 * @return int
 */
int DictionaryUlPb::check_data(string sql_str)
{
    sqlite3_stmt *stmt;
    int exit = sqlite3_prepare_v2(db, sql_str.c_str(), -1, &stmt, 0);
    if (exit != SQLITE_OK)
    {
        spdlog::error("sqlite3_prepare_v2 error.");
    }
    bool exists = false;
    exit = sqlite3_step(stmt);
    if (exit == SQLITE_ROW)
    {
        exists = true;
    }
    sqlite3_finalize(stmt);
    return exists;
}

int DictionaryUlPb::insert_data(string sql_str)
{
    sqlite3_stmt *stmt;
    int exit = sqlite3_prepare_v2(db, sql_str.c_str(), -1, &stmt, 0);
    if (exit != SQLITE_OK)
    {
        spdlog::error("sqlite3_prepare_v2 error.");
    }
    exit = sqlite3_step(stmt);
    if (exit != SQLITE_DONE)
    {
        spdlog::info("sqlite3_step error.");
    }
    sqlite3_finalize(stmt);
    return 0;
}

pair<string, bool> DictionaryUlPb::build_sql(const string &sp_str, vector<string> &pinyin_list)
{
    bool all_entire_pinyin = true;
    bool all_jp = true;
    vector<string>::size_type jp_cnt = 0; // 简拼的数量
    for (vector<string>::size_type i = 0; i < pinyin_list.size(); i++)
    {
        string cur_pinyin = pinyin_list[i];
        if (cur_pinyin.size() == 1)
        {
            all_entire_pinyin = false;
            jp_cnt += 1;
        }
        else
        {
            all_jp = false;
        }
    }
    string sql;
    string base_sql("select * from {0} where {1} = '{2}' order by weight desc limit {3};");
    string table = choose_tbl(sp_str, pinyin_list.size());
    bool need_filtering = false;
    if (all_entire_pinyin) // Segmentations are all quanpin
    {
        sql = fmt::format(base_sql, table, "key", sp_str, default_candicate_page_limit);
    }
    else if (all_jp) // Segmentations are all jianpin
    {
        sql = fmt::format(sql, table, "jp", sp_str, default_candicate_page_limit);
    }
    else if (jp_cnt == 1) // Only one jianpin
    {
        string sql_param0("");
        string sql_param1("");
        for (vector<string>::size_type i = 0; i < pinyin_list.size(); i++)
        {
            if (pinyin_list[i].size() == 1)
            {
                sql_param0 = sql_param0 + pinyin_list[i] + "a";
                sql_param1 = sql_param1 + pinyin_list[i] + "z";
            }
            else
            {
                sql_param0 += pinyin_list[i];
                sql_param1 += pinyin_list[i];
            }
        }
        sql = fmt::format(                                                                           //
            "select * from {0} where key >= '{1}' and key <= '{2}' order by weight desc limit {3};", //
            table, sql_param0, sql_param1, default_candicate_page_limit                              //
        );
    }
    else // Neithor pure quanpin, nor pure jianpin, and count of jianpin is more than 1
    {
        need_filtering = true;
        string sql_param("");
        for (string &cur_pinyin : pinyin_list)
        {
            sql_param += cur_pinyin.substr(0, 1);
        }
        // TODO: not adding weight desc
        sql = fmt::format("select * from {0} where jp = '{1}';", table, sql_param);
    }
    return make_pair(sql, need_filtering);
}

string DictionaryUlPb::build_sql_for_creating_word(const string &sp_str)
{
    string base_sql = "select * from(select * from {} where key = '{}' order by weight desc limit {})";
    string res_sql =
        fmt::format(base_sql, choose_tbl(sp_str.substr(0, 2), 1), sp_str.substr(0, 2), default_candicate_page_limit);
    string trimed_sp_str = sp_str.substr(0, 8); // 4 hanzi at most
    for (size_t i = 4; i <= sp_str.size(); i += 2)
    {
        res_sql = fmt::format(                                //
                      base_sql,                               //
                      choose_tbl(sp_str.substr(0, i), i / 2), //
                      sp_str.substr(0, i),                    //
                      default_candicate_page_limit)           //
                  + " union all "                             //
                  + res_sql;
    }
    return res_sql;
}

string DictionaryUlPb::build_sql_for_checking_word(string key, string jp, string value)
{
    string table = choose_tbl(key, jp.size());
    string base_sql = "select 1 from {} where key = '{}' and value = '{}';";
    return fmt::format(base_sql, table, key, value); // Default weight is 10,000
}

string DictionaryUlPb::build_sql_for_inserting_word(string key, string jp, string value)
{
    string table = choose_tbl(key, jp.size());
    string base_sql = "insert into {} (key, jp, value, weight) values ('{}', '{}', '{}', '{}');";
    return fmt::format(base_sql, table, key, jp, value, 10000); // Default weight is 10,000
}

string DictionaryUlPb::choose_tbl(const string &sp_str, size_t word_len)
{
    string base_tbl("tbl_{}_{}");
    if (word_len >= 8)
        return fmt::format(base_tbl, "others", sp_str[0]);
    return fmt::format(base_tbl, word_len, sp_str[0]);
}

bool DictionaryUlPb::do_validate(string key, string jp, string value)
{
    if (key.size() % 2 || jp.size() != key.size() / 2 || key.size() != PinyinUtil::cnt_han_chars(value) * 2)
        return false;
    return true;
}

string from_utf16(const ime_pinyin::char16 *buf, size_t len)
{
    u16string utf16Str(reinterpret_cast<const char16_t *>(buf), len);
    return boost::locale::conv::utf_to_utf<char>(utf16Str);
}

string DictionaryUlPb::search_sentence_from_ime_engine(const string &user_pinyin)
{
    string pinyin_str = user_pinyin;
    const char *pinyin = pinyin_str.c_str();
    size_t cand_cnt = ime_pinyin::im_search(pinyin, strlen(pinyin));
    string msg;
    cand_cnt = cand_cnt > 0 ? 1 : 0;
    for (size_t i = 0; i < cand_cnt; ++i)
    {
        ime_pinyin::char16 buf[256] = {0};
        ime_pinyin::im_get_candidate(i, buf, 255);
        size_t len = 0;
        while (buf[len] != 0 && len < 255)
            ++len;
        msg = from_utf16(buf, len);
    }
    return msg;
}

void DictionaryUlPb::reset_state()
{
    _is_help_mode = false;
    _help_mode_raw_pos = 0;
    _kb_input_sequence.clear();
    _pinyin_sequence = "";
    _pure_pinyin_sequence = "";
    _help_codes_sequence.fill(0);
    _cur_candidate_list.clear();
    _cur_page_candidate_list.clear();
}