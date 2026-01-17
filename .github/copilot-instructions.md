# Copilot Instructions (ICS-OS)

- Target OS: ICS-OS (32-bit, freestanding, no host libc). Avoid POSIX assumptions.
- Use only the SDK stdlib shipped in ics-os/sdk and kernel stdlib. If a new function is needed, add it explicitly to the SDK with clear headers and implementations.
- Third-party libraries must be explicitly vendored into ics-os/sdk and verified to be GPL-compatible with the project license. Document the source and license.
- Prefer incremental changes and keep existing style; avoid unrelated refactors.
- Use Makefile targets in ics-os/ (e.g., make, make usb, make boot-usb) and update README when workflows change.
- When adding files, keep paths under ics-os/ and include headers from existing SDK locations.
- Ensure new features work without dynamic linking unless already supported by the OS loaders.
