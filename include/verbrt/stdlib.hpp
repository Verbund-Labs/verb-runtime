#pragma once

#include <vector>
#include <string>
#include <cmath>
#include <chrono>
#include <thread>
#include <fstream>
#include "verbrt/value.hpp"
#include "verbrt/dom.hpp"
#include "verbrt/css.hpp"
#include "verbrt/memory.hpp"
#include "verbrt/event_loop.hpp"

namespace verbrt {

// Builtin Print function
inline Value builtin_print(const std::vector<Value>& args) {
    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) std::cout << " ";
        std::cout << args[i].to_string();
    }
    std::cout << "\n";
    return Value();
}

inline Value builtin_len(const std::vector<Value>& args) {
    if (args.empty()) return Value(0);
    return Value(static_cast<int64_t>(args[0].length()));
}

inline Value builtin_range(const std::vector<Value>& args) {
    int64_t start = 0;
    int64_t stop = 0;
    int64_t step = 1;

    if (args.size() == 1) {
        stop = args[0].as_int();
    } else if (args.size() == 2) {
        start = args[0].as_int();
        stop = args[1].as_int();
    } else if (args.size() >= 3) {
        start = args[0].as_int();
        stop = args[1].as_int();
        step = args[2].as_int();
    }

    auto list = std::make_shared<ValueList>();
    if (step > 0) {
        for (int64_t i = start; i < stop; i += step) {
            list->push_back(Value(i));
        }
    } else if (step < 0) {
        for (int64_t i = start; i > stop; i += step) {
            list->push_back(Value(i));
        }
    }
    return Value(list);
}

inline Value builtin_type(const std::vector<Value>& args) {
    if (args.empty()) return Value("nil");
    switch (args[0].type) {
        case ValueType::Nil: return Value("nil");
        case ValueType::Bool: return Value("bool");
        case ValueType::Int: return Value("int");
        case ValueType::Float: return Value("float");
        case ValueType::String: return Value("string");
        case ValueType::List: return Value("list");
        case ValueType::Dict: return Value("dict");
        case ValueType::Function: return Value("function");
        case ValueType::Pointer: return Value("pointer");
        case ValueType::Node: return Value("node");
        case ValueType::StyleRule: return Value("style");
    }
    return Value("unknown");
}

inline Value builtin_now_ns(const std::vector<Value>& /*args*/) {
    auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
    return Value(static_cast<int64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(now).count()));
}

inline Value builtin_now_ms(const std::vector<Value>& /*args*/) {
    auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
    return Value(static_cast<int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(now).count()));
}

inline Value builtin_sleep(const std::vector<Value>& args) {
    if (!args.empty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(args[0].as_int()));
    }
    return Value();
}

inline Value builtin_read_file(const std::vector<Value>& args) {
    if (args.empty()) return Value("");
    std::ifstream file(args[0].to_string());
    if (!file.is_open()) return Value("");
    std::stringstream buffer;
    buffer << file.rdbuf();
    return Value(buffer.str());
}

inline Value builtin_write_file(const std::vector<Value>& args) {
    if (args.size() < 2) return Value(false);
    std::ofstream file(args[0].to_string());
    if (!file.is_open()) return Value(false);
    file << args[1].to_string();
    return Value(true);
}

// Low-level C++ Memory operations
inline Value builtin_alloc(const std::vector<Value>& args) {
    int64_t bytes = args.empty() ? 0 : args[0].as_int();
    return Value(raw_alloc(bytes));
}

inline Value builtin_free(const std::vector<Value>& args) {
    if (!args.empty() && args[0].is_pointer()) {
        raw_free(args[0].ptr_val);
    }
    return Value();
}

inline Value builtin_peek64(const std::vector<Value>& args) {
    if (args.empty() || !args[0].is_pointer()) return Value(0);
    return Value(raw_peek64(args[0].ptr_val));
}

inline Value builtin_poke64(const std::vector<Value>& args) {
    if (args.size() >= 2 && args[0].is_pointer()) {
        raw_poke64(args[0].ptr_val, args[1].as_int());
    }
    return Value();
}

// Math builtins
inline Value builtin_sqrt(const std::vector<Value>& args) {
    if (args.empty()) return Value(0.0);
    return Value(std::sqrt(args[0].as_float()));
}

inline Value builtin_pow(const std::vector<Value>& args) {
    if (args.size() < 2) return Value(0.0);
    return Value(std::pow(args[0].as_float(), args[1].as_float()));
}

inline Value builtin_sin(const std::vector<Value>& args) {
    if (args.empty()) return Value(0.0);
    return Value(std::sin(args[0].as_float()));
}

inline Value builtin_cos(const std::vector<Value>& args) {
    if (args.empty()) return Value(0.0);
    return Value(std::cos(args[0].as_float()));
}

// DOM / CSS creation helpers
inline Value builtin_create_node(const std::string& tag, const ValueDict& attrs, const ValueList& children) {
    auto node = std::make_shared<VerbNode>(tag);
    for (const auto& [k, v] : attrs) {
        node->set_attr(k, v);
    }
    for (const auto& child : children) {
        node->add_child(child);
    }
    return Value(node);
}

inline Value builtin_create_style(const std::string& selector, const std::unordered_map<std::string, std::string>& props) {
    auto rule = std::make_shared<VerbStyleRule>(selector);
    rule->properties = props;
    StyleSheet::global().add_rule(rule);
    return Value(rule);
}

inline Value builtin_render_html(const std::vector<Value>& args) {
    if (!args.empty() && args[0].is_node() && args[0].node_val) {
        return Value(args[0].node_val->to_html());
    }
    return Value("");
}

inline Value builtin_render_tui(const std::vector<Value>& args) {
    if (!args.empty() && args[0].is_node() && args[0].node_val) {
        args[0].node_val->compute_styles();
        return Value(args[0].node_val->render_tui());
    }
    return Value("");
}

} // namespace verbrt
