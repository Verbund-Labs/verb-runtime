#include "verbrt/value.hpp"
#include "verbrt/dom.hpp"
#include "verbrt/css.hpp"
#include <iomanip>

namespace verbrt {

std::string Value::to_string() const {
    switch (type) {
        case ValueType::Nil: return "nil";
        case ValueType::Bool: return b_val ? "true" : "false";
        case ValueType::Int: return std::to_string(i_val);
        case ValueType::Float: {
            std::ostringstream ss;
            ss << f_val;
            return ss.str();
        }
        case ValueType::String: return s_val;
        case ValueType::List: {
            if (!list_val) return "[]";
            std::stringstream ss;
            ss << "[";
            for (size_t i = 0; i < list_val->size(); ++i) {
                if (i > 0) ss << ", ";
                ss << (*list_val)[i].inspect();
            }
            ss << "]";
            return ss.str();
        }
        case ValueType::Dict: {
            if (!dict_val) return "{}";
            std::stringstream ss;
            ss << "{";
            size_t i = 0;
            for (const auto& [k, v] : *dict_val) {
                if (i++ > 0) ss << ", ";
                ss << "\"" << k << "\": " << v.inspect();
            }
            ss << "}";
            return ss.str();
        }
        case ValueType::Function: return "<function>";
        case ValueType::Pointer: {
            std::stringstream ss;
            ss << "<pointer: 0x" << std::hex << reinterpret_cast<uintptr_t>(ptr_val) << ">";
            return ss.str();
        }
        case ValueType::Node: {
            return node_val ? node_val->to_html() : "<null-node>";
        }
        case ValueType::StyleRule: {
            return style_val ? style_val->to_css_string() : "<null-style>";
        }
    }
    return "";
}

std::string Value::inspect() const {
    if (type == ValueType::String) {
        std::stringstream ss;
        ss << std::quoted(s_val);
        return ss.str();
    }
    return to_string();
}

Value Value::get_item(const Value& key_or_index) const {
    if (type == ValueType::List && list_val) {
        int64_t idx = key_or_index.as_int();
        if (idx < 0) idx += static_cast<int64_t>(list_val->size());
        if (idx < 0 || idx >= static_cast<int64_t>(list_val->size())) {
            throw std::out_of_range("List index out of range: " + std::to_string(idx));
        }
        return (*list_val)[static_cast<size_t>(idx)];
    }
    if (type == ValueType::Dict && dict_val) {
        std::string key = key_or_index.to_string();
        auto it = dict_val->find(key);
        if (it != dict_val->end()) {
            return it->second;
        }
        return Value(); // nil
    }
    if (type == ValueType::String) {
        int64_t idx = key_or_index.as_int();
        if (idx < 0) idx += static_cast<int64_t>(s_val.size());
        if (idx < 0 || idx >= static_cast<int64_t>(s_val.size())) {
            throw std::out_of_range("String index out of range: " + std::to_string(idx));
        }
        return Value(std::string(1, s_val[static_cast<size_t>(idx)]));
    }
    if (type == ValueType::Node && node_val) {
        return node_val->get_attr(key_or_index.to_string());
    }
    throw std::runtime_error("Type does not support indexing: " + to_string());
}

void Value::set_item(const Value& key_or_index, const Value& val) {
    if (type == ValueType::List) {
        if (!list_val) list_val = std::make_shared<ValueList>();
        int64_t idx = key_or_index.as_int();
        if (idx < 0) idx += static_cast<int64_t>(list_val->size());
        if (idx < 0 || idx >= static_cast<int64_t>(list_val->size())) {
            throw std::out_of_range("List index out of range: " + std::to_string(idx));
        }
        (*list_val)[static_cast<size_t>(idx)] = val;
        return;
    }
    if (type == ValueType::Dict) {
        if (!dict_val) dict_val = std::make_shared<ValueDict>();
        (*dict_val)[key_or_index.to_string()] = val;
        return;
    }
    if (type == ValueType::Node && node_val) {
        node_val->set_attr(key_or_index.to_string(), val);
        return;
    }
    throw std::runtime_error("Type does not support assignment indexing: " + to_string());
}

Value Value::slice(int64_t start, int64_t end, int64_t step) const {
    if (step == 0) throw std::invalid_argument("Slice step cannot be 0");
    if (type == ValueType::List && list_val) {
        int64_t len = static_cast<int64_t>(list_val->size());
        if (start < 0) start += len;
        if (end < 0) end += len;
        start = std::max(int64_t(0), std::min(len, start));
        end = std::max(int64_t(0), std::min(len, end));

        auto result = std::make_shared<ValueList>();
        if (step > 0) {
            for (int64_t i = start; i < end; i += step) {
                result->push_back((*list_val)[static_cast<size_t>(i)]);
            }
        } else {
            for (int64_t i = start; i > end; i += step) {
                result->push_back((*list_val)[static_cast<size_t>(i)]);
            }
        }
        return Value(result);
    }
    if (type == ValueType::String) {
        int64_t len = static_cast<int64_t>(s_val.size());
        if (start < 0) start += len;
        if (end < 0) end += len;
        start = std::max(int64_t(0), std::min(len, start));
        end = std::max(int64_t(0), std::min(len, end));

        std::string res;
        for (int64_t i = start; i < end; i += step) {
            res += s_val[static_cast<size_t>(i)];
        }
        return Value(res);
    }
    throw std::runtime_error("Type does not support slicing: " + to_string());
}

Value Value::operator+(const Value& other) const {
    if (is_int() && other.is_int()) {
        return Value(i_val + other.i_val);
    }
    if (is_number() && other.is_number()) {
        return Value(as_float() + other.as_float());
    }
    if (is_string() || other.is_string()) {
        return Value(to_string() + other.to_string());
    }
    if (is_list() && other.is_list()) {
        auto result = std::make_shared<ValueList>(*list_val);
        if (other.list_val) {
            result->insert(result->end(), other.list_val->begin(), other.list_val->end());
        }
        return Value(result);
    }
    throw std::runtime_error("Invalid operands for +");
}

Value Value::operator-(const Value& other) const {
    if (is_int() && other.is_int()) return Value(i_val - other.i_val);
    if (is_number() && other.is_number()) return Value(as_float() - other.as_float());
    throw std::runtime_error("Invalid operands for -");
}

Value Value::operator*(const Value& other) const {
    if (is_int() && other.is_int()) return Value(i_val * other.i_val);
    if (is_number() && other.is_number()) return Value(as_float() * other.as_float());
    if (is_string() && other.is_int()) {
        std::string res;
        for (int64_t i = 0; i < other.i_val; ++i) res += s_val;
        return Value(res);
    }
    throw std::runtime_error("Invalid operands for *");
}

Value Value::operator/(const Value& other) const {
    if (other.is_number() && other.as_float() == 0.0) {
        throw std::runtime_error("Division by zero");
    }
    if (is_int() && other.is_int()) {
        if (i_val % other.i_val == 0) return Value(i_val / other.i_val);
        return Value(static_cast<double>(i_val) / static_cast<double>(other.i_val));
    }
    if (is_number() && other.is_number()) return Value(as_float() / other.as_float());
    throw std::runtime_error("Invalid operands for /");
}

Value Value::operator%(const Value& other) const {
    if (is_int() && other.is_int()) {
        if (other.i_val == 0) throw std::runtime_error("Modulo by zero");
        return Value(i_val % other.i_val);
    }
    throw std::runtime_error("Invalid operands for %");
}

Value Value::operator==(const Value& other) const {
    if (type != other.type) {
        if (is_number() && other.is_number()) return Value(as_float() == other.as_float());
        return Value(false);
    }
    switch (type) {
        case ValueType::Nil: return Value(true);
        case ValueType::Bool: return Value(b_val == other.b_val);
        case ValueType::Int: return Value(i_val == other.i_val);
        case ValueType::Float: return Value(f_val == other.f_val);
        case ValueType::String: return Value(s_val == other.s_val);
        case ValueType::Pointer: return Value(ptr_val == other.ptr_val);
        default: return Value(this == &other);
    }
}

Value Value::operator!=(const Value& other) const {
    return Value(!((*this == other).b_val));
}

Value Value::operator<(const Value& other) const {
    if (is_int() && other.is_int()) return Value(i_val < other.i_val);
    if (is_number() && other.is_number()) return Value(as_float() < other.as_float());
    if (is_string() && other.is_string()) return Value(s_val < other.s_val);
    throw std::runtime_error("Invalid operands for <");
}

Value Value::operator<=(const Value& other) const {
    return Value((*this < other).b_val || (*this == other).b_val);
}

Value Value::operator>(const Value& other) const {
    return Value(!((*this <= other).b_val));
}

Value Value::operator>=(const Value& other) const {
    return Value(!((*this < other).b_val));
}

} // namespace verbrt
