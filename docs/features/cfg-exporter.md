# How to use `cfg-exporter` feature

This feature constructs a Control Flow Graph (CFG) of the program and embeds it into the binary.

## Usage

```
$ fuzzuf-cc --features cfg-exporter [options] <file>
```

## Options

### --cfg-exporter-load-point arg

Specifies the timing for constructing the CFG. Default value is `OptimizerLast`.

| arg                            | effect                                               |
|--------------------------------|------------------------------------------------------|
| `OptimizerLast`                | Construct the CFG after optimization                 |
| `FullLinkTimeOptimizationLast` | Construct the CFG after Link Time Optimization (LTO) |
