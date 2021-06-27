#include "bst.hpp"
#include <x86intrin.h>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstring>
#include <chrono>
#include <random>
#include <omp.h>

#define MAX_SIZE 65536
//#define INITIAL MAX_SIZE * 2
#define MAX_THREAD 88
const long TOTAL = ((long) 1 << 26);

using namespace std;
using namespace std::chrono;

#ifdef STM
typedef struct stm_data
{
    unsigned long nb_aborts;
    unsigned long nb_aborts_1;
    unsigned long nb_aborts_2;
    unsigned long nb_aborts_locked_read;
    unsigned long nb_aborts_locked_write;
    unsigned long nb_aborts_validate_read;
    unsigned long nb_aborts_validate_write;
    unsigned long nb_aborts_validate_commit;
    unsigned long nb_aborts_invalid_memory;
    unsigned long nb_aborts_killed;
    unsigned long locked_reads_ok;
    unsigned long locked_reads_failed;
    unsigned long max_retries;
} stm_data_t;
#endif

int main(int argch, char **argv)
{
    long INITIAL = MAX_SIZE * atoi(argv[2]);

    string *input = new string[TOTAL + INITIAL];
    int64_t *latency = new int64_t[TOTAL + INITIAL];

    // Read file with the input values
    string fname(argv[1]);
    ifstream f;
    f.open(fname);
    if (f.is_open())
    {
        string line;
        int idx = 0;
        while(getline(f, line) && idx < (TOTAL + INITIAL))
        {
            input[idx] = line;
            idx++;
        }
    } else return 1;
    f.close();

    // Create BST
    BST *ht = new BST();

#ifdef STM
    // Tiny STM initialize stuff
    TM_INIT;
    TM_INIT_THREAD;
#endif
    for (long i = 0; i < INITIAL; i++)
    {
        // Add initial population
        string aux = input[i];
        string type = aux.substr(0, aux.find(' '));
        aux.erase(0, aux.find(' ') + 1);
        if (aux == "") continue;
        int key = stoi(aux.substr(0, aux.find(' ')));
        ht->add(key);
    }

#ifdef STM
    // Tiny STM finalization stuff
    TM_EXIT_THREAD;
    
    // TinySTM get data stuff
    stm_data_t **dd = new stm_data_t*[MAX_THREAD];
    for (int i = 0; i < MAX_THREAD; i++)
    {
        dd[i] = new stm_data_t;
        memset(dd[i], 0, sizeof(stm_data_t));
    }
#endif

    auto start = high_resolution_clock::now();
    cout << "Start" << endl;

    long g = 0, a = 0, d = 0;
#pragma omp parallel
    {
#ifdef STM
        TM_INIT_THREAD;
#endif

#pragma omp for reduction(+:g, a, d)
        for (long i = INITIAL; i < (TOTAL + INITIAL); i++)
        {
            string aux = input[i];
            string type = aux.substr(0, aux.find(' '));
            aux.erase(0, aux.find(' ') + 1);
            if (aux == "") continue;
            int key = stoi(aux.substr(0, aux.find(' ')));

            unsigned int reg;
            if (type == "g")
            {
                // Get operation
#ifdef LAT
                // Start latency
                int64_t start_local = _rdtscp(&reg);
#endif
                ht->get(key);
#ifdef LAT
                // End latency
                int64_t finish = _rdtscp(&reg);
                // Calculate and save latency
                latency[i] = finish - start_local;
#endif
                g++;
            } else if (type == "i")
            {
                // Insert peration
#ifdef LAT
                // Start latency
                int64_t start_local = _rdtscp(&reg);
#endif
                ht->add(key);
#ifdef LAT
                // End latency
                int64_t finish = _rdtscp(&reg);
                // Calculate and save latency
                latency[i] = finish - start_local;
#endif
                a++;
            } else if (type == "d")
            {
                // Delete operation
#ifdef LAT
                // Start latency
                int64_t start_local = _rdtscp(&reg);
#endif
                ht->del(key);
#ifdef LAT
                // End latency
                int64_t finish = _rdtscp(&reg);
                // Calculate and save latency
                latency[i] = finish - start_local;
#endif
                d++;
            }
        }
#ifdef STM
        stm_get_stats("nb_aborts", &dd[omp_get_thread_num()]->nb_aborts);
        stm_get_stats("nb_aborts_1", &dd[omp_get_thread_num()]->nb_aborts_1);
        stm_get_stats("nb_aborts_2", &dd[omp_get_thread_num()]->nb_aborts_2);
        stm_get_stats("nb_aborts_locked_read", &dd[omp_get_thread_num()]->nb_aborts_locked_read);
        stm_get_stats("nb_aborts_locked_write", &dd[omp_get_thread_num()]->nb_aborts_locked_write);
        stm_get_stats("nb_aborts_validate_read", &dd[omp_get_thread_num()]->nb_aborts_validate_read);
        stm_get_stats("nb_aborts_validate_write", &dd[omp_get_thread_num()]->nb_aborts_validate_write);
        stm_get_stats("nb_aborts_validate_commit", &dd[omp_get_thread_num()]->nb_aborts_validate_commit);
        stm_get_stats("nb_aborts_invalid_memory", &dd[omp_get_thread_num()]->nb_aborts_invalid_memory);
        stm_get_stats("nb_aborts_killed", &dd[omp_get_thread_num()]->nb_aborts_killed);
        stm_get_stats("locked_reads_ok", &dd[omp_get_thread_num()]->locked_reads_ok);
        stm_get_stats("locked_reads_failed", &dd[omp_get_thread_num()]->locked_reads_failed);
        stm_get_stats("max_retries", &dd[omp_get_thread_num()]->max_retries);
        TM_EXIT_THREAD;
#endif
    }

    auto end = high_resolution_clock::now();

    cout << "Get: " << g << " Ins: " << a << " Del: " << d << " Total: ";
    cout << g + a + d << endl;

    // Get time
    auto duration = duration_cast<milliseconds>(end - start);
    cout << "Time (ms): " << duration.count() << endl;
#ifdef HTM
    ht->printHTM();
#endif

#ifdef LAT
    for (long i = INITIAL; i < INITIAL + TOTAL; i++)
    {
        cout << latency[i] << endl;
    }
#endif
//}
}
