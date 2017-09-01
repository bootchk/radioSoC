Files authored by Segger.
Copied from NRF_SDK.

Not C++ compatible without -fpermissive flag to compiler
Or: hacked to cast to (char*) in two places.

This are configured by sdk_config.h and seggerConfig.h one directory up.

These are not platform independent.
The build configuration has include paths to Nordic SDK, for these files to use.

Logging is potent if LOGGING defined.
If logging is potent, it uses these files.

Built into the library.
Can be used by calling app.
If calling app does not use these files AND if logging is not potent, the linker will not use the functions defined in these files.