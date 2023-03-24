PRS is a parallel SAT solver developed with C/C++.

Now, there is only one excutable file `PRS' in this repository.
Source codes will be released after SAT Competition 2023.


## How to build
```
make clean; make
```

## How to use
```
./PRS <instance> [config=config_filename] [--option=param]
```

For example,

```
./PRS ./test.cnf --clause_sharing=1 --DCE=1 --preprocessor=1 --nThreads=32 --cutoff=5000
```

## Parameters and Options
instance:           input CNF
config_filename:    the name configuration files
nThreads:           the number of workers in PRS
cutoff:             the wall time for SAT solving 
clause_sharing:     whether use clause sharing (1: enable;  0: disable)
DCE:                whether use delayed clauses exchange (DCE) for reproducibility (1: enable;  0: disable)
preprocessor:       whether use preprocessing (1: enable;  0: disable)
