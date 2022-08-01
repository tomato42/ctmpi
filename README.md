test on https://godbolt.org/

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
