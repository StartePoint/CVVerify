# Security Policy

## Supported Versions

CVVerify is currently pre-release. Security fixes are handled on the active development line until tagged releases and long-term support windows are defined.

At this stage, treat `main` as the only supported code line unless a future release document states otherwise.

## Reporting a Vulnerability

Do not open a public GitHub issue for a security-sensitive report.

Use GitHub private vulnerability reporting if it is enabled for the repository. If that channel is not available yet, contact the maintainers through a private channel once one is published in the repository profile or project site.

Please include:

- the affected commit, branch, or version
- a concise impact summary
- reproduction steps or a proof of concept
- whether the issue requires a bundled dependency, model package, or specific operator workflow
- any mitigation ideas you have already validated

## Response Expectations

Current targets are best-effort rather than contractual:

- acknowledgement within 5 business days
- initial triage within 10 business days
- coordinated disclosure timing after impact and fix scope are understood

## Scope Notes

Security reports may involve:

- application source code
- build and packaging scripts
- bundled OpenCV SDK integration
- model import and export workflows
- sample assets or configuration files in the repository

Please avoid sending proprietary datasets, credentials, or private model weights unless a secure transfer path has been agreed in advance.
