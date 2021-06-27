# Hash Table

Multiple Hash Table implementations:

1. Fine grain lock (Heller et al.) [4]
2. Software Transactional Memory (STM)
3. Lock Free (Harris, the linked list) [5]
4. Hardware Transactional Memory (Intel RTM)

## About the code

You need to download the [tinySTM](https://github.com/patrickmarlier/tinystm) 
library to use the STM version. The `build_stm.sh` script help to do that and 
compile the code.

The locks version and STM are implemented with pthreads. OpenMP is used to
test the data structure.

Important thing, remove all __.o__  before build any version.

Also the code trying that every node size is equal to a Intel Cache Line (64
bytes) to avoid false sharing.

### Requirements

* GCC
* OpenMP support
* TinySTM

### Build

```Bash
make 
```

## Sources

1. [tinySTM](https://github.com/patrickmarlier/tinystm)
2. [openMP](https://www.openmp.org/)
3. [Why you can not use rnd() in openMP parallel zones](https://stackoverflow.com/questions/4287531/how-to-generate-random-numbers-in-parallel)
4. [A Lazy Concurrent List-Based Set Algorithm, Heller et al. OPODIS'05](https://people.csail.mit.edu/shanir/publications/Lazy_Concurrent.pdf)
5. [A Pragmatic Implementation of Non-Blocking Linked-Lists, Harris, DISC'01](https://www.microsoft.com/en-us/research/wp-content/uploads/2001/10/2001-disc.pdf)
