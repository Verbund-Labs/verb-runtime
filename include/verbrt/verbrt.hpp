#pragma once

// VerbLang Native Runtime System
// Developed by Verbund Labs

#include <vector>
#include <string>
#include "verbrt/value.hpp"
#include "verbrt/memory.hpp"
#include "verbrt/css.hpp"
#include "verbrt/dom.hpp"
#include "verbrt/event_loop.hpp"
#include "verbrt/json.hpp"
#include "verbrt/stdlib.hpp"
#include "verbrt/ffi.hpp"

namespace verbrt {

inline void initialize(int argc = 0, char** argv = nullptr) {
    g_cli_args.clear();
    for (int i = 0; i < argc; ++i) {
        g_cli_args.push_back(Value(std::string(argv[i])));
    }
}

} // namespace verbrt
