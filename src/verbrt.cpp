#include "verbrt/verbrt.hpp"

namespace verbrt {

// Version and metadata
extern "C" {

const char* verbrt_get_version() {
    return "1.0.0";
}

const char* verbrt_get_author() {
    return "Verbund Labs";
}

void verbrt_init() {
    initialize();
}

} // extern "C"

} // namespace verbrt
