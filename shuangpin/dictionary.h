#pragma once

#include <windows.h>
#include <shared_mutex>
#include <array>
#include <vector>
#include <tuple>
#include <unordered_map>
#include <string>
#include <fstream>
#include <sqlite3.h>
#include <memory>
#include <boost/algorithm/string.hpp>
#include <boost/circular_buffer.hpp>

class DictionaryUlPb
{
  public:
    /**
     * @brief
     *
     * first: pinyin
     * second: word
     * third: weight
     */
    using WordItem = std::tuple<std::string, std::string, int>;

    static const int OK = 0;
    static const int ERROR_CODE = -1;

    std::vector<WordItem> generate(            //
        const std::string &pinyin_sequence,    //
        const std::string &pinyin_segmentation //
    );
    std::vector<WordItem> generate_with_helpcodes(   //
        const std::string &pure_pinyin,              //
        const std::string &pure_pinyin_segmentation, //
        const std::string &pinyin_sequence,          //
        const std::string &help_codes                //
    );
    int handleVkCode(UINT vk);
    std::vector<WordItem> generate_for_creating_word(const std::string code);
    int create_word(std::string pinyin, std::string word);
    // 一次到顶
    int update_weight_by_word(std::string word);

    /*
      Return: list of complete item data of database table
    */
    std::vector<WordItem> generate_tuple(const std::string code);

    std::string search_sentence_from_ime_engine(const std::string &user_pinyin);

    DictionaryUlPb();
    ~DictionaryUlPb();

  private:
    std::ifstream inputFile;
    std::string db_path;
    sqlite3 *db = nullptr;
    std::unordered_map<std::string, std::vector<std::string>> dict_map;
    int default_candicate_page_limit = 80;

    static std::vector<std::string> alpha_list;
    static std::vector<std::string> single_han_list;

    void generate_for_single_char(std::vector<WordItem> &candidate_list, std::string code);
    void filter_with_single_helpcode(                //
        const std::vector<WordItem> &candidate_list, //
        std::vector<WordItem> &filtered_list,        //
        const std::string &help_code                 //
    );
    void filter_with_double_helpcodes(               //
        const std::vector<WordItem> &candidate_list, //
        std::vector<WordItem> &filtered_list,        //
        const std::string &help_codes                //
    );
    void filter_key_value_list(                            //
        std::vector<WordItem> &candidate_list,             //
        const std::vector<std::string> &pinyin_list,       //
        const std::vector<WordItem> &key_value_weight_list //
    );
    std::vector<std::string> select_data(std::string sql_str);
    std::vector<WordItem> select_complete_data(std::string sql_str);
    std::vector<std::pair<std::string, std::string>> select_key_and_value(std::string sql_str);
    int check_data(std::string sql_str);
    int insert_data(std::string sql_str);
    int update_data(std::string sql_str);
    std::pair<std::string, bool> build_sql(const std::string &sp_str, std::vector<std::string> &pinyin_list);
    std::string build_sql_for_creating_word(const std::string &sp_str);
    std::string build_sql_for_checking_word(std::string key, std::string jp, std::string value);
    std::string build_sql_for_inserting_word(std::string key, std::string jp, std::string value);
    std::string build_sql_for_updating_word(std::string value);
    std::string choose_tbl(const std::string &sp_str, size_t word_len);
    bool do_validate(std::string key, std::string jp, std::string value);

  private:
    // Lock
    std::shared_mutex mutex_; // Read-write separation lock

    // Whether in help mode
    bool _is_help_mode = false;
    // Localtion of starting position
    int _help_mode_raw_pos = 0;                    // Start from pos, e.g. 妮: ninv: 2
    std::string _pinyin_helpcodes = "";            // Help codes
    std::vector<UINT> _kb_input_sequence;          // Keyboard input sequence
    std::string _pinyin_sequence = "";             // Pinyin extracted from from keyboard sequence
    std::string _pure_pinyin_sequence = "";        // Pinyin without help code
    std::array<char, 2> _help_codes_sequence = {}; // Help code extracted from from keyboard sequence
    std::string _pinyin_segmentation = "";         // Segmentation pinyin
    std::string _preedit_pinyin = "";              // Preedit
    /* Current candidate list, computed by current kb_input_sequence */
    std::vector<WordItem> _cur_candidate_list;
    std::vector<WordItem> _cur_page_candidate_list; // Current candidate list
    boost::circular_buffer<std::pair<std::string, std::vector<WordItem>>> _cached_buffer;

  public:
    // Getters and setters
    bool get_help_mode()
    {
        return this->_is_help_mode;
    }
    void set_help_mode(bool is_help_mode)
    {
        this->_is_help_mode = is_help_mode;
    }

    int get_help_mode_raw_pos()
    {
        return this->_help_mode_raw_pos;
    }
    void set_help_mode_raw_pos(int raw_pos)
    {
        this->_help_mode_raw_pos = raw_pos;
    }

    const std::string &get_segmentation_pinyin()
    {
        return this->_pinyin_segmentation;
    }

    const std::vector<DictionaryUlPb::WordItem> &get_cur_candiate_list()
    {
        return this->_cur_candidate_list;
    }

    void reset_state();
};