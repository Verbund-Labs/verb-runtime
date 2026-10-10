#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <sstream>
#include "verbrt/value.hpp"
#include "verbrt/css.hpp"

namespace verbrt {

class VerbNode : public std::enable_shared_from_this<VerbNode> {
public:
    std::string tag;
    std::unordered_map<std::string, Value> attributes;
    std::vector<Value> children;
    std::unordered_map<std::string, std::string> computed_styles;

    VerbNode() : tag("div") {}
    explicit VerbNode(const std::string& t) : tag(t) {}

    void set_attr(const std::string& key, const Value& val) {
        attributes[key] = val;
    }

    Value get_attr(const std::string& key, const Value& default_val = Value()) const {
        auto it = attributes.find(key);
        return it != attributes.end() ? it->second : default_val;
    }

    void add_child(const Value& child) {
        children.push_back(child);
    }

    // Resolves and cascades CSS rules into computed_styles
    void compute_styles(const StyleSheet& sheet = StyleSheet::global()) {
        computed_styles.clear();
        std::string class_attr = get_attr("class").to_string();
        std::string id_attr = get_attr("id").to_string();

        for (const auto& rule : sheet.rules()) {
            bool matches = false;
            switch (rule->selector.type) {
                case SelectorType::Universal:
                    matches = true;
                    break;
                case SelectorType::Tag:
                    matches = (tag == rule->selector.value);
                    break;
                case SelectorType::Class:
                    matches = (class_attr.find(rule->selector.value) != std::string::npos);
                    break;
                case SelectorType::Id:
                    matches = (id_attr == rule->selector.value);
                    break;
            }

            if (matches) {
                for (const auto& [k, v] : rule->properties) {
                    computed_styles[k] = v;
                }
            }
        }

        // Inline style attribute override
        Value inline_style = get_attr("style");
        if (inline_style.is_dict() && inline_style.dict_val) {
            for (const auto& [k, v] : *inline_style.dict_val) {
                computed_styles[k] = v.to_string();
            }
        }

        // Recursively compute children styles
        for (auto& child : children) {
            if (child.is_node() && child.node_val) {
                child.node_val->compute_styles(sheet);
            }
        }
    }

    std::string to_html(int indent = 0) const {
        std::stringstream ss;
        std::string pad(indent * 2, ' ');
        ss << pad << "<" << tag;

        for (const auto& [k, v] : attributes) {
            if (k != "children") {
                ss << " " << k << "=\"" << v.to_string() << "\"";
            }
        }

        if (children.empty()) {
            if (tag == "br" || tag == "hr" || tag == "img" || tag == "input") {
                ss << " />\n";
            } else {
                ss << "></" << tag << ">\n";
            }
            return ss.str();
        }

        ss << ">\n";
        for (const auto& child : children) {
            if (child.is_node() && child.node_val) {
                ss << child.node_val->to_html(indent + 1);
            } else {
                ss << pad << "  " << child.to_string() << "\n";
            }
        }
        ss << pad << "</" << tag << ">\n";
        return ss.str();
    }

    // Terminal ANSI colored output honoring CSS styling
    std::string render_tui(int indent = 0) const {
        std::stringstream ss;
        std::string pad(indent * 2, ' ');

        // ANSI Color mappings
        std::string color_code = "";
        std::string reset_code = "\033[0m";
        std::string bold_code = "";

        auto color_it = computed_styles.find("color");
        if (color_it != computed_styles.end()) {
            std::string c = color_it->second;
            if (c == "red" || c == "#ff0000") color_code = "\033[31m";
            else if (c == "green" || c == "#00ff00") color_code = "\033[32m";
            else if (c == "yellow" || c == "#ffff00") color_code = "\033[33m";
            else if (c == "blue" || c == "#0000ff") color_code = "\033[34m";
            else if (c == "magenta" || c == "purple") color_code = "\033[35m";
            else if (c == "cyan" || c == "#00ffff") color_code = "\033[36m";
            else if (c == "white") color_code = "\033[37m";
        }

        auto weight_it = computed_styles.find("font-weight");
        if (weight_it != computed_styles.end() && weight_it->second == "bold") {
            bold_code = "\033[1m";
        }

        bool has_border = computed_styles.find("border") != computed_styles.end();

        if (has_border) {
            ss << pad << "┌──────────────────────────────────────────────┐\n";
        }

        if (tag == "h1" || tag == "h2" || tag == "h3") {
            bold_code = "\033[1m";
        }

        if (tag == "button") {
            ss << pad << " [ " << color_code << bold_code;
            for (const auto& child : children) {
                ss << child.to_string() << " ";
            }
            ss << reset_code << "]\n";
        } else {
            for (const auto& child : children) {
                if (child.is_node() && child.node_val) {
                    ss << child.node_val->render_tui(indent + (has_border ? 1 : 0));
                } else {
                    ss << pad << (has_border ? "│ " : "") << color_code << bold_code
                       << child.to_string() << reset_code << "\n";
                }
            }
        }

        if (has_border) {
            ss << pad << "└──────────────────────────────────────────────┘\n";
        }

        return ss.str();
    }
};

} // namespace verbrt
