# verb-runtime
**The official native runtime engine for VerbLang.**

Maintained by **Verbund Labs**.

[![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)](LICENSE)
[![C++ Standard](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B20)

`verb-runtime` (`libverbrt`) provides the foundational runtime primitives, memory allocators, virtual DOM tree, CSS cascade engine, and asynchronous event loop for VerbLang programs compiled to native binaries.

---

## Architectural Components

- **Type Model (`verbrt::Value`)**: High-performance variant representing numbers, strings, lists, dictionaries, closures, raw pointers, DOM nodes, and styles with zero unnecessary indirection.
- **Memory Subsystem**:
  - Direct 64-bit memory primitives (`alloc`, `free`, `peek64`, `poke64`).
  - High-throughput bump/arena allocator (`ArenaAllocator`).
  - Deterministic RAII `defer` cleanup (`VERB_DEFER`).
- **DOM & CSS Layout Engine**:
  - Virtual DOM tree with attribute and child management (`VerbNode`).
  - Cascading stylesheet engine resolving tag, `.class`, and `#id` selectors (`StyleSheet`).
  - Dual render targets: Standard HTML5 serialization and ANSI terminal UI (TUI).
- **Asynchronous Runtime**: Thread pool and event loop supporting microtasks and promises (`EventLoop`, `Promise`).
- **Standard Library Primitives**: Math, JSON serialization (`json_stringify`, `json_parse`), string operations, list utilities, process execution (`exec`), and reactive state signals (`create_signal`).

---

## Building & Verification

### Build with Make
```bash
make
make test
```

### Build with CMake
```bash
mkdir -p build && cd build
cmake ..
cmake --build .
ctest --verbose
```

---

## Repository Structure

```
verb-runtime/
├── include/verbrt/
│   ├── verbrt.hpp       # Umbrella header
│   ├── value.hpp        # Dynamic value variant & operator overloads
│   ├── memory.hpp       # Arena allocator, raw pointers & defer
│   ├── dom.hpp          # Virtual DOM tree & multi-target renderers
│   ├── css.hpp          # Cascading style sheets & selector engine
│   ├── event_loop.hpp   # Async event loop, thread pool & Promise
│   ├── stdlib.hpp       # Standard library functions
│   ├── json.hpp         # JSON parser and serializer
│   └── ffi.hpp          # C dynamic library loader
├── src/
│   ├── value.cpp        # Value operations implementation
│   └── verbrt.cpp       # Runtime initialization and metadata
└── tests/
    └── test_runtime.cpp # Comprehensive native test suite
```

---

## License

Apache License 2.0. Copyright (c) 2026 Verbund Labs.
