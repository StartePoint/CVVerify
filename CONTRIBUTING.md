# Contributing to CVVerify

Thanks for contributing to CVVerify. This repository is still pre-release, so small, well-scoped changes are much easier to review and merge than broad refactors.

## Before Opening a Pull Request

- Read the build guide in `docs/build-windows-mingw.md`.
- Review the current design and implementation plans in `docs/superpowers/specs/` and `docs/superpowers/plans/` when working on larger changes.
- Open or reference an issue before starting work that affects architecture, packaging, operator behavior, export format, or public APIs.
- Keep one pull request focused on one problem or one feature area.

## Local Development Baseline

The maintained local toolchain is:

- Qt 5.14.2
- MinGW 7.3.0 64-bit
- bundled OpenCV SDK in `opencvsdk/windows/opencv4.12`
- CMake and qmake project files

Use the commands in `docs/build-windows-mingw.md` instead of inventing a new local layout in pull requests.

## Development Guidelines

- Keep core image-processing logic in `src/core` and framework glue in `src/infra`.
- Keep UI behavior in `src/ui` and avoid mixing QWidget code into core processing modules.
- Reuse the schema-driven operator and parameter infrastructure before introducing custom one-off editors.
- Preserve Windows-first compatibility while avoiding unnecessary hard-coded machine paths.
- Avoid unrelated formatting churn in files you touch.

## Branches and Commits

Use a dedicated branch per feature or fix. Clear commit messages make release notes and review easier.

Suggested commit prefixes:

- `feat:`
- `fix:`
- `docs:`
- `refactor:`
- `test:`
- `chore:`

## Pull Request Expectations

Every pull request should explain:

- what changed
- why the change was needed
- how it was verified
- any known gaps or follow-up work

For UI changes, include screenshots or short notes describing the visible result. For operator behavior changes, mention any affected parameter names, defaults, or preview behavior.

## Tests

When behavior changes:

- add or update unit tests when practical
- run the relevant local test path when the environment is available
- describe manual verification steps if hosted or local automated coverage is not yet possible

If you cannot run tests, say so explicitly in the pull request.

## Documentation

Update documentation when:

- build steps change
- operator behavior changes
- menu text or workflow changes
- export format or file layout changes
- public repository policies change

At minimum, review `README.md`, `docs/build-windows-mingw.md`, and any feature-specific documents touched by the change.

## Bundled Resource Maintenance

When operator parameters change in C++:

```bash
python scripts/generate_operator_schemas.py
```

When sample images need regeneration:

```bash
python scripts/generate_sample_assets.py
```

See [scripts/README.md](../scripts/README.md) for details.
