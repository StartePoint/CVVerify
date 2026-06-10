# Third-Party Notices

This repository includes bundled third-party components that keep their own licenses and notices.

## OpenCV SDK

The repository tracks a bundled Windows MinGW OpenCV SDK at:

- `opencvsdk/windows/opencv4.12`

Primary upstream license file:

- `opencvsdk/windows/opencv4.12/LICENSE`

Additional bundled notices and dependency licenses:

- `opencvsdk/windows/opencv4.12/etc/licenses/`

## Maintainer Notes

When upgrading the bundled SDK:

- review the upstream OpenCV license and all bundled dependency notices
- confirm redistribution is still allowed for the packaged binaries and support files
- update this file if the bundled dependency set changes in a meaningful way
- keep any required vendor notices in the repository

## Project Source License

The CVVerify source code is licensed separately under the repository root [MIT License](LICENSE). Third-party components do not inherit that license.
