#ifndef _parse_hpp_INCLUDED
#define _parse_hpp_INCLUDED
#include "vec.hpp"

char *read_int(char *p, int *i);
char *read_until_new_line(char *p);
void readfile(const char *file, int *orivars, int *oriclauses, vec<vec<int>> &clause);

#endif