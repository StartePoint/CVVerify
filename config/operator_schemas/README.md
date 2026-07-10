# Operator Schemas

JSON parameter schemas for built-in pipeline operators. Files in this directory are used for:

- documentation and preset tooling
- runtime validation against operator `StepSchema` definitions
- packaging alongside the application binary (`config/operator_schemas/`)

Each file name (without `.json`) is the schema key passed to `OperatorSchemaLoader::loadSchema()`.

The `id` field must match the operator id registered in `OperatorRegistry` (for example `builtin.gaussian_blur`).

`index.json` lists the curated schema set validated in unit tests via `OperatorSchemaLoader::validateIndexedSchemas()`.

## Regenerating schemas

When operator parameters change in C++, regenerate JSON from source:

```bash
python scripts/generate_operator_schemas.py
```

At runtime, `OperatorSchemaExporter::exportAllRegisteredSchemas()` can also dump schemas from the live registry (useful for diffing after refactors).
