# Contributing to CVVerify

Thanks for contributing.

## Before You Start

- Read the project specification in `docs/superpowers/specs/`
- Open or review an issue before starting larger work
- Keep changes focused and easy to review

## Development Principles

- Prefer small, composable modules
- Keep UI logic separated from processing logic
- Preserve cross-platform compatibility even when implementing Windows-first features
- Do not introduce direct platform-specific dependencies into core processing modules unless isolated behind adapters

## Branching

- Create one branch per feature or fix
- Use clear commit messages

Suggested prefixes:

- `feat:`
- `fix:`
- `docs:`
- `refactor:`
- `test:`
- `chore:`

## Pull Requests

Please include:

- what changed
- why it changed
- how it was tested
- any known limitations

For UI or result-rendering changes, include screenshots when possible.

## Code Style

- C++17
- CMake-based build
- Qt Widgets for desktop UI
- Keep files focused on a single responsibility
- Prefer descriptive names over abbreviated names

## Tests

When adding behavior:

- add or update tests where practical
- describe manual verification steps if automated tests are not yet available

## Documentation

Update documentation when:

- behavior changes
- configuration changes
- new user-facing capabilities are introduced
