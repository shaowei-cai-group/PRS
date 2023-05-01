#ifndef _clause_hpp_INCLUDED
#define _clause_hpp_INCLUDED
#include <boost/thread.hpp>
#include <boost/thread/thread.hpp>

struct clause_store {
    int size, lbd;
    int *data;
    std::atomic<int> refs;
    clause_store(int sz) {
        size = sz;
        data = (int*) malloc(sizeof(int) * sz);
        lbd = 0;
        refs = 1;
    }
    void increase_refs(int inc) {
        refs += inc;
    }
    bool free_clause() {
        int ref = refs.fetch_sub(1);
        if (ref <= 1) {
            // for (int i = 0; i < size; i++)
            //     printf("%d ", data[i]);
            // puts("");
            free(data);
            return true;
        }
        return false;
    }
    ~clause_store() {
        puts("free");
    }
};


struct period_clauses {
    int period;
    std::atomic<int> refs;
    vec<clause_store *> cls;
    period_clauses(int p = 0) {
        refs = 0;
        period = p;
    }

    void increase_refs(int inc) {
        refs += inc;
    }

    bool free_clause() {
        int ref = refs.fetch_sub(1);
        if (ref <= 1) {
            if (ref <= 0) puts("**************************========================");
            return true;
        }
        return false;
    }
    void release_clause() {
        for (int i = 0; i < cls.size(); i++)
            cls[i]->free_clause();
    }
    void push(clause_store * c) {
        cls.push(c);
    }
};

struct period_queue {
    std::deque<period_clauses *> q;
    mutable boost::mutex mtx;
    void push(period_clauses * c) {
        boost::mutex::scoped_lock lock(mtx);
        q.push_back(c);
    }
    void pop(int assert_period = -1) {
        boost::mutex::scoped_lock lock(mtx);
        if (assert_period != -1 && q.front()->period != assert_period) printf("c ............false free pos\n");
        q.front()->release_clause();
        q.pop_front();
    }
    period_clauses* find(int period) {
        boost::mutex::scoped_lock lock(mtx);
        int sp = q.front()->period;
        if (period - sp >= q.size()) puts("may occur wrong");
        return q[period - sp];
    }
};

#endif