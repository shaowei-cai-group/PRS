#ifndef _sharer_hpp_INCLUDED
#define _sharer_hpp_INCLUDED
#include "../utils/paras.hpp"
#include <boost/thread.hpp>

class basesolver;
class sharer {
public:
    int id;
    int share_intv, share_lits, dps;
    int margin;
    mutable boost::mutex mtx;
    boost::condition_variable cond;
    int terminated;
    int waitings;
    vec<vec<clause_store *>> bucket[64]; //need to update
    
    vec<basesolver *> producers, consumers;
    vec<clause_store *> cls;
    sharer(int idx, int intv, int lits, int share_dps = 0) {
        share_intv = intv;
        share_lits = lits;
        dps = share_dps;
        id = idx;
        waitings = terminated = 0;
    }
    void set_terminated() {
        boost::mutex::scoped_lock lock(mtx);
        terminated = 1;
        cond.notify_one();
    }
    bool should_sharing() const {
        boost::mutex::scoped_lock lock(mtx);
        return waitings == producers.size();
    }
    void waiting_for_all_ready() {
        boost::mutex::scoped_lock lock(mtx);
        while (waitings != producers.size() && !terminated) {
            cond.wait(lock);
        }
    }
    void sharing_finish() {
        boost::mutex::scoped_lock lock(mtx);
        waitings = 0;
        // printf("c sharing thread finish sharing\n");
        cond.notify_all();
    }
    int sort_clauses(int x);
    // void select_clauses(int id);
    int import_clauses(int id);
};

#endif