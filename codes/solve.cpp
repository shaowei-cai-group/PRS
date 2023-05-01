#include "light.hpp"
#include "workers/basekissat.hpp"
#include "workers/sharer.hpp"
#include <unistd.h>
#include <chrono>
#include <algorithm>
#include <mutex>
auto clk_st = std::chrono::high_resolution_clock::now();
char* worker_sign = "";

std::atomic<int> terminated;
int result = 0;
int winner_conf;
vec<int> model;

void * read_worker(void *arg) {
   basesolver * sq = (basesolver *)arg;
    if (worker_sign == "") 
      sq->parse_from_PAR(sq->controller->pre);
   else 
      sq->parse_from_CNF(worker_sign);
   return NULL;
}

void * solve_worker(void *arg) {
   basesolver * sq = (basesolver *)arg;
   while (!terminated) {
        int res = sq->solve();
        if (sq->controller->opt->DCE) {
            printf("c %d solved, res is %d\n", sq->id, res);
            if (res) {
                terminated = 1;
                result = res; 
                // printf("c %d solved 1\n", sq->id);
                sq->internal_terminate();
                // printf("c %d solved 2\n", sq->id);
                sq->controller->update_winner(sq->id, sq->period);
                // printf("c %d solved 3\n", sq->id);
                if (res == 10) sq->get_model(sq->model);
            }
            printf("c %d really solved, period is %d\n", sq->id, sq->period);
        }
        else {
            if (res && !terminated) {
                printf("c result: %d, winner is %d, winner run %d confs\n", res, sq->id, sq->get_conflicts());
                terminated = 1;
                sq->controller->terminate_workers();
                result = res;
                sq->controller->update_winner(sq->id, 0);
                winner_conf = sq->get_conflicts();
                if (res == 10) sq->get_model(sq->model);
            }
            printf("c get result %d with res %d\n", sq->id, res);
        }
   }
   return NULL;
}

void light::init_workers() {
    terminated = 0;
    for (int i = 0; i < OPT(nThreads); i++) {
        basekissat* kissat = new basekissat(i, this);
        workers.push(kissat);
    }
}

void light::diversity_workers() {
    for (int i = 0; i < OPT(nThreads); i++) {
        if (OPT(shuffle)) {
            if (i) workers[i]->configure("order_reset", i);
        }
        if (OPT(pakis)) {
            if (i == 13 || i == 14 || i == 20 || i == 21)
                workers[i]->configure("tier1", 3);
            else
                workers[i]->configure("tier1", 2);

            if (i == 3 || i == 4 || i == 6 || i == 8 || i == 11 || i == 12 || i == 13 || i == 14 || i == 16 || i == 18 || i == 23)
                workers[i]->configure("chrono", 0);
            else
                workers[i]->configure("chrono", 1);    

            if (i == 2 || i == 23)
                workers[i]->configure("stable", 0);
            else if (i == 6 || i == 16)
                workers[i]->configure("stable", 2);
            else
                workers[i]->configure("stable", 1);

            if (i == 10 || i == 22)
                workers[i]->configure("walkinitially", 1);
            else
                workers[i]->configure("walkinitially", 0);

            if (i == 7 || i == 8 || i == 9 || i == 17 || i == 18 || i == 19 || i == 20)
                workers[i]->configure("target", 0);
            else if (i == 0 || i == 2 || i == 3 || i == 4 || i == 5 || i == 6 || i == 10 || i == 23)
                workers[i]->configure("target", 1);
            else
                workers[i]->configure("target", 2);
            
            if (i == 4 || i == 5 || i == 8 || i == 9 || i == 12 || i == 13 || i == 15 || i == 18 || i == 19)
                workers[i]->configure("phase", 0);
            else
                workers[i]->configure("phase", 1);
        }
        for (int j = 0; j < configure_name[i].size(); j++) {
            workers[i]->configure(configure_name[i][j], configure_val[i][j]);
        }
    }
}

void light::terminate_workers() {
    printf("c controller reach limit\n");
    for (int i = 0; i < OPT(nThreads); i++) {
        if (OPT(clause_sharing) == 1 && OPT(DCE) == 1) 
            workers[i]->external_terminate();
        else 
            workers[i]->terminate();
    }
    for (int i = 0; i < sharers.size(); i++) {
        sharers[i]->set_terminated();
    }
}

void light::parse_input() {
    pthread_t *ptr = new pthread_t[OPT(nThreads)];
    for (int i = 0; i < OPT(nThreads); i++) {
      pthread_create(&ptr[i], NULL, read_worker, workers[i]);
   }   
   for (int i = 0; i < OPT(nThreads); i++) {
      pthread_join(ptr[i], NULL);
   }    
   delete []ptr;
}

int light::solve() {
    printf("c -----------------solve start----------------------\n");
    pthread_t *ptr = new pthread_t[OPT(nThreads)];
    for (int i = 0; i < OPT(nThreads); i++) {
      pthread_create(&ptr[i], NULL, solve_worker, workers[i]);
    }

    thread_inf unimprove[OPT(nThreads)];
    auto clk_sol_st = std::chrono::high_resolution_clock::now();
    int pre_time = std::chrono::duration_cast<std::chrono::seconds>(clk_sol_st - clk_st).count();
    int sol_thd = 0, intv_time = OPT(reset_time);
    while (!terminated) {
        usleep(100000);        
        auto clk_now = std::chrono::high_resolution_clock::now();
        int solve_time = std::chrono::duration_cast<std::chrono::seconds>(clk_now - clk_st).count();
        if (solve_time >= OPT(cutoff)) {
            terminated = 1;
            terminate_workers();
        }
    }
    printf("c ending solve\n");
    // terminate_workers(); //important, need combine nps/dps !!!!!!!!!!!!!!!!

    for (int i = 0; i < OPT(nThreads); i++) {
        pthread_join(ptr[i], NULL);
    }
    
    printf("c ending join\n");
    if (result == 10)
        workers[winner_id]->model.copyTo(model);
    auto clk_now = std::chrono::high_resolution_clock::now();
    double solve_time = std::chrono::duration_cast<std::chrono::milliseconds>(clk_now - clk_sol_st).count();
    solve_time = 0.001 * solve_time;
    printf("c solve time: %.2lf\nc winner is %d, period is %d\n", solve_time, winner_id, winner_period);
    for (int i = 0; i < OPT(nThreads); i++) {
        printf("c thread %d waiting time: %.2lf\n", i, workers[i]->get_waiting_time());
    }   
    delete []ptr;
    return result;
}

int light::run() {
    init_workers();
    diversity_workers();
    if (OPT(preprocessor)) {
        pre = new preprocess();
        int res = pre->do_preprocess(filename);
        if (!res) return 20;
    }
    else worker_sign = filename;
    parse_input();
    if (OPT(clause_sharing)) share();
    int res = solve();
    if (res == 10 && OPT(preprocessor)) {
        for (int i = 1; i <= pre->orivars; i++)
            if (pre->mapto[i]) pre->mapval[i] = (model[abs(pre->mapto[i])-1] > 0 ? 1 : -1) * (pre->mapto[i] > 0 ? 1 : -1);
        pre->get_complete_model();
        model.clear();
        for (int i = 1; i <= pre->orivars; i++) {
            model.push(i * pre->mapval[i]);
        }    
    }
    return res;
}

void print_model(vec<int> &model) {
    printf("v");
    for (int i = 0; i < model.size(); i++) {
        printf(" %d", model[i]);
    }
    puts(" 0");
}

void solve(int argc, char **argv) {
    light* S = new light();
    S->arg_parse(argc, argv);
    int res = S->run();
    if (res == 10) {
        printf("s SATISFIABLE\n");
        // print_model(model);
    }
    else if (res == 20) {
        printf("s UNSATISFIABLE\n");
    }
    else {
        printf("s UNKNOWN\n");
    }
    delete(S);
    return;
}