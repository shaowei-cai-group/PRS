#include <chrono>
#include "light.hpp"
#include <unistd.h>
#include <thread>
#include <iostream>

light::light():
  finalResult           (0),
  winner_period         (1e9),
  winner_id             (-1),
  maxtime               (5000)
{
    opt = new paras();
    opt->init_paras();
}

light::~light() {
    for (int i = 0; i < workers.size(); i++) delete(workers[i]);
    workers.clear(true);
}


void light::configure_from_file(const char* file) {
    if (!strcmp(file, "")) {
        configure_name = new vec<char*>[OPT(nThreads)];
        configure_val = new vec<double>[OPT(nThreads)];
        return;
    }
    printf("c Get configure file: %s\n", file);
    std::ifstream fin(file);
    char buf[1000];
    fin.getline(buf, 1000);
    char *p = buf + 6;
    int ws, ss, id = 0;
    p = read_int(p, &ws);
    p = read_int(p, &ss);
    // printf("%d %d\n", ws, ss);
    opt->set_para("nThreads", ws);
    configure_name = new vec<char*>[ws];
    configure_val = new vec<double>[ws];
    while (fin.getline(buf, 1000)) {
        p = strtok(buf, " ");
        // printf("%s\n", p);
        solver_type.push(0);
        while (p) {
            p = strtok(NULL, " ");
            // printf("%s\n", p);
            if (!p) break;
            int l = strlen(p), pos = 0;
            for (int i = 1; i < l; i++)
                if (p[i] == '=') pos = i;
            char *name = new char[pos];
            strncpy(name, p, pos);
            // printf("%s\n", name);
            configure_name[id].push(name);
            char *val = p + pos + 1;
            double v = atof(val);
            // printf("%.2lf\n", v);
            configure_val[id].push(v);
        }
        // printf("out\n");
        id++;
    }
}

void light::arg_parse(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        char *arg = argv[i];
        if (arg[0] != '-' || arg[1] != '-') {
            filename = arg; continue;
        }
        int l = strlen(arg), pos = 0;
        for (int i = 2; i < l; i++)
            if (arg[i] == '=') pos = i;
        if (!pos) continue;
        char name[50];
        strncpy(name, arg + 2, pos - 2);
        name[pos - 2] = '\0';
        char* val = arg + pos + 1;
        if (opt->map_int.count(name)) opt->set_para(name, atoi(val));
        else if (opt->map_double.count(name)) opt->set_para(name, atof(val));
        else opt->set_para(name, val);
    }
    opt->sync_paras();
    printf("%s\n", OPT(config_filename).c_str());
    configure_from_file(OPT(config_filename).c_str());
    opt->sync_paras();
    opt->print_change();
    printf("c filename: %s\n", filename);
}