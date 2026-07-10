# Maintenance Scripts

Small Python utilities for keeping bundled resources aligned with C++ operator definitions.

## generate_operator_schemas.py

Regenerates all JSON files under `config/operator_schemas/` from built-in operator `StepSchema` definitions in `src/core/operators/builtin/`.

```bash
python scripts/generate_operator_schemas.py
```

Run this after changing operator parameters in C++ so documentation, validation tests, and packaged `config/` stay in sync.

## generate_sample_assets.py

Creates lightweight PNG fixtures under `samples/images/` for offline validation without external datasets.

```bash
python scripts/generate_sample_assets.py
```

Outputs:

- `samples/images/checkerboard.png`
- `samples/images/gradient_ramp.png`

## Notes

- Scripts use Python 3 standard library only.
- They do not invoke CMake or compile the application.
- At runtime, `OperatorSchemaExporter` can also dump schemas from the live registry when built into the application.
