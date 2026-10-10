#pragma once

#include "verbrt/value.hpp"
#include <string>
#include <sstream>
#include <cctype>

namespace verbrt {

class Json {
public:
    static std::string stringify(const Value& val) {
        switch (val.type) {
            case ValueType::Nil: return "null";
            case ValueType::Bool: return val.b_val ? "true" : "false";
            case ValueType::Int: return std::to_string(val.i_val);
            case ValueType::Float: {
                std::ostringstream ss;
                ss << val.f_val;
                return ss.str();
            }
            case ValueType::String: {
                std::stringstream ss;
                ss << "\"";
                for (char c : val.s_val) {
                    if (c == '\"') ss << "\\\"";
                    else if (c == '\\') ss << "\\\\";
                    else if (c == '\n') ss << "\\n";
                    else if (c == '\t') ss << "\\t";
                    else if (c == '\r') ss << "\\r";
                    else ss << c;
                }
                ss << "\"";
                return ss.str();
            }
            case ValueType::List: {
                if (!val.list_val) return "[]";
                std::stringstream ss;
                ss << "[";
                for (size_t i = 0; i < val.list_val->size(); ++i) {
                    if (i > 0) ss << ", ";
                    ss << stringify((*val.list_val)[i]);
                }
                ss << "]";
                return ss.str();
            }
            case ValueType::Dict: {
                if (!val.dict_val) return "{}";
                std::stringstream ss;
                ss << "{";
                size_t i = 0;
                for (const auto& [k, v] : *val.dict_val) {
                    if (i++ > 0) ss << ", ";
                    ss << "\"" << k << "\": " << stringify(v);
                }
                ss << "}";
                return ss.str();
            }
            default:
                return "null";
        }
    }

    static Value parse(const std::string& json_str) {
        size_t cursor = 0;
        skip_ws(json_str, cursor);
        return parse_value(json_str, cursor);
    }

private:
    static void skip_ws(const std::string& str, size_t& cursor) {
        while (cursor < str.size() && std::isspace(static_cast<unsigned char>(str[cursor]))) {
            cursor++;
        }
    }

    static Value parse_value(const std::string& str, size_t& cursor) {
        skip_ws(str, cursor);
        if (cursor >= str.size()) return Value();

        char c = str[cursor];
        if (c == '{') return parse_object(str, cursor);
        if (c == '[') return parse_array(str, cursor);
        if (c == '\"') return parse_string(str, cursor);
        if (c == 't' || c == 'f') return parse_bool(str, cursor);
        if (c == 'n') return parse_null(str, cursor);
        if (std::isdigit(static_cast<unsigned char>(c)) || c == '-') return parse_number(str, cursor);

        return Value();
    }

    static Value parse_object(const std::string& str, size_t& cursor) {
        cursor++; // consume '{'
        skip_ws(str, cursor);
        auto dict = std::make_shared<ValueDict>();

        if (cursor < str.size() && str[cursor] == '}') {
            cursor++;
            return Value(dict);
        }

        while (cursor < str.size()) {
            skip_ws(str, cursor);
            if (cursor >= str.size() || str[cursor] != '\"') break;
            std::string key = parse_string(str, cursor).to_string();
            skip_ws(str, cursor);
            if (cursor >= str.size() || str[cursor] != ':') break;
            cursor++; // consume ':'
            skip_ws(str, cursor);
            Value val = parse_value(str, cursor);
            (*dict)[key] = val;
            skip_ws(str, cursor);
            if (cursor < str.size() && str[cursor] == ',') {
                cursor++;
            } else if (cursor < str.size() && str[cursor] == '}') {
                cursor++;
                break;
            } else {
                break;
            }
        }
        return Value(dict);
    }

    static Value parse_array(const std::string& str, size_t& cursor) {
        cursor++; // consume '['
        skip_ws(str, cursor);
        auto list = std::make_shared<ValueList>();

        if (cursor < str.size() && str[cursor] == ']') {
            cursor++;
            return Value(list);
        }

        while (cursor < str.size()) {
            skip_ws(str, cursor);
            Value val = parse_value(str, cursor);
            list->push_back(val);
            skip_ws(str, cursor);
            if (cursor < str.size() && str[cursor] == ',') {
                cursor++;
            } else if (cursor < str.size() && str[cursor] == ']') {
                cursor++;
                break;
            } else {
                break;
            }
        }
        return Value(list);
    }

    static Value parse_string(const std::string& str, size_t& cursor) {
        cursor++; // consume '"'
        std::string res;
        while (cursor < str.size() && str[cursor] != '\"') {
            if (str[cursor] == '\\' && cursor + 1 < str.size()) {
                cursor++;
                char esc = str[cursor++];
                if (esc == 'n') res += '\n';
                else if (esc == 't') res += '\t';
                else if (esc == 'r') res += '\r';
                else if (esc == '\"') res += '\"';
                else if (esc == '\\') res += '\\';
                else res += esc;
            } else {
                res += str[cursor++];
            }
        }
        if (cursor < str.size() && str[cursor] == '\"') cursor++;
        return Value(res);
    }

    static Value parse_bool(const std::string& str, size_t& cursor) {
        if (str.substr(cursor, 4) == "true") {
            cursor += 4;
            return Value(true);
        }
        if (str.substr(cursor, 5) == "false") {
            cursor += 5;
            return Value(false);
        }
        return Value(false);
    }

    static Value parse_null(const std::string& str, size_t& cursor) {
        if (str.substr(cursor, 4) == "null") {
            cursor += 4;
        }
        return Value();
    }

    static Value parse_number(const std::string& str, size_t& cursor) {
        size_t start = cursor;
        bool has_dot = false;
        if (str[cursor] == '-') cursor++;
        while (cursor < str.size() && (std::isdigit(static_cast<unsigned char>(str[cursor])) || str[cursor] == '.')) {
            if (str[cursor] == '.') has_dot = true;
            cursor++;
        }
        std::string num_str = str.substr(start, cursor - start);
        if (has_dot) {
            return Value(std::stod(num_str));
        } else {
            return Value(static_cast<int64_t>(std::stoll(num_str)));
        }
    }
};

} // namespace verbrt
