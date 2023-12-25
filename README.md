# fuzzuf-cc

fuzzuf-cc is a tool designed to instrument C and C++ programs for seamless integration with fuzzuf. Using features, you can easily customize program instrumentation to your exact needs.

## Building

For build instructions, please follow [building.md](./docs/building.md).

## Usage

fuzzuf-cc has two main executable files:

- `fuzzuf-cc` for compiling C program
- `fuzzuf-c++` for compiling C++ program

These executables can be used as drop-in replacements for clang and clang++. You can compile C program as follows:

```shell
fuzzuf-cc -o program program.c
```

Also, you can use the `--features` option to enable features. To compile with `bb-id-assigner` and `bitmap-feedback` features:

```shell
fuzzuf-cc --features bb-id-assigner,bitmap-feedback -o program program.c
```

List of features and their descriptions can be found below. And consult the [tutorial.md](https://github.com/fuzzuf/fuzzuf/blob/master/docs/tutorial.md) to learn about use with fuzzuf.

## List of Currently Available Features

| Feature           | Description                                                                                                                | CLI Usage                                       |
|-------------------|----------------------------------------------------------------------------------------------------------------------------|-------------------------------------------------|
| `bb-id-assigner`  | Assign IDs to the basic blocks of the program                                                                              | [CLI Usage](./docs/features/bb-id-assigner.md)  |
| `bitmap-feedback` | Instrument the program for bitmap feedback                                                                                 | [CLI Usage](./docs/features/bitmap-feedback.md) |
| `cfg-exporter`    | Construct a Control Flow Graph of the program                                                                              | [CLI Usage](./docs/features/cfg-exporter.md)    |
| `forkserver`      | Build the program as a forkserver                                                                                          | [CLI Usage](./docs/features/forkserver.md)      |
| `ijon-feedback`   | Build the annotated program with [IJON](https://github.com/fuzzuf/fuzzuf/blob/master/docs/algorithms/ijon/algorithm_en.md) | [CLI Usage](./docs/features/ijon-feedback.md)   |

## License

fuzzuf-cc is licensed under the GNU Affero General Public License v3.0. Some codes originate from external projects are licensed under their own licenses. Please refer to [LICENSE](./LICENSE) for details.

## Acknowledgements

This project has received funding from the Acquisition, Technology & Logistics Agency (ATLA) under the Innovative Science and Technology Initiative for Security (JPJ004596).
