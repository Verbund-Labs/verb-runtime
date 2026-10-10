#pragma once

#include <string>
#include <dlfcn.h>
#include <stdexcept>
#include "verbrt/value.hpp"

namespace verbrt {

class DynamicLibrary {
public:
    explicit DynamicLibrary(const std::string& path) {
        handle_ = dlopen(path.c_str(), RTLD_NOW | RTLD_GLOBAL);
        if (!handle_) {
            throw std::runtime_error("Failed to load library: " + path + " - " + dlerror());
        }
    }

    ~DynamicLibrary() {
        if (handle_) {
            dlclose(handle_);
        }
    }

    void* get_symbol(const std::string& name) {
        void* sym = dlsym(handle_, name.c_str());
        if (!sym) {
            throw std::runtime_error("Symbol not found: " + name + " - " + dlerror());
        }
        return sym;
    }

private:
    void* handle_ = nullptr;
};

} // namespace verbrt
