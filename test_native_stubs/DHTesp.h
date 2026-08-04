#pragma once
// Minimal stand-in for the DHT sensor library's TempAndHumidity type,
// the only piece feeder.h needs for host-side (native) testing.
struct TempAndHumidity {
  float temperature;
  float humidity;
};
