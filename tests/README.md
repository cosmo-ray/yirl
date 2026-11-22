# YIRL Tests

Unit tests for YIRL, built on glib GTest.

## Build

```bash
make build-tests
```

## Run

```bash
./yirl-tests                 # run all tests
./yirl-tests -l              # list registered tests
./yirl-tests -j /entity/lifecycle/simple  # run one test
./yirl-tests -s              # skip slow tests
./yirl-tests -w              # skip widget/gui tests
./yirl-tests -W              # only widget/gui tests
```

## Windows/MSYS2

MSYS2 converts args starting with `/` to Windows paths, so `-j /test/path`
will not match. Prefix with `MSYS_NO_PATHCONV=1`:

```bash
MSYS_NO_PATHCONV=1 ./yirl-tests -j /script/quickjs/no-leak
```

## Layout

- `main.c`: test registration and CLI options (`TEST_TRY_ADD`, `-j`, `-l`, etc.)
- `tests.h`: test function declarations
- `*.c`: test implementations, grouped by engine area (entity, script, widget, etc.)
