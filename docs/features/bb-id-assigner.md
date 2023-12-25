# How to use `bb-id-assigner` feature

This feature assigns IDs to basic blocks of the program. The assigned IDs are used by `bitmap-feedback` and `cfg-exporter` features.

## Usage

```
$ fuzzuf-cc --features bb-id-assigner [options] <file>
```

## Options

### --bb-id-assigner-path-splitting / --bb-id-assigner-no-path-splitting

Specifies whether to perform Critical Edge Splitting. Disabled by default.

### --bb-id-assigner-node-selection-strategy arg

Specifies the algorithm for selecting basic blocks for ID assignment. Default value is `ALL`.

| arg                | effect                                                          |
|--------------------|-----------------------------------------------------------------|
| `ALL`              | Assigns IDs to all basic blocks                                 |
| `NO_MULTIPLE_PRED` | Assigns IDs only to basic blocks with an in-degree of 1 or less |

### --bb-id-assigner-id-generation-strategy arg

Specifies the algorithm for ID generation. Default value is `RANDOM`.

| arg          | effect                                              |
|--------------|-----------------------------------------------------|
| `RANDOM`     | Assigns a random integer between 0 and the map size |
| `SEQUENTIAL` | Assigns an integer sequentially starting from 0     |

### --bb-id-assigner-map-size arg

Specifies the map size. Default value is 65536.
