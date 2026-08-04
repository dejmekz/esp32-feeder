#pragma once
// Minimal stand-in for the Arduino core so pure-logic sources (feeder.cpp)
// can be unit-tested on the host via the "native" PlatformIO environment,
// without pulling in the real hardware-specific Arduino core.
#include <cstdint>
#include <cstddef>
