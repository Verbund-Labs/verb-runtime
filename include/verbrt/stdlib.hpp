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
#include "verbrt/json.hpp"

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

// JSON operations
inline Value builtin_json_stringify(const std::vector<Value>& args) {
    if (args.empty()) return Value("null");
    return Value(Json::stringify(args[0]));
}

inline Value builtin_json_parse(const std::vector<Value>& args) {
    if (args.empty()) return Value();
    return Json::parse(args[0].to_string());
}

// String operations
inline Value builtin_str_upper(const std::vector<Value>& args) {
    if (args.empty()) return Value("");
    std::string s = args[0].to_string();
    for (char& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return Value(s);
}

inline Value builtin_str_lower(const std::vector<Value>& args) {
    if (args.empty()) return Value("");
    std::string s = args[0].to_string();
    for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return Value(s);
}

inline Value builtin_str_split(const std::vector<Value>& args) {
    if (args.empty()) return Value::make_list();
    std::string s = args[0].to_string();
    std::string delim = args.size() > 1 ? args[1].to_string() : " ";
    auto list = std::make_shared<ValueList>();
    size_t start = 0;
    size_t end = s.find(delim);
    while (end != std::string::npos) {
        list->push_back(Value(s.substr(start, end - start)));
        start = end + delim.length();
        end = s.find(delim, start);
    }
    list->push_back(Value(s.substr(start)));
    return Value(list);
}

inline Value builtin_str_join(const std::vector<Value>& args) {
    if (args.empty()) return Value("");
    if (!args[0].is_list() || !args[0].list_val) return Value("");
    std::string delim = args.size() > 1 ? args[1].to_string() : ", ";
    std::stringstream ss;
    const auto& list = *args[0].list_val;
    for (size_t i = 0; i < list.size(); ++i) {
        if (i > 0) ss << delim;
        ss << list[i].to_string();
    }
    return Value(ss.str());
}

inline Value builtin_str_trim(const std::vector<Value>& args) {
    if (args.empty()) return Value("");
    std::string s = args[0].to_string();
    size_t first = s.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return Value("");
    size_t last = s.find_last_not_of(" \t\n\r");
    return Value(s.substr(first, (last - first + 1)));
}

inline Value builtin_str_contains(const std::vector<Value>& args) {
    if (args.size() < 2) return Value(false);
    std::string s = args[0].to_string();
    std::string sub = args[1].to_string();
    return Value(s.find(sub) != std::string::npos);
}

// List operations
inline Value builtin_list_push(const std::vector<Value>& args) {
    if (args.size() >= 2 && args[0].is_list() && args[0].list_val) {
        args[0].list_val->push_back(args[1]);
    }
    return Value();
}

inline Value builtin_list_pop(const std::vector<Value>& args) {
    if (!args.empty() && args[0].is_list() && args[0].list_val && !args[0].list_val->empty()) {
        Value val = args[0].list_val->back();
        args[0].list_val->pop_back();
        return val;
    }
    return Value();
}

inline Value builtin_list_reverse(const std::vector<Value>& args) {
    if (!args.empty() && args[0].is_list() && args[0].list_val) {
        auto rev = std::make_shared<ValueList>(args[0].list_val->rbegin(), args[0].list_val->rend());
        return Value(rev);
    }
    return Value();
}

inline Value builtin_list_sort(const std::vector<Value>& args) {
    if (!args.empty() && args[0].is_list() && args[0].list_val) {
        auto sorted = std::make_shared<ValueList>(*args[0].list_val);
        std::sort(sorted->begin(), sorted->end(), [](const Value& a, const Value& b) {
            return (a < b).is_truthy();
        });
        return Value(sorted);
    }
    return Value();
}

// Reactive Signal Primitive: create_signal(initial_val)
inline Value builtin_create_signal(const std::vector<Value>& args) {
    auto val_ptr = std::make_shared<Value>(args.empty() ? Value() : args[0]);
    auto subs_ptr = std::make_shared<std::vector<NativeFunction>>();
    auto dict = std::make_shared<ValueDict>();

    (*dict)["get"] = Value([val_ptr](const std::vector<Value>&) -> Value {
        return *val_ptr;
    });

    (*dict)["set"] = Value([val_ptr, subs_ptr](const std::vector<Value>& a) -> Value {
        if (!a.empty()) {
            *val_ptr = a[0];
            for (const auto& sub : *subs_ptr) {
                if (sub) sub({*val_ptr});
            }
        }
        return *val_ptr;
    });

    (*dict)["subscribe"] = Value([subs_ptr](const std::vector<Value>& a) -> Value {
        if (!a.empty() && a[0].is_function()) {
            subs_ptr->push_back(a[0].fn_val);
        }
        return Value();
    });

    return Value(dict);
}

} // namespace verbrt
