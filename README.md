Simple, portable implementation of 2 methods for multi-precision integers:
multiplication and modulo operation.

Running timing tests
====================

Compile the test harness:
```
gcc -O3 -o harness -ggdb -Wall harness.c
```

Addition
--------

Create test directory to store results and temporary files
```
mkdir add_time
```

Create the test data for addition:
```
python3 ../test_data_gen.py -N 100000 --add -n 32
```
(`-n 32` specifies the number of words/limbs to use for numbers)

Run the test harness with the test data:
```
taskset --cpu-list 1 ../harness -a -i data.bin -o raw_times.csv -n $((8*32))
```
(8 in `-n $((8*32))` is the word size (64bits) and 32 is the number of
words/limbs, 32 is the default for `test_data_gen.py`)
With the assumption that core 1 is one of the isolated CPU cores.

After running the test you can delete the `data.bin` file.

Subtraction
-----------

Create test directory to store results and temporary files:
```
mkdir sub_time
```

Create the test data for subtraction:
```
python3 ../test_data_gen.py -N 100000 --sub -n 32
```

Run the test harness with the test data:
```
taskset --cpu-list 1 ../harness -s -i data.bin -o raw_times.csv -n $((8*32))
```

Again, you can delete the `data.bin` file after running the harness.

Multiplication
--------------

Create test directory to store results and temporary files:
```
mkdir mul_time
```

Create test data:
```
python3 ../test_data_gen.py -N 100000 --mul -n 32
```

Run the test harness:
```
taskset --cpu-list 1 ../harness -m -i data.bin -o raw_times.csv -n $((8*32))
```

Delete `data.bin` after.

Modulo
------

Create test directory:
```
mkdir mod_time
```

Create test data:
```
python3 ../test_data_gen.py -N 100000 --mod -n 64 -2 32
```
(since mod() is used for reducing results of different operations, not just
multiplication, the input to mod() can be arbitrary, but we mostly care about
feeding the output of mul(), so twice as large as output it is)

Run the harness:
```
taskset --cpu-list 1 ../harness -d -i data.bin -o raw_times.csv -n $((8*64)) -2 $((8*32))
```

Delete the `data.bin` after.

Analysis
--------
To analyse the timing data, download tlsfuzzer and install the
dependencies needed for timing analysis
(`pip3 install -r requirements-timing.txt`).

Convert the data into tuples:
```
PYTHONPATH=~/tlsfuzzer python3 ~/tlsfuzzer/tlsfuzzer/extract.py -l log.csv -o . --raw-times raw_times.csv
```

Run the analysis:
```
PYTHONPATH=~/tlsfuzzer python3 ~/tlsfuzzer/tlsfuzzer/analysis.py -o .
```

Multiplatform compiler/decompiler
=================================
test on https://godbolt.org/


Precise low overhead cycle counting
===================================

Getting cycle count on ARM:

```
{
uint64_t val;
asm volatile("mrs %0, cntvct_el0" : "=r" (val));
return val;
}
```

On PPC:
```
{
int64_t tbl, tbu0, tbu1;
asm("mftbu %0" : "=r"(tbu0));
asm("mftb  %0" : "=r"(tbl));
asm("mftbu %0" : "=r"(tbu1));
tbl &= -static_cast<int64>(tbu0 == tbu1);
return (tbu1 << 32) | tbl;
}
```

on s390x
(or stckf)
```
	stck	16($sp)
	lg	%r2,16($sp)
	br	$ra
```
Fallback:
```
struct timeval tv;
gettimeofday(&tv, nullptr);
return static_cast<int64_t>(tv.tv_sec) * 1000000 + tv.tv_usec;
```
