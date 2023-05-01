#ifndef _light_hpp_INCLUDED
#define _light_hpp_INCLUDED

#include "utils/paras.hpp"
#include "utils/parse.hpp"
#include "preprocess/preprocess.hpp"
#include <atomic>
#include <iostream>
#include <boost/thread.hpp>
#include <boost/thread/thread.hpp>
#include <boost/lockfree/spsc_queue.hpp>
typedef long long ll;

class basesolver;
class sharer;

extern std::atomic<int> terminated;
extern std::mutex mtx;

enum Solver_Type {KISSAT};

struct thread_inf{
    int id, inf;
    bool operator < ( const thread_inf &other ) const
	{
		return inf > other.inf;
	}
};

struct light
{
public:
    light();
    ~light();

    char *filename;
    paras *opt;
    preprocess *pre;
    vec<int> solver_type;
    vec<basesolver *> workers;
    vec<sharer *> sharers;
    
    vec<char*> *configure_name;
    vec<double> *configure_val;

    int finalResult;
    int winner_period, winner_id;
    mutable boost::mutex winner_mtx;
    int maxtime;
    void update_winner(int id, int period) {
        boost::mutex::scoped_lock lock(winner_mtx);
        if (period < winner_period || (period == winner_period && id < winner_id)) {
            winner_period = period;
            winner_id = id;
        }
    }
    int  get_winner_period() {
        boost::mutex::scoped_lock lock(winner_mtx);
        return winner_period;
    }
    void arg_parse(int argc, char **argv);
    void configure_from_file(const char* file);
    void init_workers();
    void diversity_workers();
    void parse_input();
    int  run();
    void share();
    int  solve();
    void terminate_workers();
    void print_model();
};

#endif