fuzzuf forkserver
====


How to build 
----
```shell
cd features/forkserver
${BUILD}/fuzzuf-afl-cc -o test_put test_put.c
```


How to check forkserver works
----
Use benchmark client. `./benchmark_client` repeats PUT execution.

```shell
${BUILD}/features/forkserver/benchmark_client test_put
```


How to test
----
### Read stdin
This test checks if PUT prints "test" if file `/dev/shm/fuzzuf-cc.forkserver.executor_id-1.stdin` is given as stdin.
We can see that PUT prints `[*] [PUT] Read from stdin: test`.

```shell
$ echo test > /dev/shm/fuzzuf-cc.forkserver.executor_id-1.stdin
$ N=10 TEST=stdin ${BUILD}/features/forkserver/benchmark_client test_put stdin
[*] [Bench] Requested server to attach: executor_id=1
[*] [Bench] pargv[0]="test_put": pid=3264847
[*] [ForkServer] WaitClientAttach: waiting
[*] [Bench] Forkserver started: pid=3264847
[*] [Bench] Testing stdin
[*] [Bench] N = 10
[*] [PUT] Read from stdin: test
[*] Response { error=0, exit_code=115 }
[*] [Bench] Warm up done
[*] [PUT] Read from stdin: test
[*] Response { error=0, exit_code=115 }
Elapsed time at i=1: 158 [µs]
[*] [PUT] Read from stdin: test
[*] Response { error=0, exit_code=115 }
Elapsed time at i=2: 335 [µs]
[*] [PUT] Read from stdin: test
[*] Response { error=0, exit_code=115 }
... snipped ...
Elapsed time at i=8: 1292 [µs]
[*] [PUT] Read from stdin: test
[*] Response { error=0, exit_code=115 }
Elapsed time at i=9: 1433 [µs]
[*] [PUT] Read from stdin: test
[*] Response { error=0, exit_code=115 }
... snipped ...
```

### Write stdout/stderr
This test checks if PUT prints "Testing stdout" given from command line.

```shell
$ N=10 TEST=stdout ${BUILD}/features/forkserver/benchmark_client test_put "Testing stdout"
[*] [Bench] Requested server to attach: executor_id=1
[*] [Bench] pargv[0]="test_put": pid=3276364
[*] [ForkServer] WaitClientAttach: waiting
[*] [Bench] Forkserver started: pid=3276364
[*] [Bench] Testing stdout
[*] [Bench] N = 10
[*] Response { error=0, exit_code=115 }
[*] [Bench] Warm up done
[*] Response { error=0, exit_code=115 }
Elapsed time at i=1: 191 [µs]
[*] Response { error=0, exit_code=115 }
Elapsed time at i=2: 375 [µs]
[*] Response { error=0, exit_code=115 }
Elapsed time at i=3: 720 [µs]
[*] Response { error=0, exit_code=115 }
Elapsed time at i=4: 904 [µs]
[*] Response { error=0, exit_code=115 }
Elapsed time at i=5: 1110 [µs]
[*] Response { error=0, exit_code=115 }
Elapsed time at i=6: 1266 [µs]
[*] Response { error=0, exit_code=115 }
Elapsed time at i=7: 1438 [µs]
[*] Response { error=0, exit_code=115 }
Elapsed time at i=8: 1625 [µs]
[*] Response { error=0, exit_code=115 }
Elapsed time at i=9: 1817 [µs]
[*] Response { error=0, exit_code=115 }
----
Number of iteration = 10 [times]
Total iteration time = 2018 [µs]
Iteration time = 201 [µs/iter]
Total last 10% iteration time = 201 [µs]
Last 10% Iteration time = 201 [µs/iter]
[!] [ForkServer] Failed on WaitForAPICall: Success
        Requested to read 4 bytes, but read 0 bytes

$ ls -lh /dev/shm/fuzzuf-cc.forkserver.executor_id-1.*
-rw------- 1 aoki aoki 42  4月  4 15:19 /dev/shm/fuzzuf-cc.forkserver.executor_id-1.stderr
-rw------- 1 aoki aoki 42  4月  4 15:19 /dev/shm/fuzzuf-cc.forkserver.executor_id-1.stdout

$ cat /dev/shm/fuzzuf-cc.forkserver.executor_id-1.stdout 
[*] [PUT:stdout] Message "Testing stdout"

$ cat /dev/shm/fuzzuf-cc.forkserver.executor_id-1.stderr
[*] [PUT:stderr] Message "Testing stdout"
```

We can see that stdout and stderr files `/dev/shm/fuzzuf-cc.forkserver.executor_id-1.std{out,err}` contains stdout/err of PUT. 


Ref.) Benchmark result
----
```shell
$ taskset 0x03 ${BUILD}/features/forkserver/benchmark_client test_put
[*] N = 100000
[*] pargv[0]="test_put": pid=505494
[*] Warm up done
Elapsed time at i=10000: 838138 [µs]
Elapsed time at i=20000: 1674711 [µs]
Elapsed time at i=30000: 2510577 [µs]
Elapsed time at i=40000: 3346171 [µs]
Elapsed time at i=50000: 4182330 [µs]
Elapsed time at i=60000: 5016753 [µs]
Elapsed time at i=70000: 5851961 [µs]
Elapsed time at i=80000: 6687639 [µs]
Elapsed time at i=90000: 7523881 [µs]
----
Number of iteration = 100000 [times]
Total iteration time = 8363765 [µs]
Iteration time = 83 [µs/iter]
Total last 10% iteration time = 839884 [µs]
Last 10% Iteration time = 83 [µs/iter]
[!] [ForkServer] Failed to read command: Success
        Tips: Is this forkserver attached to client?
```
