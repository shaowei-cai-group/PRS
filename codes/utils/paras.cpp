#include "paras.hpp"
#include <cstdio>
#include <iostream>
#include <string>
 
void paras::init_paras() {
#define PARA(N, T, D, L, H, C) \
    if (!strcmp(#T, "int")) map_int[#N] = D; \
    else map_double[#N] = D; 
    PARAS 
#undef PARA

#define STR_PARA(N, D, C) \
    map_string[#N] = D; 
    STR_PARAS 
#undef STR_PARA
}

void paras::sync_paras() {
#define PARA(N, T, D, L, H, C) \
    if (!strcmp(#T, "int")) N = map_int[#N]; \
    else N = map_double[#N]; 
    PARAS 
#undef PARA
#define STR_PARA(N, D, C) \
    N = map_string[#N];
    STR_PARAS 
#undef STR_PARAs
}


void paras::set_para(char *name, int val) {
    map_int[name] = val;
}

void paras::set_para(char *name, double val) {
    map_double[name] = val;
}

void paras::set_para(char *name, char* val) {
    map_string[name] = val;
}


void paras::print_change() {
    printf("c ------------------- Paras list -------------------\n");
    printf("c %-20s\t %-10s\t %-10s\t %-10s\t %s\n",
           "Name", "Type", "Now", "Default", "Comment");

#define PARA(N, T, D, L, H, C) \
    if (map_int.count(#N)) printf("c %-20s\t %-10s\t %-10d\t %-10s\t %s\n", (#N), (#T), N, (#D), (C)); \
    else printf("c %-20s\t %-10s\t %-10f\t %-10s\t %s\n", (#N), (#T), N, (#D), (C)); 
    PARAS
#undef PARA

#define STR_PARA(N, D, C) \
    printf("c %-20s\t string\t\t %-10s\t %-10s\t %s\n", (#N), N.c_str(), (#D), (C));
    STR_PARAS
#undef STR_PARA



    printf("c --------------------------------------------------\n");
}