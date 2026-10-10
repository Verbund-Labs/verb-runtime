# verb-runtime ⚡

The official native runtime engine for **VerbLang**, developed by **Verbund Labs**.

Designed from the ground up for extreme performance, combining the raw speed and direct hardware control of **C++20** with high-level conveniences from Python, JavaScript, HTML, and CSS.

---

## ✨ Features

- **Blazing Native Performance**: Written in modern C++20 with zero-overhead abstractions.
- **Dynamic & Static Type Value System**: High-speed unified variant (`verbrt::Value`) supporting integers, floats, strings, dynamic lists, hash dicts, closures, pointers, DOM nodes, and styles.
- **Memory Management & Direct Pointer Control**:
  - Raw memory allocation (`alloc`, `free`, `peek64`, `poke64`, `memcpy`).
  - High-throughput bump / Arena allocator for zero-fragmentation allocations.
  - Deterministic RAII `defer` cleanup semantics (`VERB_DEFER`).
- **First-Class DOM & CSS Layout Engine**:
  - Lightweight virtual DOM tree (`VerbNode`).
  - Native CSS style resolver supporting tag, class (`.class`), and ID (`#id`) selectors.
  - Multi-target renderers: Rich ANSI styled terminal UI (TUI) and standard HTML5 serialization.
- **Asynchronous Event Loop**: Microtasks, macrotasks, native thread pool, and Promise/Future primitives.
- **Seamless C/C++ FFI**: Direct dynamic library loading and ABI interop.

---

## 🛠️ Building & Testing

### Prerequisites
- Modern C++ compiler (`clang++` >= 15 or `g++` >= 11) supporting C++20.
- `make` or `cmake` >= 3.16.

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

## 🏛️ Architecture Overview

```
verb-runtime/
├── include/verbrt/
│   ├── verbrt.hpp       # Master umbrella header
│   ├── value.hpp        # Dynamic value model & operators
│   ├── memory.hpp       # Arena allocator, raw pointers & defer
│   ├── dom.hpp          # Virtual DOM tree & ANSI/HTML renderers
│   ├── css.hpp          # Cascading style sheets & selector engine
│   ├── event_loop.hpp   # Async event loop, thread pool & Promise
│   ├── stdlib.hpp       # Builtin functions (I/O, Math, Sys, Memory)
│   └── ffi.hpp          # C dynamic library loader
├── src/
│   ├── value.cpp        # Value operations implementation
│   └── verbrt.cpp       # Runtime entry point & metadata
└── tests/
    └── test_runtime.cpp # Comprehensive runtime test suite
```

---

## 📄 License

Apache License 2.0. Copyright © 2026 Verbund Labs.
