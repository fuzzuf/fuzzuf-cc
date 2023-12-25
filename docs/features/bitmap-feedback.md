# How to use `bitmap-feedback` feature

This feature instruments the program to enable bitmap feedback.

## Usage

```
$ fuzzuf-cc --features bitmap-feedback [options] <file>
```

## Options

### --bitmap-load-point arg

Specifies the timing for instrumentation. Default value is `OptimizerLast`.

| arg                            | effect                                                      |
|--------------------------------|-------------------------------------------------------------|
| `OptimizerLast`                | Performs instrumentation after optimization                 |
| `FullLinkTimeOptimizationLast` | Performs instrumentation after Link Time Optimization (LTO) |

### --bitmap-instrumentation-method arg

Specifies the algorithm for calculating the index when changing the bitmap. Default value is `HASHED_EDGE`.

| arg           | effect                            |
|---------------|-----------------------------------|
| `HASHED_EDGE` | `loc = (prev_loc >> 1) ^ cur_loc` |
| `NODE`        | `loc = cur_loc`                   |

### --bitmap-update-method arg

Specifies the algorithm for changing the bitmap. Default value is `NAIVE`.

| arg          | effect                                                   |
|--------------|----------------------------------------------------------|
| `NAIVE`      | `bitmap[loc] = bitmap[loc] + 1`                          |
| `AVOID_ZERO` | `bitmap[loc] = bitmap[loc] + 1 + (bitmap[loc] + 1 == 0)` |
| `CAP_FF`     | `bitmap[loc] = bitmap[loc] + (bitmap[loc] != 0xff)`      |
