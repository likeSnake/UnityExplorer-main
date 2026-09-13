#pragma once

#include <string>

#include "il2cpp_dump_core.hpp"

namespace ildump {

// Captures the live BinaryDesignData configuration caches after the normal
// IL2CPP image/class walk has established a trustworthy memory reader.
// `mode` is "off", "auto" or "required". In auto mode an unavailable or
// not-yet-initialized cache is reported in the dump log without failing the
// ordinary dump; required mode makes the caller treat that as a failed run.
bool CaptureHeroCatalogRuntime(Dumper &dumper, const char *outputDirectory,
                               const std::string &mode, std::string *error);

}  // namespace ildump

