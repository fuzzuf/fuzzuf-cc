cat
====


How to build
----
```shell
cd put/cat
${BUILD}/fuzzuf-afl-cc -o cat cat.c
```


How to run
----
```shell
$ echo Hello > /dev/shm/fuzzuf-cc.forkserver.executor_id-1.stdin

$ N=1 TEST=stdin ${BUILD}/features/forkserver/benchmark_client ./cat
[*] [Bench] Requested server to attach: executor_id=1
[*] [Bench] pargv[0]="./cat": pid=187
[*] [Bench] Forkserver started: pid=187
[*] [Bench] Testing stdin
[*] [Bench] N = 1
stdout: Hello
stderr: Hello
[*] Response { error=0, exit_code=0 }
----
Number of iteration = 1 [times]
Total iteration time = 243 [µs]
Iteration time = 243 [µs/iter]
Total last 10% iteration time = 243 [µs]
Last 10% Iteration time = [!] [ForkServer] Failed on WaitForAPICall: Success
        Requested to read 4 bytes, but read 0 bytes
Floating point exception (core dumped)
```
