#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <sstream>

namespace verbrt {

enum class SelectorType {
    Tag,        // e.g. div, button, h1
    Class,      // e.g. .btn, .container
    Id,         // e.g. #main, #header
    Universal   // *
};

struct StyleSelector {
    SelectorType type = SelectorType::Tag;
    std::string value; // e.g. "button", "container", "main"

    static StyleSelector parse(const std::string& raw) {
        StyleSelector sel;
        if (raw.empty() || raw == "*") {
            sel.type = SelectorType::Universal;
            sel.value = "*";
        } else if (raw[0] == '.') {
            sel.type = SelectorType::Class;
            sel.value = raw.substr(1);
        } else if (raw[0] == '#') {
            sel.type = SelectorType::Id;
            sel.value = raw.substr(1);
        } else {
            sel.type = SelectorType::Tag;
            sel.value = raw;
        }
        return sel;
    }
};

class VerbStyleRule {
public:
    StyleSelector selector;
    std::unordered_map<std::string, std::string> properties;

    VerbStyleRule() = default;
    explicit VerbStyleRule(const std::string& selector_str)
        : selector(StyleSelector::parse(selector_str)) {}

    void set(const std::string& property, const std::string& value) {
        properties[property] = value;
    }

    std::string get(const std::string& property, const std::string& default_val = "") const {
        auto it = properties.find(property);
        return it != properties.end() ? it->second : default_val;
    }

    bool has(const std::string& property) const {
        return properties.find(property) != properties.end();
    }

    std::string to_css_string() const {
        std::stringstream ss;
        if (selector.type == SelectorType::Class) ss << "." << selector.value;
        else if (selector.type == SelectorType::Id) ss << "#" << selector.value;
        else ss << selector.value;
        ss << " {\n";
        for (const auto& [k, v] : properties) {
            ss << "  " << k << ": " << v << ";\n";
        }
        ss << "}";
        return ss.str();
    }
};

// Global or scoped StyleSheet manager
class StyleSheet {
public:
    static StyleSheet& global() {
        static StyleSheet instance;
        return instance;
    }

    void add_rule(std::shared_ptr<VerbStyleRule> rule) {
        rules_.push_back(rule);
    }

    void add_rule(const std::string& selector, const std::unordered_map<std::string, std::string>& props) {
        auto rule = std::make_shared<VerbStyleRule>(selector);
        rule->properties = props;
        rules_.push_back(rule);
    }

    void clear() {
        rules_.clear();
    }

    const std::vector<std::shared_ptr<VerbStyleRule>>& rules() const {
        return rules_;
    }

private:
    std::vector<std::shared_ptr<VerbStyleRule>> rules_;
};

} // namespace verbrt
