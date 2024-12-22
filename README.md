# **Overview**
**SimpleKvStore is a simple K-V database based on LSM-Tree.**

# **Features**
- Simple interface. The SimpleKvStore includes simple interfaces, such as `Insert`, `Delete`, `Get` and `ContainsKey`, provided by `class TCDB`. For more details, see [SimpleKvStore-doc]().
- Clear structure and good coding style. This project mostly follows [Google C++ style](https://google.github.io/styleguide/cppguide.html), and has detailed comments. All modules are well organized that you can replace any of them without making significant changes to the project.
- Easy to use. The users can run a database by specifying a `Config` object and then create a `TCDB` instance by passing the object as the construction function's parameter.

# **Limitations**
- The SimpleKvStore works only on Linux.
- Multithread operations are not supported yet for the current version.

# **Getting the source**
Get the debug version of SimpleKvStore by running command:
```
git clone -b debug https://github.com/NekonekoTom/SimpleKvStore.git
```

# **Building**
SimpleKvStore can be build through `makefile`:
```
cd SimpleKvStore
make
```
Then you can run `main.out` for test.

SimpleKvStore provides two build pipelines.
- Build using CMake script (recommended)

  Run the bash script `build_debug.sh`.
  The script will create a directory `build`.
  If `build` already exists, it will be removed by the script.
  All modules are compiled to static library `.a` files, and linked with the `main.cc`.

- Build from `makefile`
  - Rename the `old_makefile.txt` to makefile;
  - Rename all `.cc` source files to `.cpp` by running the Python script in `./src`:
    ```
    cd ./src
    python rename_postfix.py .cc .cpp -r
    ```
    Or you can modify the `old_makefile.txt` by replacing all `.cpp` with `.cc` without renaming the source files.
  - Move main.cpp or main.cc to the root directory;
  - Run command `make` under the root directory. All intermediate files will NOT be reserved by default.

# **Performance**
- Write performance:
  When compiling with `-O2` optimization, SimpleKvStore's write performance is shown in the chart below.
  Sequential write:
  | Data scale | Write time (second) | Estimated QPS |
  | ---------- | ------------------- | ------------- |
  | 100'000    | 0.250               | 400000        |
  | 500'000    | 1.865               | 268097        |
  | 1'000'000  | 4.834               | 206868        |

  Concurrently write by 4 threads:
  | Data scale | Write time (second) | Estimated QPS |
  | ---------- | ------------------- | ------------- |
  | 100'000    | 0.294               | 340000        |
  | 500'000    | 1.787               | 280000        |
  | 1'000'000  | 4.471               | 224000        |

- Read performance (Disabled cache):
  All requests are for existing key-values, which means the bloom filter will never be hit.
  Sequential read:
  | Data scale | Read time (second)  | Estimated QPS |
  | ---------- | ------------------- | ------------- |
  | 100'000    | 7.217               | 13856         |
  | 500'000    | 67.476              | 7410          |
  | 1'000'000  | 204.316             | 4894          |

  Concurrently read by 4 threads:
  | Data scale | Read time (second)  | Estimated QPS |
  | ---------- | ------------------- | ------------- |
  | 100'000    | 4.223               | 23223         |
  | 500'000    | 33.016              | 15144         |
  | 1'000'000  | 98.153              | 10188         |

- To reproduce the performance test, please run `./bin/perf_test`. The result may differ under different platforms, hardware configurations, data scales and entry sizes.

# **Others**
## TODO list
-[x] Build by CMake instead of makefile.

-[x] Bloom filter.

-[x] Multithread access.

-[ ] More functions for TCLogger.

-[x] LRU Cache.
