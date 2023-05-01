#ifndef _basesolver_hpp_INCLUDED
#define _basesolver_hpp_INCLUDED

#include "../light.hpp"
#include "../utils/vec.hpp"
#include "clause.hpp"
#include <fstream>
#include <iostream>
#include <boost/thread.hpp>
#include <boost/thread/thread.hpp>
#include <boost/lockfree/spsc_queue.hpp>

class sharer;
class basesolver {
public:
    virtual void add(int l) = 0;
    virtual int  solve() = 0;
    virtual int  val(int l) = 0;
    virtual void terminate() = 0;
    virtual void configure(const char* name, int id) = 0;
    virtual int  get_conflicts() = 0;

    virtual void parse_from_CNF(char* filename) = 0;
    virtual void parse_from_PAR(preprocess* pre) = 0;
    virtual void exp_clause(void *cl, int lbd) = 0;
    virtual bool imp_clause(clause_store *cls, void *cl) = 0;
    
    void export_clauses_to(vec<clause_store *> &clauses);
    void import_clauses_from(vec<clause_store *> &clauses);

    void get_model(vec<int> &model);
    void select_clauses();
    void broaden_export_limit();
    void restrict_export_limit();
    double get_waiting_time();
    int  get_period();
    void inc_period();
    void internal_terminate();
    void external_terminate();

    double waiting_time = 0;
    int good_clause_lbd = 0;
    light * controller;
    int id;
    sharer* in_sharer;
    vec<int> model;

    int terminate_period = 1e9;
    int maxvar, period, margin, terminated = 0;
    mutable boost::mutex mtx;
    boost::condition_variable cond;
    period_queue pq;
    vec<clause_store *> cls;
    vec<vec<clause_store *>> bucket;
    vec<int> unfree;

    boost::lockfree::spsc_queue<clause_store*, boost::lockfree::capacity<1024000>> import_clause;
    boost::lockfree::spsc_queue<clause_store*, boost::lockfree::capacity<1024000>> export_clause;

    basesolver(int sid, light* light) : id(sid), controller(light) {
        good_clause_lbd = 2;
        period = 0;
        margin = 0;
    }

    ~basesolver() {
        if (controller) {
            controller = NULL;
        }
    }
    friend void cbk_start_new_period(void *);
    friend void cbk_free_clauses(void *);
};

void cbk_start_new_period(void *);
void cbk_free_clauses(void *);

#endif