#ifndef LAB_DB_H
#define LAB_DB_H
#include <stdbool.h>
#define INDEX_MAX_SIZE 32
void console_interface();
struct Instrument {
    unsigned id;
    char name[32];
    char type[32];
    char country[32];
};

struct Materials {
    unsigned id;
    char material[32];
    float amount;
};

struct IndexItem {
    unsigned id;
    unsigned record_no;
};

struct DataMeta {
    unsigned size;
    unsigned size_valid;
    unsigned max_id;
};

struct MIndex {
    struct IndexItem data[INDEX_MAX_SIZE];
    unsigned int size;
    unsigned int max_id;
} m_index;

void load_db();

void save_index();

void onclose_db();

int get_m_record_no(unsigned id);

int get_s_record_no(unsigned id);

int get_s_of_m_record_no(unsigned m_id, unsigned id);

struct Instrument *get_m(unsigned id);

struct Materials *get_s_at_line(int record_no);

struct Materials *get_s(unsigned id);

struct Materials *get_s_of_m(unsigned m_id, unsigned id);

int insert_m(const char name[32], const char type[32], const char country[32]);

int insert_s(unsigned m_id, const char material[32], float amount);

int update_m(unsigned id, const char name[32], const char type[32], const char contry[32]);

int update_s_at_line(int record_no, const char material[32], float amount);

int update_s(unsigned id, const char material[32], float amount);

int update_s_of_m(unsigned m_id, unsigned id, const char material[32], float amount);

int del_m(unsigned id);

int del_s_at_line(int record_no);

int del_s(unsigned id);

int del_s_of_m(unsigned m_id, unsigned id);

unsigned size_m();

unsigned size_s();

int size_s_of_m(unsigned m_id);

void ut_m(bool print_removed);

void ut_s(bool print_removed);

void defragment_m();

void defragment_s();

#endif