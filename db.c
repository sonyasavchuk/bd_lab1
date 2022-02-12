#include "db.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

const char M_INDEX_FILENAME[] = "index_m.ind";
const char M_DATA_FILENAME[] = "data_m.fl";
const char S_DATA_FILENAME[] = "data_s.fl";

void load_db() {
    struct DataMeta m_meta = {0, 0, 0};
    struct DataMeta s_meta = {0, 0, 0};
    FILE *m_data_file = fopen(M_DATA_FILENAME, "rb");
    if (m_data_file == NULL) {
        m_data_file = fopen(M_DATA_FILENAME, "ab");
        fwrite(&m_meta, sizeof(struct DataMeta), 1, m_data_file);
    } else {
        fread(&m_meta, sizeof(struct DataMeta), 1, m_data_file);
    }
    fclose(m_data_file);
    FILE *s_data_file = fopen(S_DATA_FILENAME, "rb");
    if (s_data_file == NULL) {
        s_data_file = fopen(S_DATA_FILENAME, "ab");
        fwrite(&s_meta, sizeof(s_meta), 1, s_data_file);
    }
    fclose(s_data_file);
    m_index.max_id = m_meta.max_id;
    m_index.size = m_meta.size_valid;
    FILE *m_index_file = fopen("index_m.ind", "rb");
    if (m_index_file == NULL) {
        m_index_file = fopen("index_m.ind", "ab");
    } else {
        for (unsigned i = 0; i < m_index.size; i++) {
            fread(m_index.data + i, sizeof(struct IndexItem), 1, m_index_file);
        }
    }
    fclose(m_index_file);
}

void save_index() {
    FILE *m_index_file = fopen("index_m.ind", "wb");
    for (unsigned i = 0; i < m_index.size; i++) {
        fwrite(m_index.data + i, sizeof(struct IndexItem), 1, m_index_file);
    }
    fclose(m_index_file);
}

void onclose_db(){
    defragment_m();
    defragment_s();
    save_index();
};

int get_m_record_no(unsigned id) {
    if (m_index.size == 0) {
        return -1;
    }
    unsigned left = 0;
    unsigned right = m_index.size - 1;
    while (left <= right) {
        unsigned mid = (left + right) / 2;
        if (m_index.data[mid].id > id) {
            if (mid != 0) {
                right = mid - 1;
            } else {
                break;
            }
        } else if (m_index.data[mid].id < id) {
            if (mid != m_index.size - 1) {
                left = mid + 1;
            } else {
                break;
            }
        } else {
            return (int) m_index.data[mid].record_no;
        }
    }
    return -1;
}

int get_s_record_no(unsigned id) {
    FILE *s_data_file = fopen(S_DATA_FILENAME, "rb");
    struct DataMeta s_meta;
    fread(&s_meta, sizeof(struct DataMeta), 1, s_data_file);
    if (id > s_meta.max_id) {
        fclose(s_data_file);
        return -1;
    }
    for (int i = 1; i <= s_meta.size_valid; i++) {
        struct Materials *inf = malloc(sizeof(struct Materials));
        bool valid;
        fread(inf, sizeof(struct Materials), 1, s_data_file);
        fread(&valid, sizeof(bool), 1, s_data_file);
        if (inf->id == id) {
            fclose(s_data_file);
            return (valid) ? i : -1;
        }
        fseek(s_data_file, sizeof(int), SEEK_CUR);
    }
    fclose(s_data_file);
    return -1;
}

int get_s_of_m_record_no(unsigned m_id, unsigned id) {
    unsigned m_record_no = get_m_record_no(m_id);
    if (m_record_no == -1) {
        return -1;
    }
    FILE *m_data_file = fopen(M_DATA_FILENAME, "rb");
    FILE *s_data_file = fopen(S_DATA_FILENAME, "rb");
    struct Materials *inf = malloc(sizeof(struct Materials));
    int s_record_no;
    fseek(m_data_file,
          sizeof(struct DataMeta) + (m_record_no) * (sizeof(struct Instrument) + sizeof(bool)) +
          (m_record_no - 1) * sizeof(int),
          SEEK_SET
    );
    fread(&s_record_no, sizeof(int), 1, m_data_file);
    while (s_record_no != -1) {
        fseek(s_data_file,
              sizeof(struct DataMeta) + (s_record_no - 1) * (sizeof(struct Materials) + sizeof(bool) + sizeof(int)),
              SEEK_SET
        );
        fread(inf, sizeof(struct Materials), 1, s_data_file);
        if (inf->id == id) {
            return s_record_no;
        }
        fseek(s_data_file, sizeof(bool), SEEK_CUR);
        fread(&s_record_no, sizeof(int), 1, s_data_file);
    }
    fclose(m_data_file);
    fclose(s_data_file);
    return -1;
}

struct Instrument* get_m(unsigned id) {
    int record_no = get_m_record_no(id);
    if (record_no != -1) {
        struct Instrument *instrument = malloc(sizeof(struct Instrument));
        FILE *m_data_file = fopen(M_DATA_FILENAME, "rb");
        fseek(m_data_file,
              sizeof(struct DataMeta) + (record_no - 1) * (sizeof(struct Instrument) + sizeof(bool) + sizeof(int)),
              SEEK_SET
        );
        fread(instrument, sizeof(struct Instrument), 1, m_data_file);
        fclose(m_data_file);
        return instrument;
    } else {
        return NULL;
    }
}

struct Materials *get_s_at_line(int record_no) {
    if (record_no != -1) {
        struct Materials *material = malloc(sizeof(struct Materials));
        FILE *s_data_file = fopen(S_DATA_FILENAME, "rb");
        fseek(s_data_file,
              sizeof(struct DataMeta) + (record_no - 1) * (sizeof(struct Materials) + sizeof(bool) + sizeof(int)),
              SEEK_SET
        );
        fread(material, sizeof(struct Materials), 1, s_data_file);
        fclose(s_data_file);
        return material;
    } else {
        return NULL;
    }
}

struct Materials *get_s(unsigned id) {
    return get_s_at_line(get_s_record_no(id));
}

struct Materials *get_s_of_m(unsigned m_id, unsigned id) {
    return get_s_at_line(get_s_of_m_record_no(m_id, id));
}

int insert_m(const char name[32], const char type[32], const char country[32]) {
    if (m_index.size == INDEX_MAX_SIZE) {
        return 1;
    }
    FILE *m_data_file = fopen(M_DATA_FILENAME, "rb+");
    struct DataMeta m_meta;
    fseek(m_data_file, 0, SEEK_SET);
    fread(&m_meta, sizeof(struct DataMeta), 1, m_data_file);
    if (m_meta.size_valid == m_meta.size) {
        m_meta.size++;
    }
    m_meta.size_valid++;
    m_index.max_id = ++m_meta.max_id;
    fseek(m_data_file, 0, SEEK_SET);
    fwrite(&m_meta, sizeof(struct DataMeta), 1, m_data_file);
    struct Instrument instrument;
    instrument.id = m_index.max_id;
    strcpy(instrument.name, name);
    strcpy(instrument.type,type);
    strcpy(instrument.country,country);
    fseek(m_data_file,
          sizeof(struct DataMeta) + (m_meta.size_valid - 1) * (sizeof(struct Instrument) + sizeof(bool) + sizeof(int)),
          SEEK_SET
    );
    fwrite(&instrument, sizeof(struct Instrument), 1, m_data_file);
    bool valid = true;
    fwrite(&valid, sizeof(bool), 1, m_data_file);
    int s_record_no = -1;
    fwrite(&s_record_no, sizeof(int), 1, m_data_file);
    fclose(m_data_file);
    struct IndexItem newIndexItem = {instrument.id, m_meta.size_valid};
    m_index.data[m_index.size++] = newIndexItem;
    return 0;
}

int insert_s(unsigned m_id, const char material[32], float amount) {
    int m_record_no = get_m_record_no(m_id);
    if (m_record_no == -1) {
        return 1;
    }
    FILE *s_data_file = fopen(S_DATA_FILENAME, "rb+");
    struct DataMeta s_meta;
    fseek(s_data_file, 0, SEEK_SET);
    fread(&s_meta, sizeof(struct DataMeta), 1, s_data_file);
    if (s_meta.size_valid == s_meta.size) {
        s_meta.size++;
    }
    s_meta.size_valid++;
    s_meta.max_id++;
    fseek(s_data_file, 0, SEEK_SET);
    fwrite(&s_meta, sizeof(struct DataMeta), 1, s_data_file);
    struct Materials inf;
    inf.id = s_meta.max_id;
    strcpy(inf.material,material);
    inf.amount = amount;
    fseek(s_data_file,
          sizeof(struct DataMeta) + (s_meta.size_valid - 1) * (sizeof(struct Materials) + sizeof(bool) + sizeof(int)),
          SEEK_SET);
    fwrite(&inf, sizeof(struct Materials), 1, s_data_file);
    bool valid = true;
    fwrite(&valid, sizeof(bool), 1, s_data_file);
    int next_s_record_no = -1;
    fwrite(&next_s_record_no, sizeof(int), 1, s_data_file);
    FILE *m_data_file = fopen(M_DATA_FILENAME, "rb+");
    fseek(m_data_file,
          sizeof(struct DataMeta) + (m_record_no) * (sizeof(struct Instrument) + sizeof(bool)) +
          (m_record_no - 1) * sizeof(int),
          SEEK_SET
    );
    fread(&next_s_record_no, sizeof(int), 1, m_data_file);
    if (next_s_record_no == -1) {
        fseek(m_data_file, (long) -sizeof(int), SEEK_CUR);
        fwrite(&s_meta.size_valid, sizeof(int), 1, m_data_file);
    } else {
        while (next_s_record_no != -1) {
            int s_record_no = next_s_record_no;
            fseek(s_data_file,
                  sizeof(struct DataMeta) + (s_record_no) * (sizeof(struct Materials) + sizeof(bool)) +
                  (s_record_no - 1) * sizeof(int),
                  SEEK_SET
            );
            fread(&next_s_record_no, sizeof(int), 1, s_data_file);
        }
        fseek(s_data_file, (long) -sizeof(int), SEEK_CUR);
        fwrite(&s_meta.size_valid, sizeof(int), 1, s_data_file);
    }
    fclose(m_data_file);
    fclose(s_data_file);
    return 0;
}

int update_m(unsigned id, const char name[32], const char type[32], const char country[32]) {
    int record_no = get_m_record_no(id);
    if (record_no == -1) {
        return 1;
    }
    struct Instrument instrument;
    FILE *m_data_file = fopen(M_DATA_FILENAME, "rb+");
    fseek(m_data_file,
          sizeof(struct DataMeta) + (record_no - 1) * (sizeof(struct Instrument) + sizeof(bool) + sizeof(int)),
          SEEK_SET
    );
    fread(&instrument, sizeof(struct Instrument), 1, m_data_file);
    strcpy(instrument.name, name);
    strcpy(instrument.type,type);
    strcpy(instrument.country, country);
    fseek(m_data_file, (long) -sizeof(struct Instrument), SEEK_CUR);
    fwrite(&instrument, sizeof(struct Instrument), 1, m_data_file);
    fclose(m_data_file);
    return 0;
}

int update_s_at_line(int record_no, const char material[32], float amount) {
    if (record_no == -1) {
        return 1;
    }

    struct Materials inf;
    FILE *s_data_file = fopen(S_DATA_FILENAME, "rb+");
    fseek(s_data_file,
          sizeof(struct DataMeta) + (record_no - 1) * (sizeof(struct Materials) + sizeof(bool) + sizeof(int)),
          SEEK_SET
    );
    fread(&inf, sizeof(struct Materials), 1, s_data_file);
    strcpy(inf.material, material);
   inf.amount=amount;
    fseek(s_data_file, (long) -sizeof(struct Materials), SEEK_CUR);
    fwrite(&inf, sizeof(struct Materials), 1, s_data_file);
    fclose(s_data_file);
    return 0;
}

int update_s(unsigned id, const char material[32], float amount) {
    return update_s_at_line(get_s_record_no(id), material, amount);
}

int update_s_of_m(unsigned m_id, unsigned id, const char material[32], float amount) {
    return update_s_at_line(get_s_of_m_record_no(m_id, id), material, amount);
}

int del_m(unsigned id) {
    if (m_index.size == 0) {
        return 1;
    }
    int record_no = -1;
    int s_record_no;
    unsigned m_index_no;
    bool valid = false;
    unsigned left = 0;
    unsigned right = m_index.size - 1;
    while (left <= right) {
        unsigned mid = (left + right) / 2;
        if (m_index.data[mid].id > id) {
            if (mid != 0) {
                right = mid - 1;
            } else {
                break;
            }
        } else if (m_index.data[mid].id < id) {
            if (mid != m_index.size - 1) {
                left = mid + 1;
            } else {
                break;
            }
        } else {
            record_no = (int) m_index.data[mid].record_no;
            m_index_no = mid;
            break;
        }
    }
    if (record_no == -1) {
        return 1;
    }
    FILE *m_data_file = fopen(M_DATA_FILENAME, "rb+");
    fseek(m_data_file,
          sizeof(struct DataMeta) + (record_no) * sizeof(struct Instrument) +
          (record_no - 1) * (sizeof(bool) + sizeof(int)),
          SEEK_SET
    );
    fwrite(&valid, sizeof(bool), 1, m_data_file);
    fseek(m_data_file,
          sizeof(struct DataMeta) + (record_no) * (sizeof(struct Instrument) + sizeof(bool)) +
          (record_no - 1) * (sizeof(int)),
          SEEK_SET
    );
    fread(&s_record_no, sizeof(int), 1, m_data_file);
    FILE *s_data_file = fopen(S_DATA_FILENAME, "rb+");
    while (s_record_no != -1) {
        fseek(s_data_file,
              sizeof(struct DataMeta) + (s_record_no) * sizeof(struct Materials) +
              (s_record_no - 1) * (sizeof(bool) + sizeof(int)),
              SEEK_SET
        );
        fwrite(&valid, sizeof(bool), 1, s_data_file);
        fseek(s_data_file,
              sizeof(struct DataMeta) + (s_record_no) * (sizeof(struct Materials) + sizeof(bool)) +
              (s_record_no - 1) * sizeof(int),
              SEEK_SET
        );
        fread(&s_record_no, sizeof(int), 1, s_data_file);
    }
    fclose(m_data_file);
    fclose(s_data_file);
    for (unsigned i = m_index_no; i < m_index.size - 1; i++) {
        m_index.data[i] = m_index.data[i + 1];
    }
    m_index.size--;
    return 0;
}

int del_s_at_line(int record_no) {
    if (record_no == -1) {
        return 1;
    }
    bool valid;
    int floating_s_record_no;
    int next_s_record_no;
    FILE *s_data_file = fopen(S_DATA_FILENAME, "rb+");
    fseek(s_data_file,
          sizeof(struct DataMeta) + (record_no) * sizeof(struct Materials) +
          (record_no - 1) * (sizeof(bool) + sizeof(int)),
          SEEK_SET
    );
    fread(&valid, sizeof(bool), 1, s_data_file);
    if (!valid) {
        return 1;
    }
    fseek(s_data_file,
          sizeof(struct DataMeta) + (record_no) * sizeof(struct Materials) +
          (record_no - 1) * (sizeof(bool) + sizeof(int)),
          SEEK_SET
    );
    valid = false;
    fwrite(&valid, sizeof(bool), 1, s_data_file);
    fread(&floating_s_record_no, sizeof(int), 1, s_data_file);
    fseek(s_data_file, 0, SEEK_SET);
    struct DataMeta s_meta;
    fread(&s_meta, sizeof(struct DataMeta), 1, s_data_file);
    for (int i = 0; i < s_meta.size_valid; i++) {
        fseek(s_data_file, sizeof(struct Materials), SEEK_CUR);
        fread(&valid, sizeof(bool), 1, s_data_file);
        fread(&next_s_record_no, sizeof(int), 1, s_data_file);
        if ((next_s_record_no == record_no) && valid) {
            fseek(s_data_file, (long) -sizeof(int), SEEK_CUR);
            fwrite(&floating_s_record_no, sizeof(int), 1, s_data_file);
            fclose(s_data_file);
            return 0;
        }
    }
    fclose(s_data_file);
    FILE *m_data_file = fopen(M_DATA_FILENAME, "rb+");
    struct DataMeta m_meta;
    fread(&m_meta, sizeof(struct DataMeta), 1, m_data_file);
    for (int i = 0; i < m_meta.size_valid; i++) {
        fseek(m_data_file, sizeof(struct Instrument) + sizeof(bool), SEEK_CUR);
        fread(&next_s_record_no, sizeof(int), 1, m_data_file);
        if (next_s_record_no == record_no) {
            fseek(m_data_file, (long) -sizeof(int), SEEK_CUR);
            fwrite(&floating_s_record_no, sizeof(int), 1, m_data_file);
            fclose(m_data_file);
            return 0;
        }
    }

    return 0;
}

int del_s(unsigned id) {
    return del_s_at_line(get_s_record_no(id));
}

int del_s_of_m(unsigned m_id, unsigned id) {
    return del_s_at_line(get_s_of_m_record_no(m_id, id));
}

unsigned size_m() {
    FILE *m_data_file = fopen(M_DATA_FILENAME, "rb");
    struct DataMeta m_meta;
    fread(&m_meta, sizeof(struct DataMeta), 1, m_data_file);
    unsigned res = 0;
    for (unsigned i = 0; i < m_meta.size_valid; i++) {
        struct Instrument instrument;
        bool valid;
        fread(&instrument, sizeof(struct Instrument), 1, m_data_file);
        fread(&valid, sizeof(bool), 1, m_data_file);
        fseek(m_data_file, sizeof(int), SEEK_CUR);
        if (valid) {
            res++;
        }
    }
    fclose(m_data_file);
    return res;
}

unsigned size_s() {
    FILE *s_data_file = fopen(S_DATA_FILENAME, "rb");
    struct DataMeta s_meta;
    fread(&s_meta, sizeof(struct DataMeta), 1, s_data_file);
    unsigned res = 0;
    for (unsigned i = 0; i < s_meta.size_valid; i++) {
        struct Materials inf;
        bool valid;
        fread(&inf, sizeof(struct Materials), 1, s_data_file);
        fread(&valid, sizeof(bool), 1, s_data_file);
        fseek(s_data_file, sizeof(int), SEEK_CUR);
        if (valid) {
            res++;
        }
    }
    fclose(s_data_file);
    return res;
}

int size_s_of_m(unsigned m_id) {
    unsigned m_record_no = get_m_record_no(m_id);
    if (m_record_no == -1) {
        return -1;
    }
    FILE *m_data_file = fopen(M_DATA_FILENAME, "rb");
    FILE *s_data_file = fopen(S_DATA_FILENAME, "rb");
    int s_record_no;
    int res = 0;
    fseek(m_data_file,
          sizeof(struct DataMeta) + (m_record_no) * (sizeof(struct Instrument) + sizeof(bool)) +
          (m_record_no - 1) * sizeof(int),
          SEEK_SET
    );
    fread(&s_record_no, sizeof(int), 1, m_data_file);
    while (s_record_no != -1) {
        res++;
        fseek(s_data_file,
              sizeof(struct DataMeta) + (s_record_no - 1) * (sizeof(struct Materials) + sizeof(bool) + sizeof(int)),
              SEEK_SET
        );
        fseek(s_data_file, sizeof(struct Materials) + sizeof(bool), SEEK_CUR);
        fread(&s_record_no, sizeof(int), 1, s_data_file);
    }

    fclose(m_data_file);
    fclose(s_data_file);
    return res;
}

void ut_m(bool print_removed) {
    FILE *m_data_file = fopen(M_DATA_FILENAME, "rb");
    struct DataMeta m_meta;
    fread(&m_meta, sizeof(struct DataMeta), 1, m_data_file);
    printf("TOTAL NUMBER OF RECORDS IN FILE: %d\n", m_meta.size);
    printf("SIZE OF VALID BLOCK: %d\n", m_meta.size_valid);
    printf("MAX ID: %d \n", m_meta.max_id);
    for (unsigned i = 0; i < m_meta.size_valid; i++) {
        struct Instrument instrument;
        int s_record_no;
        bool valid;
        fread(&instrument, sizeof(struct Instrument), 1, m_data_file);
        fread(&valid, sizeof(bool), 1, m_data_file);
        fread(&s_record_no, sizeof(int), 1, m_data_file);
        if (valid || print_removed) {
            printf("%d)\n", i + 1);
            printf("\t id: %d \n", instrument.id);
            printf("\t name: %s \n", instrument.name);
            printf("\t type: %s \n", instrument.type);
            printf("\t country: %s \n", instrument.country);
            printf("\t [first s record no.: %d] \n", s_record_no);
            if (print_removed) {
                printf("\t [state: %s] \n", (valid) ? "valid" : "deleted");
            }
        }
    }
    fclose(m_data_file);
}

void ut_s(bool print_removed) {
    FILE *s_data_file = fopen(S_DATA_FILENAME, "rb");
    struct DataMeta s_meta;
    fread(&s_meta, sizeof(struct DataMeta), 1, s_data_file);
    printf("TOTAL NUMBER OF RECORDS IN FILE: %d\n", s_meta.size);
    printf("SIZE OF VALID BLOCK: %d\n", s_meta.size_valid);
    printf("MAX ID: %d \n", s_meta.max_id);
    for (unsigned i = 0; i < s_meta.size_valid; i++) {
        struct Materials inf;
        int next_s_record_no;
        bool valid;
        fread(&inf, sizeof(struct Materials), 1, s_data_file);
        fread(&valid, sizeof(bool), 1, s_data_file);
        fread(&next_s_record_no, sizeof(int), 1, s_data_file);
        if (valid || print_removed) {
            printf("%d)\n", i + 1);
            printf("\t id: %d \n", inf.id);
            printf("\t material: %s \n", inf.material);
            printf("\t amount: %f \n", inf.amount);
            printf("\t [next s record no.: %d] \n", next_s_record_no);
            if (print_removed) {
                printf("\t [state: %s] \n", (valid) ? "valid" : "deleted");
            }
        }
    }

    fclose(s_data_file);
}

void defragment_m() {
    FILE *m_data_file = fopen(M_DATA_FILENAME, "rb+");
    struct DataMeta m_meta;
    fread(&m_meta, sizeof(struct DataMeta), 1, m_data_file);
    struct Instrument *instrument = malloc(sizeof(struct Instrument));
    bool valid;
    int next_s_record_no;
    unsigned curr_size_valid = 0;
    for (unsigned i = 0; i < m_meta.size_valid; i++) {
        fseek(m_data_file,
              sizeof(struct DataMeta) + i * (sizeof(struct Instrument) + sizeof(bool) + sizeof(int)),
              SEEK_SET
        );
        fread(instrument, sizeof(struct Instrument), 1, m_data_file);
        fread(&valid, sizeof(bool), 1, m_data_file);
        fread(&next_s_record_no, sizeof(int), 1, m_data_file);
        if (valid) {
            curr_size_valid++;
            if (curr_size_valid - 1 != i) {
                fseek(m_data_file,
                      sizeof(struct DataMeta) +
                      (curr_size_valid - 1) * (sizeof(struct Instrument) + sizeof(bool) + sizeof(int)),
                      SEEK_SET
                );
                fwrite(instrument, sizeof(struct Instrument), 1, m_data_file);
                fwrite(&valid, sizeof(bool), 1, m_data_file);
                fwrite(&next_s_record_no, sizeof(int), 1, m_data_file);
            }
        }
    }
    m_meta.size_valid = curr_size_valid;
    fseek(m_data_file, 0, SEEK_SET);
    fwrite(&m_meta, sizeof(struct DataMeta), 1, m_data_file);
    fclose(m_data_file);
    for (unsigned i = 0; i < m_index.size; i++) {
        m_index.data[i].record_no = i + 1;
    }
}

void defragment_s() {
    FILE *s_data_file = fopen(S_DATA_FILENAME, "rb+");
    struct DataMeta s_meta;
    fread(&s_meta, sizeof(struct DataMeta), 1, s_data_file);
    FILE *m_data_file = fopen(M_DATA_FILENAME, "rb+");
    struct DataMeta m_meta;
    fread(&m_meta, sizeof(struct DataMeta), 1, m_data_file);
    struct Materials *inf = malloc(sizeof(struct Materials));
    bool valid;
    int next_s_record_no;
    unsigned curr_size_valid = 0;
    for (unsigned i = 0; i < s_meta.size_valid; i++) {
        fseek(s_data_file,
              sizeof(struct DataMeta) + i * (sizeof(struct Materials) + sizeof(bool) + sizeof(int)),
              SEEK_SET
        );
        fread(inf, sizeof(struct Materials), 1, s_data_file);
        fread(&valid, sizeof(bool), 1, s_data_file);
        fread(&next_s_record_no, sizeof(int), 1, s_data_file);
        if (valid) {
            curr_size_valid++;
            if (curr_size_valid - 1 != i) {
                fseek(s_data_file,
                      sizeof(struct DataMeta) +
                      (curr_size_valid - 1) * (sizeof(struct Materials) + sizeof(bool) + sizeof(int)),
                      SEEK_SET
                );
                fwrite(inf, sizeof(struct Materials), 1, s_data_file);
                fwrite(&valid, sizeof(bool), 1, s_data_file);
                fwrite(&next_s_record_no, sizeof(int), 1, s_data_file);
            }
            bool flag = false;
            fseek(s_data_file, sizeof(struct DataMeta), SEEK_SET);
            for (unsigned j = 0; j < i; j++) {
                fseek(s_data_file, sizeof(struct Materials) + sizeof(bool), SEEK_CUR);
                fread(&next_s_record_no, sizeof(int), 1, s_data_file);
                if (next_s_record_no == i + 1) {
                    fseek(s_data_file, (long) -sizeof(int), SEEK_CUR);
                    fwrite(&curr_size_valid, sizeof(int), 1, s_data_file);
                    flag = true;
                    break;
                }
            }
            if (flag) {
                continue;
            }
            fseek(m_data_file, sizeof(struct DataMeta), SEEK_SET);
            for (unsigned j = 0; j < m_meta.size_valid; j++) {
                fseek(m_data_file, sizeof(struct Instrument) + sizeof(bool), SEEK_CUR);
                fread(&next_s_record_no, sizeof(int), 1, m_data_file);
                if (next_s_record_no == i + 1) {
                    fseek(m_data_file, (long) -sizeof(int), SEEK_CUR);
                    fwrite(&curr_size_valid, sizeof(int), 1, m_data_file);
                    break;
                }
            }
        }
    }

    s_meta.size_valid = curr_size_valid;
    fseek(s_data_file, 0, SEEK_SET);
    fwrite(&s_meta, sizeof(struct DataMeta), 1, s_data_file);
    fclose(m_data_file);
    fclose(s_data_file);
}

