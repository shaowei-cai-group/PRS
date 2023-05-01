#include "preprocess.hpp"
#include <algorithm>
#include <set>
#include "m4ri/m4ri.h"
#define MAX_XOR 6

bool cmpvar(int x, int y) {
    if (abs(x) == abs(y)) return x > y;
    return abs(x) < abs(y);
}

int preprocess::cal_dup_val(int i) {
    for (int j = 0; j < clause[i].size(); j++) a[j] = clause[i][j];
    std::sort(a, a + clause[i].size(), cmpvar);
    int v = 0;
    for (int j = 0; j < clause[i].size(); j++)
        if (a[j] < 0) v |= (1 << j);
    return v;
}

int preprocess::search_xors() {
    vec<int> xorsp;
    vec<bool> dup_table;
    for (int i = 1; i <= vars; i++) {
        seen[i] = 0;
        occurp[i].clear();
        occurn[i].clear();
    } 
    for (int i = 1; i <= clauses; i++) {
        abstract[i] = clause_delete[i] = nxtc[i] = 0;
        int l = clause[i].size();
        for (int j = 0; j < l; j++) {
            if (clause[i][j] > 0) occurp[clause[i][j]].push(i);
            else occurn[-clause[i][j]].push(i);
            abstract[i] |= 1 << (abs(clause[i][j]) & 31);
        }
    }
    for (int i = 1; i <= clauses; i++) {
        if (nxtc[i]) continue;
        nxtc[i] = 1;
        int l = clause[i].size();
        if (l <= 2 || l > MAX_XOR) continue;
        int required_num = 1 << (l - 2), skip = 0, mino = clauses + 1, mino_id = 0;
        for (int j = 0; j < l; j++) {
            int idx = abs(clause[i][j]);
            if (occurp[idx].size() < required_num || occurn[idx].size() < required_num) {
                skip = 1; break;
            }
            if (occurp[idx].size() + occurn[idx].size() < mino) {
                mino = occurp[idx].size() + occurn[idx].size();
                mino_id = idx;
            }
        }
        if (skip) continue;
        xorsp.clear();
        xorsp.push(i);
        for (int j = 0; j < occurp[mino_id].size(); j++) {
            int o = occurp[mino_id][j];
            if (!nxtc[o] && clause[o].size() == l && abstract[o] == abstract[i])
                xorsp.push(o);
        }
        for (int j = 0; j < occurn[mino_id].size(); j++) {
            int o = occurn[mino_id][j];
            if (!nxtc[o] && clause[o].size() == l && abstract[o] == abstract[i])
                xorsp.push(o);
        }
        if (xorsp.size() < 2 * required_num) continue;

        int rhs[2] = {0, 0};
        for (int j = 0; j < l; j++)
            seen[abs(clause[i][j])] = i;
        dup_table.clear();
        dup_table.growTo(1 << MAX_XOR, false);
        
        for (int j = 0; j < xorsp.size(); j++) {
            int o = xorsp[j], dup_v;
            bool xor_sign = true;
            for (int k = 0; k < clause[o].size(); k++) {
                if (seen[abs(clause[o][k])] != i) goto Next;
                if (clause[o][k] < 0) xor_sign = !xor_sign;
            }
            dup_v = cal_dup_val(o);
            if (dup_table[dup_v]) continue;
            dup_table[dup_v] = true;
            rhs[xor_sign]++;
            if (j) nxtc[o] = 1;
        Next:;
        }
        assert(rhs[0] <= 2 * required_num);
        assert(rhs[1] <= 2 * required_num);
        if (rhs[0] == 2 * required_num)
            xors.push(xorgate(i, 0, l));
        if (rhs[1] == 2 * required_num)
            xors.push(xorgate(i, 1, l));
        // if (rhs[0] == 2 * required_num && rhs[1] == 2 * required_num) {
        //     printf("%d %d\n", rhs[0], rhs[1]);
        //     for (int j = 0; j < xorsp.size(); j++) {
        //         int o = xorsp[j];
        //         for (int k = 0; k < clause[o].size(); k++) {
        //             printf("%d ", clause[o][k]);
        //         }
        //         printf(" ---------- %d\n", o);
        //     }
        // }
        // int a = abs(clause[i][0]) - 1;
        // int b = abs(clause[i][1]) - 1;
        // int c = abs(clause[i][2]) - 1;
        // if (a > b) std::swap(a, b);
        // if (a > c) std::swap(a, c);
        // if (b > c) std::swap(b, c);
        // printf("%d %d %d ", a, b, c);
        // puts("");
    }
    return xors.size();
}

bool cmpxorgate(xorgate a, xorgate b) {
    return a.sz > b.sz;
}

int preprocess::ecc_var() {
    scc_id.clear();
    scc_id.growTo(vars + 1, -1);
    scc.clear();
    //sort(xors.data, xors.data + xors.sz, cmpxorgate);
    std::set<int> xids;

    for (int i = 0; i < xors.size(); i++) {
        int x = xors[i].c;
        xids.clear();
        for (int j = 0; j < clause[x].size(); j++)
            if (scc_id[abs(clause[x][j])] != -1)
                xids.insert(scc_id[abs(clause[x][j])]);

        if (xids.size() == 0) {
            scc.push();
            for (int j = 0; j < clause[x].size(); j++) {
                scc_id[abs(clause[x][j])] = scc.size() - 1;
                scc[scc.size() - 1].push(abs(clause[x][j]));
            }
        }
        else if (xids.size() == 1) {
            int id = *xids.begin();
            for (int j = 0; j < clause[x].size(); j++) {
                if (scc_id[abs(clause[x][j])] == -1) {
                    scc_id[abs(clause[x][j])] = id;
                    scc[id].push(abs(clause[x][j]));
                }
            }
        }
        else {
            int id_max = -1; int sz_max = 0;
            for (std::set<int>::iterator it = xids.begin(); it != xids.end(); it++) {
                if (scc[*it].size() > sz_max)
                    sz_max = scc[*it].size(), id_max = *it;
            }
            for (std::set<int>::iterator it = xids.begin(); it != xids.end(); it++) {
                if (*it != id_max) {
                    vec<int>& v = scc[*it];
                    for (int k = 0; k < v.size(); k++) {
                        scc_id[v[k]] = id_max;
                        scc[id_max].push(v[k]);
                    }
                    v.clear();
                }
            }
            for (int j = 0; j < clause[x].size(); j++) {
                if (scc_id[abs(clause[x][j])] == -1) {
                    scc_id[abs(clause[x][j])] = id_max;
                    scc[id_max].push(abs(clause[x][j]));
                }
            }
        }
    }
    return scc.size();
}

int preprocess::ecc_xor() {
    for (int i = 0; i < scc.size(); i++) seen[i] = -1;
    for (int i = 0; i < xors.size(); i++) {
        int id = scc_id[abs(clause[xors[i].c][0])];
        if (seen[id] == -1) xor_scc.push(), seen[id] = xor_scc.size() - 1;
        int id2 = seen[id];
        xor_scc[id2].push(i);
    }
    return xor_scc.size();
}

int preprocess::gauss_elimination() {
    gauss_eli_unit = gauss_eli_binary = 0;
    vec<int> v2mzd(vars + 1, -1);
    vec<int> mzd2v;
    for (int i = 0; i < xor_scc.size(); i++) {
        if (xor_scc[i].size() == 1) continue;
        int id = scc_id[abs(clause[xors[xor_scc[i][0]].c][0])];
        assert(scc[id].size() > 3);
        if (scc[id].size() > 1e7 / xor_scc[i].size()) continue;
        mzd2v.clear();
        std::sort(scc[id].data, scc[id].data + scc[id].size(), cmpvar);
        for (int j = 0; j < scc[id].size(); j++) {
            assert(scc[id][j] > 0);
            assert(scc[id][j] <= vars);
            v2mzd[scc[id][j]] = j;
            mzd2v.push(scc[id][j]);
        }
        int cols = scc[id].size() + 1;
        mzd_t* mat = mzd_init(xor_scc[i].size(), cols);
        for (int row = 0; row < xor_scc[i].size(); row++) {
            int k = xors[xor_scc[i][row]].c;
            for (int j = 0; j < clause[k].size(); j++) 
                mzd_write_bit(mat, row, v2mzd[abs(clause[k][j])], 1);
            if (xors[xor_scc[i][row]].rhs) 
                mzd_write_bit(mat, row, cols - 1, 1); 
        }
        mzd_echelonize(mat, true);
        mzd_free(mat);
        for (int row = 0, rhs; row < xor_scc[i].size(); row++) {
            vec<int> ones;
            for (int col = 0; col < cols - 1; col++) 
                if (mzd_read_bit(mat, row, col)) {
                    if (ones.size() == 2) goto NextRow;
                    ones.push(mzd2v[col]);
                }
            
            rhs = mzd_read_bit(mat, row, cols - 1);
            if (ones.size() == 1) {
                ++gauss_eli_unit;
                clause.push();
                ++clauses;
                clause[clauses].push(ones[0] * (rhs ? 1 : -1));
            }
            else if (ones.size() == 2) {
                ++gauss_eli_binary;
                // assert(clauses == clause.size() - 1);
                int p = ones[0], q = rhs ? ones[1] : -ones[1];
                clause.push();
                ++clauses;
                clause[clauses].push(p);
                clause[clauses].push(q);
                clause.push();
                ++clauses;
                clause[clauses].push(-p);
                clause[clauses].push(-q);
            }
            else if (rhs) {
                // for (int row = 0; row < xor_scc[i].size(); row++) {
                //     int k = xors[xor_scc[i][row]].c;
                //     for (int j = 0; j < clause[k].size(); j++) 
                //         printf("%d ", clause[k][j]);
                //     printf("-------- %d %d %d\n", xor_scc[i][row], k, xors[xor_scc[i][row]].rhs);
                // }
                return false;
            }
        NextRow:;
        }
    }
    return true;
}

bool preprocess::preprocess_gauss() {
    int nxors = search_xors();
    printf("c [GE] XORs: %d (time: 0.00)\n", nxors);
    if (!nxors) return true;
    int nvarscc = ecc_var();
    printf("c [GE] VAR SCC: %d\n", nvarscc);
    int nxorscc = ecc_xor();
    printf("c [GE] XOR SCCs: %d (time: 0.00)\n", nxorscc);
    int res = gauss_elimination();
    printf("c [GE] unary xor: %d, bin xor: %d, bin added\n", gauss_eli_unit, gauss_eli_binary);
    if (!res) {printf("c [GE] UNSAT\n");}
    xors.clear(true);
    scc_id.clear(true);
    for (int i = 0; i < scc.size(); i++)
        scc[i].clear(true);
    scc.clear(true);
    for (int i = 0; i < xor_scc.size(); i++)
        xor_scc[i].clear(true);
    xor_scc.clear(true);
    if (!res) return false;
    clause_delete.growTo(clauses + 1, 0);
    nxtc.growTo(clauses + 1, 0);
    return true;
}