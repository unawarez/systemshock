New over-engineered stuff that may not mesh with the rest of the codebase.

## files.h/files.cpp

The idea is all disk access goes through here. The original code `fopen`s wherever it wants a file, so every call site has different paths, different fallbacks, different case (in)sensitivity, and none of it is configurable. The goal is having an API around `ShockFilesConfig` can allow for consistency and user control.

There is a little voice that says "each game subsystem should have its own file logic; a generic file access layer doesn't make sense", but it's likely one or two user settings could/should affect all the subsystems at once.
