#include "preprocess.hpp"
#include "../utils/parse.hpp"

preprocess::preprocess():
  vars                  (0),
  clauses               (0),
  maxlen                (0)
{}

void preprocess::preprocess_init() {
    f = new int[vars + 10];
    val = new int[vars + 10];
    color = new int[vars + 10];
    varval = new int[vars + 10];
    q = new int[vars + 10];
    clean = new int[vars + 10];
    seen = new int[(vars << 1) + 10];
    abstract = new int[clauses + 10];
    clause_delete.growTo(clauses+1, 0);
    nxtc.growTo(clauses+1, 0);
    occurp = new vec<int>[vars + 1];
    occurn = new vec<int>[vars + 1]; 
    for (int i = 1; i <= clauses; i++) {
        int l = clause[i].size();
        if (l > maxlen) maxlen = l;
    }
    resseen = new int[(vars << 1) + 10];
    a = new int[maxlen + 1];

    mapval = new int[vars + 10];
    mapto = new int[vars + 10];
    for (int i = 1; i <= vars; i++) mapto[i] = i, mapval[i] = 0;
}

void preprocess::release() {
    delete []f;
    delete []val;
    delete []color;
    delete []varval;
    delete []q;
    delete []clean;
    delete []seen;
    clause_delete.clear(true);
    nxtc.clear(true);
    delete []resseen;   
    delete []a;
    delete []mapfrom; 
    delete []abstract;
    for (int i = 0; i <= vars; i++)
        occurp[i].clear(true), occurn[i].clear(true);
    delete []occurp;
    delete []occurn;
    // delete C;
}


int count = 0;
void preprocess::update_var_clause_label() {
    ++count;
    int remain_var = 0;
    for (int i = 1; i <= vars; i++) color[i] = 0;
    for (int i = 1; i <= clauses; i++) {
        if (clause_delete[i]) {continue;}
        int l = clause[i].size();
        for (int j = 0; j < l; j++) {
            if (color[abs(clause[i][j])] == 0) color[abs(clause[i][j])] = ++remain_var;       
        }
    }

    int id = 0;
    for (int i = 1; i <= clauses; i++) {
        if (clause_delete[i]) {clause[i].setsize(0); continue;}
        ++id;
        int l = clause[i].size();
        if (i == id) {
            for (int j = 0; j < l; j++)
                clause[id][j] = color[abs(clause[i][j])] * pnsign(clause[i][j]);    
            continue;
        }
        clause[id].setsize(0);
        for (int j = 0; j < l; j++)
            clause[id].push(color[abs(clause[i][j])] * pnsign(clause[i][j]));
    }
    printf("c After preprocess: vars: %d -> %d , clauses: %d -> %d ,\n", vars, remain_var, clauses, id);
    for (int i = id + 1; i <= clauses; i++) 
        clause[i].clear(true);
    for (int i = remain_var + 1; i <= vars; i++)
        occurp[i].clear(true), occurn[i].clear(true);
    clause.setsize(id + 1);
    vars = remain_var, clauses = id;
}

void preprocess::get_complete_model() {
    int r = 0;
    for (int i = 1; i <= orivars; i++) 
        if (!mapto[i]) {
            if (!mapval[i]);
            else if (abs(mapval[i]) != 1) mapval[i] = 0, ++r;
        }
    if (r) { 
        occurp = new vec<int>[orivars + 1];
        occurn = new vec<int>[orivars + 1];   
        for (int i = 1; i <= orivars; i++) {
            occurp[i].clear(), occurn[i].clear();
        }
        vec<int> clause_state;
        clause_state.growTo(res_clauses + 1, 0);
        for (int i = 1; i <= res_clauses; i++) {
            int satisify = 0;
            for (int j = 0; j < res_clause[i].size(); j++) {
                int v = res_clause[i][j];
                if (v > 0) occurp[v].push(i);
                else occurn[-v].push(i);
                if (pnsign(v) * mapval[abs(v)] == 1) satisify = 1;
                if (!mapval[abs(v)]) ++clause_state[i];
            }
            if (satisify) clause_state[i] = -1;
        }
        for (int ii = resolutions; ii >= 1; ii--) {
            int v = resolution[ii];
            //attempt 1
            int assign = 1;
            for (int i = 0; i < occurn[v].size(); i++) {
                int o = occurn[v][i];
                if (clause_state[o] != -1 && clause_state[o] <= 1) {assign = 0; break;}
            }
            if (assign == 1) {
                mapval[v] = 1;
                for (int i = 0; i < occurn[v].size(); i++) {
                    int o = occurn[v][i];
                    if (clause_state[o] != -1) clause_state[o]--;
                } 
                for (int i = 0; i < occurp[v].size(); i++) 
                    clause_state[occurp[v][i]] = -1;
                continue;
            }
            //attempt -1
            assign = -1;
            for (int i = 0; i < occurp[v].size(); i++) {
                int o = occurp[v][i];
                if (clause_state[o] != -1 && clause_state[o] <= 1) {assign = 0; break;}
            }
            if (assign == -1) {
                mapval[v] = -1;
                for (int i = 0; i < occurp[v].size(); i++) {
                    int o = occurp[v][i];
                    if (clause_state[o] != -1) clause_state[o]--;
                } 
                for (int i = 0; i < occurn[v].size(); i++) 
                    clause_state[occurn[v][i]] = -1;
                continue;
            }
        }
        clause_state.clear(true);
        for (int i = 1; i <= orivars; i++) {
            occurp[i].clear(true), occurn[i].clear(true);
        }
        delete []occurp;
        delete []occurn;
        res_clause.clear(true);
        resolution.clear(true);
    }  
}

int preprocess::do_preprocess(char* filename) {
    readfile(filename, &vars, &clauses, clause);
    orivars = vars;
    oriclauses = clauses; 
    preprocess_init();
    int res = 1;
    
    res = preprocess_gauss();
    if (!res) {
        printf("c solved by gauss elimination\n");
        release();
        return 0;
    }

    res = preprocess_up();
    if (!res) {
        release();
        delete []mapto;
        delete []mapval;
        clause.clear(true);
        return 0;
    }


    if (vars <= 1e5 && clauses <= 1e6) {
        res = preprocess_card();
        if (!res) {
            printf("c solved by card elimination\n");
            release();
            delete []mapto;
            delete []mapval;
            clause.clear(true);
            res_clause.clear(true);
            resolution.clear(true);
            return 0;        
        }
    }
    res = preprocess_resolution();
    if (!res) {
        release();
        delete []mapto;
        delete []mapval;
        clause.clear(true);
        res_clause.clear(true);
        resolution.clear(true);
        return 0;
    }


    res = preprocess_binary();
    if (!res) {
        release();
        delete []mapto;
        delete []mapval;
        clause.clear(true);
        res_clause.clear(true);
        resolution.clear(true);
        return 0;
    }
    
    release();
    return 1;
}
