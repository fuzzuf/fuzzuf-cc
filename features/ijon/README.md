IJON on fuzzuf-cc
====


Referenced implementation: https://github.com/RUB-SysSec/ijon

How to build
----
```shell
cd features/ijon
${BUILD}/fuzzuf-ijon-cc -o ijon-test_put1 ../../put/ijon/test.c
```


How to run
----
```shell
${BUILD}/tool/dump_ijon_shm ./ijon-test_put1
```
