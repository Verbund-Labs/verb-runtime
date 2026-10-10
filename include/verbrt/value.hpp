#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <variant>
#include <functional>
#include <cstdint>
#include <sstream>
#include <iostream>
#include <stdexcept>

namespace verbrt {

class VerbNode;
class VerbStyleRule;

enum class ValueType {
    Nil,
    Bool,
    Int,
    Float,
    String,
    List,
    Dict,
    Function,
    Pointer,
    Node,
    StyleRule
};

class Value;

using NativeFunction = std::function<Value(const std::vector<Value>&)>;
using ValueList = std::vector<Value>;
using ValueDict = std::unordered_map<std::string, Value>;

class Value {
public:
    ValueType type = ValueType::Nil;

    // Representation storage
    bool b_val = false;
    int64_t i_val = 0;
    double f_val = 0.0;
    std::string s_val;
    std::shared_ptr<ValueList> list_val;
    std::shared_ptr<ValueDict> dict_val;
    NativeFunction fn_val;
    void* ptr_val = nullptr;
    std::shared_ptr<VerbNode> node_val;
    std::shared_ptr<VerbStyleRule> style_val;

    // Constructors
    Value() : type(ValueType::Nil) {}
    Value(std::nullptr_t) : type(ValueType::Nil) {}
    Value(bool b) : type(ValueType::Bool), b_val(b) {}
    Value(int v) : type(ValueType::Int), i_val(v) {}
    Value(int64_t v) : type(ValueType::Int), i_val(v) {}
    Value(double v) : type(ValueType::Float), f_val(v) {}
    Value(const char* s) : type(ValueType::String), s_val(s ? s : "") {}
    Value(const std::string& s) : type(ValueType::String), s_val(s) {}
    Value(std::shared_ptr<ValueList> l) : type(ValueType::List), list_val(std::move(l)) {}
    Value(const ValueList& l) : type(ValueType::List), list_val(std::make_shared<ValueList>(l)) {}
    Value(std::shared_ptr<ValueDict> d) : type(ValueType::Dict), dict_val(std::move(d)) {}
    Value(const ValueDict& d) : type(ValueType::Dict), dict_val(std::make_shared<ValueDict>(d)) {}
    Value(NativeFunction fn) : type(ValueType::Function), fn_val(std::move(fn)) {}
    Value(void* ptr) : type(ValueType::Pointer), ptr_val(ptr) {}
    Value(std::shared_ptr<VerbNode> node) : type(ValueType::Node), node_val(std::move(node)) {}
    Value(std::shared_ptr<VerbStyleRule> style) : type(ValueType::StyleRule), style_val(std::move(style)) {}

    static Value make_list() {
        return Value(std::make_shared<ValueList>());
    }

    static Value make_dict() {
        return Value(std::make_shared<ValueDict>());
    }

    // Type queries
    bool is_nil() const { return type == ValueType::Nil; }
    bool is_bool() const { return type == ValueType::Bool; }
    bool is_int() const { return type == ValueType::Int; }
    bool is_float() const { return type == ValueType::Float; }
    bool is_number() const { return type == ValueType::Int || type == ValueType::Float; }
    bool is_string() const { return type == ValueType::String; }
    bool is_list() const { return type == ValueType::List; }
    bool is_dict() const { return type == ValueType::Dict; }
    bool is_function() const { return type == ValueType::Function; }
    bool is_pointer() const { return type == ValueType::Pointer; }
    bool is_node() const { return type == ValueType::Node; }
    bool is_style() const { return type == ValueType::StyleRule; }

    bool is_truthy() const {
        switch (type) {
            case ValueType::Nil: return false;
            case ValueType::Bool: return b_val;
            case ValueType::Int: return i_val != 0;
            case ValueType::Float: return f_val != 0.0;
            case ValueType::String: return !s_val.empty();
            case ValueType::List: return list_val && !list_val->empty();
            case ValueType::Dict: return dict_val && !dict_val->empty();
            case ValueType::Function: return true;
            case ValueType::Pointer: return ptr_val != nullptr;
            case ValueType::Node: return node_val != nullptr;
            case ValueType::StyleRule: return style_val != nullptr;
        }
        return false;
    }

    double as_float() const {
        if (type == ValueType::Float) return f_val;
        if (type == ValueType::Int) return static_cast<double>(i_val);
        return 0.0;
    }

    int64_t as_int() const {
        if (type == ValueType::Int) return i_val;
        if (type == ValueType::Float) return static_cast<int64_t>(f_val);
        if (type == ValueType::Bool) return b_val ? 1 : 0;
        return 0;
    }

    std::string to_string() const;
    std::string inspect() const;

    size_t length() const {
        if (type == ValueType::String) return s_val.size();
        if (type == ValueType::List && list_val) return list_val->size();
        if (type == ValueType::Dict && dict_val) return dict_val->size();
        return 0;
    }

    // List & Dict operations
    Value get_item(const Value& key_or_index) const;
    void set_item(const Value& key_or_index, const Value& val);
    Value slice(int64_t start, int64_t end, int64_t step = 1) const;

    void append(const Value& val) {
        if (type != ValueType::List) {
            type = ValueType::List;
            list_val = std::make_shared<ValueList>();
        } else if (!list_val) {
            list_val = std::make_shared<ValueList>();
        }
        list_val->push_back(val);
    }

    // Call operator
    Value call(const std::vector<Value>& args = {}) const {
        if (type == ValueType::Function && fn_val) {
            return fn_val(args);
        }
        throw std::runtime_error("Attempted to call a non-function value: " + to_string());
    }

    Value operator()(const std::vector<Value>& args = {}) const {
        return call(args);
    }

    // Binary operations
    Value operator+(const Value& other) const;
    Value operator-(const Value& other) const;
    Value operator*(const Value& other) const;
    Value operator/(const Value& other) const;
    Value operator%(const Value& other) const;

    Value operator==(const Value& other) const;
    Value operator!=(const Value& other) const;
    Value operator<(const Value& other) const;
    Value operator<=(const Value& other) const;
    Value operator>(const Value& other) const;
    Value operator>=(const Value& other) const;
    Value operator!() const { return Value(!is_truthy()); }
};

} // namespace verbrt
