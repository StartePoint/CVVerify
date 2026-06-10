# Open Source Release Checklist

Use this checklist before publishing the repository for the first time or before making a major public update.

## Repository Metadata

- Confirm `README.md` reflects the current feature set and build status.
- Confirm `LICENSE`, `CONTRIBUTING.md`, `SECURITY.md`, and `CODE_OF_CONDUCT.md` are present and accurate.
- Confirm issue templates and pull request templates still match the preferred workflow.

## Local Artifact Hygiene

- Make sure machine-local files are ignored.
- Do not publish Qt Creator user files, local test result dumps, temporary exports, or private workspace data.
- Review the repository for accidental absolute paths, credentials, private model weights, or internal-only URLs.

## Bundled Dependency Review

- Verify the bundled OpenCV SDK version is the one you intend to distribute.
- Review `THIRD_PARTY_NOTICES.md` and the upstream license files under `opencvsdk/windows/opencv4.12`.
- Confirm no proprietary datasets, weights, or customer assets are stored in the repository.

## Build and Test Documentation

- Re-check `docs/build-windows-mingw.md` after every toolchain or dependency update.
- Confirm the documented CMake and qmake commands still match the build files.
- Update screenshots or UI review documents if the visible workflow changed significantly.

## GitHub Workflow Expectations

- Make sure the published GitHub workflow is honest about what it validates.
- Avoid shipping a workflow that appears to run a hosted build if the required Qt or MinGW provisioning has not been stabilized.
- If build CI is intentionally deferred, say so clearly in `README.md`.

## Release Notes

- Update `CHANGELOG.md` with feature, behavior, and documentation changes that matter to contributors.
- Tag any known limitations or placeholder workflows so public users do not mistake them for finished functionality.
