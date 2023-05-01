#ifndef _preprocess_hpp_INCLUDED
#define _preprocess_hpp_INCLUDED

#include "../utils/hashmap.hpp"
#include "../utils/vec.hpp"
typedef long long ll;

#define mapv(a, b) (1ll * (a) * nlit + (ll)(b))

inline int pnsign(int x) {
    return (x > 0 ? 1 : -1);
}
inline int sign(int x) {
    return x & 1 ? -1 : 1;
}
inline int tolit(int x) {
    return x > 0 ? ((x - 1) << 1) : ((-x - 1) << 1 | 1);
}
inline int negative(int x) {
    return x ^ 1;
}
inline int toiidx(int x) {
    return (x >> 1) + 1;
}
inline int toeidx(int x) {
    return (x & 1 ? -toiidx(x) : toiidx(x));
}

struct xorgate {
    xorgate(int _c, int _rhs, int _sz) {
        c = _c, rhs = _rhs, sz = _sz;
    }
public:
    int c;
    int rhs;
    int sz;
};

struct preprocess {
public:   
    preprocess();
    int vars;
    int clauses;
    vec<vec<int>> clause, res_clause;
    // void readfile(const char *file);
    void write_cnf();
    void release();
    
    int maxlen, orivars, oriclauses, res_clauses, resolutions;
    int *f, nlit, *a, *val, *color, *varval, *q, *seen, *resseen, *clean, *mapto, *mapfrom, *mapval;
    // HashMap* C;
    vec<int> *occurp, *occurn, clause_delete, nxtc, resolution;
    
    int find(int x);    
    bool res_is_empty(int var);
    void update_var_clause_label();
    void preprocess_init();
    bool preprocess_resolution();
    bool preprocess_binary();
    bool preprocess_up();
    void get_complete_model();
    int  do_preprocess(char* filename);

    vec<vec<int>> card_one;
    vec<vec<double>> mat;
    vec<int> *occur;
    vec<int> cdel;
    int  check_card(int id);
    int  preprocess_card();
    int  search_almost_one();    
    int  card_elimination();
    int  scc_almost_one();
    void upd_occur(int v, int s);

    int *abstract;
    int gauss_eli_unit;
    int gauss_eli_binary;
    vec<xorgate> xors;
    vec<int> scc_id;
    vec<vec<int>> scc;
    vec<vec<int>> xor_scc;
    bool preprocess_gauss();
    int  search_xors();
    int  cal_dup_val(int i);
    int  ecc_var();
    int  ecc_xor();
    int  gauss_elimination();
};


#endif