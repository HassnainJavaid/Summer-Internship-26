## Layout

- `include/` - public C headers.
- `src/c/` - native tokenizer implementation.
- `src/python/` - Cython wrapper.
- `data/tokenizer/` - vocabulary and merge files.
- `data/samples/` - text inputs used by verification.
- `tests/c/` - native test programs.
- `scripts/` - verification scripts.
- `artifacts/` - generated token and decoded output files.
- `build/` - generated libraries, extensions, executables, and objects.

## Build

From the project root:

```text
make all
make python
```

The Python extension is placed in `build/python/` and the native library in
`build/lib/`. If dependencies are installed in the project virtual
environment, use that environment's `python` command instead.

## Tests

Run native tests from the project root so model and artifact paths resolve
consistently:

```text
./build/bin/test_loader
./build/bin/tokenizer_test
./build/bin/test_encoder tests/shakespeare.txt
```

Run the Python verification scripts from any directory:

```text
PYTHONPATH=build/python python verifyfiles.py
PYTHONPATH=build/python python scripts/verify.py "Hello world!"
```
