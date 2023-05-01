#include "basesolver.hpp"
#include <deque>

struct kissat;
struct cvec;

class basekissat : public basesolver {
public:
    void terminate();
    void add(int l);
    int  solve();
    int  val(int l);
    void configure(const char* name, int id);

    int  get_conflicts();
    void parse_from_CNF(char* filename);
    void parse_from_PAR(preprocess *pre);
    void exp_clause(void *cl, int lbd);
    bool imp_clause(clause_store *cls, void *cl);

    basekissat(int id, light *light);
    ~basekissat();
    kissat* solver;

    friend int cbkImportClause(void *, int *, cvec *);
    friend int cbkExportClause(void *, int *, cvec *);
    friend void cbk_free_clauses(void *);
};