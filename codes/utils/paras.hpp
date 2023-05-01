#ifndef _paras_hpp_INCLUDED
#define _paras_hpp_INCLUDED

#include <cstring>
#include <unordered_map>

//        name,      type,   default, low, high, comments
#define PARAS \
    PARA( DCE        , int    , 0     , 0  , 1   , "DCE/NPS") \
    PARA( DPS_period , int    , 20000000 , 1  , 1e8 , "DCE sharing period") \
    PARA( margin     , int    , 10     , 0  , 1e3 , "DCE margin") \
    PARA( pakis      , int    , 0     , 0  , 1   , "Use pakis diversity") \
    PARA( reset      , int    , 0     , 0  , 1   , "Dynamically reseting") \
    PARA( reset_time , int    , 10    , 1  , 1e5 , "Reseting base interval (seconds)") \
    PARA( clause_sharing      , int    , 0     , 0  , 1   , "Sharing learnt clauses") \
    PARA( share_intv , int    , 500000, 0  , 1e9 , "Sharing interval (microseconds)") \
    PARA( share_lits , int    , 1500  , 0  , 1e6 , "Sharing lits (per every #share_intv seconds)") \
    PARA( shuffle    , int    , 1     , 0  , 1   , "Use random shuffle") \
    PARA( preprocessor   , int    , 1     , 0  , 1   , "Use Simplify (only preprocess)") \
    PARA( nThreads    , int    , 32    , 1  , 128 , "Thread number") \
    PARA( cutoff      , double , 5000  , 0  , 1e8 , "Cutoff time") 
    
#define STR_PARAS \
    STR_PARA( config_filename     , "" ,  "Config file") 

struct paras 
{
#define PARA(N, T, D, L, H, C) \
    T N = D;
    PARAS 
#undef PARA

#define STR_PARA(N, D, C) \
    std::string N = D;
    STR_PARAS
#undef STR_PARA

    std::unordered_map<std::string, int>    map_int;
    std::unordered_map<std::string, double> map_double;
    std::unordered_map<std::string, char*>  map_string;
    
    void init_paras ();
    void sync_paras ();
    void print_change ();
    void set_para   (char *arg, int val);
    void set_para   (char *arg, double val);
    void set_para   (char *arg, char* val);
};

#define OPT(N) (opt->N)

#endif