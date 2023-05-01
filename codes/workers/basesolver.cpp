#include "basesolver.hpp"
#include "sharer.hpp"

int basesolver::get_period() {
    boost::mutex::scoped_lock lock(mtx);
    return period;
};
void basesolver::inc_period() {
    boost::mutex::scoped_lock lock(mtx);
    period++;
    if (period % 100 == 0) printf("c %d reach period %d\n", id, period);
    cond.notify_all();
};

void basesolver::internal_terminate() {
    boost::mutex::scoped_lock lock(mtx);
    terminate_period = period;
    cond.notify_all();
}

void basesolver::external_terminate() {
    {
        boost::mutex::scoped_lock lock(mtx);
        terminated = 1;
        cond.notify_all();
        // printf("c thread %d terminated\n", id);
    }
    terminate();
}

void basesolver::broaden_export_limit() {
    ++good_clause_lbd;
}

void basesolver::restrict_export_limit() {
    if (good_clause_lbd > 2)
        --good_clause_lbd;
}

double basesolver::get_waiting_time() {
    return waiting_time;
}


void basesolver::export_clauses_to(vec<clause_store *> &clauses) {
    clause_store *cls;
    while (export_clause.pop(cls)) {
        clauses.push(cls);
    }
}

void basesolver::import_clauses_from(vec<clause_store *> &clauses) {
    for (int i = 0; i < clauses.size(); i++) {
        import_clause.push(clauses[i]);
    }
}

void basesolver::select_clauses() {
    sharer *share = in_sharer;
    cls.clear();
    export_clauses_to(cls);

    for (int i = 0; i < cls.size(); i++) {
        int sz = cls[i]->size;
        while (sz > bucket.size()) bucket.push();
        if (sz * (bucket[sz - 1].size() + 1) <= share->share_lits) 
            bucket[sz - 1].push(cls[i]);
    }
    period_clauses *pcls = new period_clauses(period);
    int space = share->share_lits;
    for (int i = 0; i < bucket.size(); i++) {
        int clause_num = space / (i + 1);
        if (!clause_num) break;
        if (clause_num >= bucket[i].size()) {
            space -= bucket[i].size() * (i + 1);
            for (int j = 0; j < bucket[i].size(); j++)
                pcls->push(bucket[i][j]);
            bucket[i].clear();
        }
        else {
            space -= clause_num * (i + 1);
            for (int j = 0; j < clause_num; j++) {
                pcls->push(bucket[i].last());
                bucket[i].pop();
            }
        }
    }
    int percent = (share->share_lits - space) * 100 / share->share_lits;
    if (percent < 75) broaden_export_limit();
    if (percent > 98) restrict_export_limit();
    //sort finish
    pcls->increase_refs(share->producers.size() - 1);
    pq.push(pcls);
}


void basesolver::get_model(vec<int> &model) {
    model.clear();
    for (int i = 1; i <= maxvar; i++) {
        model.push(val(i));
    }   
}

void cbk_start_new_period(void *solver) {
    auto clk_st = std::chrono::high_resolution_clock::now();
    basesolver* S = (basesolver *) solver;
    sharer *share = S->in_sharer;
    int should_free = 1;
 
    if (S->period >= S->controller->get_winner_period()) {
        S->internal_terminate();
        S->terminate();
        return;
    }
    // printf("c thread %d into select %d\n", S->id, S->period);
    S->select_clauses();
    // printf("c %d now finish period %d\n", S->id, S->get_period());
    S->inc_period();
    // printf("c thread %d start import at %d\n", S->id, S->period);
    should_free = share->import_clauses(S->id);
    // printf("c thread %d finish import at %d\n", S->id, S->period);
    if (should_free == -1) {
        S->internal_terminate();
        S->terminate();
    }
    auto clk_now = std::chrono::high_resolution_clock::now();
    int solve_time = std::chrono::duration_cast<std::chrono::milliseconds>(clk_now - clk_st).count();
    S->waiting_time += 0.001 * solve_time;
    // printf("c %d wait for %d.%03ds\n", S->id, solve_time / 1000, solve_time % 1000);
    // printf("c %d finish sharing\n", S->id);
}

void cbk_free_clauses(void *solver) {
    basesolver* S = (basesolver *) solver;
    sharer *share = S->in_sharer;
    // if (S->unfree.size()) printf("free %d period\n", S->unfree.size());
    for (int id = 0; id < S->unfree.size(); id++) {
        int import_period = S->unfree[id];
        // int current_period = S->get_period() - 1, import_period = current_period - share->margin;
        // if (import_period < 0) return;
        for (int i = 0; i < share->producers.size(); i++) {
            if (i == S->id) continue;
            basesolver *s = share->producers[i];
            period_clauses *pc = s->pq.find(import_period);
            if (pc->period != import_period)
                puts("*******************************************************");
            if (pc->free_clause()) {
                s->pq.pop(import_period);
                // printf("thread %d enter, %d is free\n", S->id, i);
            }
        }
    }
    S->unfree.clear();
}

