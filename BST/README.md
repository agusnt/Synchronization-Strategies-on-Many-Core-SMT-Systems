# Binary Search Tree

Multiple BST implementations:

1. Multiple locks (lock optimize)
2. Lock Free (Howley et al. version)
3. Software Transactional Memory (STM)
4. Hardware Transactional Memory (Intel RTM)

## About the code

You need to download the [tinySTM](https://github.com/patrickmarlier/tinystm) 
library to use the STM version. The `build_stm.sh` script help to do that and 
compile the code.

The locks version and STM are implemented with pthreads and OpenMP (no locks, 
just threading).

Important thing, remove all __.o__  before build any version.

### Requirements

* GCC
* OpenMP support
* TinySTM

### Build

```Bash
$ make [ilock/htm/lockfree]

or

$ ./stm.sh
```

## Sources

1. [tinySTM](https://github.com/patrickmarlier/tinystm)
2. [openMP](https://www.openmp.org/)
3. [Why you can not use rnd() in openMP parallel zones](https://stackoverflow.com/questions/4287531/how-to-generate-random-numbers-in-parallel)
4. [A Non-Blocking Internal Binary Search Tree, Howley et al. SPAA'12](https://dl.acm.org/doi/pdf/10.1145/2312005.2312036)
