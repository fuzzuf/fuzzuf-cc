exifutil
====


Original: https://github.com/fuzzuf/fuzzuf/tree/master/docs/resources/exifutil


How to build
----
```shell
cd put/exifutil
${BUILD}/fuzzuf-afl-cc -Wall -fno-stack-protector -Wno-address-of-packed-member -O3 -o fd-exifutil exifutil.c
```


How to run
----
```shell
$ N=1 ${BUILD}/features/forkserver/benchmark_client ./fd-exifutil -f fuzz_input/jpeg.jpg 
[*] N = 1
[*] Warm up done
[*] pargv[0]="./fd-exifutil": pid=499942
File: fuzz_input/jpeg.jpg
Unknown tag (262)       : 2 
Orientation             : 1 
X Resolution            : 72/1 
Y Resolution            : 72/1 
Unknown tag (40961)     : 1 
Unknown tag (40962)     : 16 
Unknown tag (40963)     : 16 
----
Number of iteration = 1 [times]
Total iteration time = 713 [µs]
Iteration time = 713 [µs/iter]
Total last 10% iteration time = 713 [µs]
Last 10% Iteration time = [!] [ForkServer] Failed to read command: Success
        Tips: Is this forkserver attached to client?
浮動小数点例外
```

NOTE: Error message `[!] [ForkServer] Failed to read command: Success` and `浮動小数点例外` (floating point exception) are not error and as expected.
