#include "verbrt/verbrt.hpp"
#include <cassert>
#include <iostream>

using namespace verbrt;

void test_values_and_operations() {
    std::cout << "[TEST] Testing Values and Operations...\n";
    Value a(10);
    Value b(25);
    Value c = a + b;
    assert(c.as_int() == 35);

    Value s1("Hello, ");
    Value s2("VerbLang!");
    Value s3 = s1 + s2;
    assert(s3.to_string() == "Hello, VerbLang!");

    Value list = Value::make_list();
    list.append(Value(100));
    list.append(Value("Verbund"));
    list.append(Value(true));
    assert(list.length() == 3);
    assert(list.get_item(Value(0)).as_int() == 100);
    assert(list.get_item(Value(1)).to_string() == "Verbund");

    // Slicing
    Value sliced = list.slice(0, 2);
    assert(sliced.length() == 2);

    // Dict
    Value dict = Value::make_dict();
    dict.set_item(Value("author"), Value("Verbund Labs"));
    dict.set_item(Value("speed"), Value("C++"));
    assert(dict.get_item(Value("author")).to_string() == "Verbund Labs");
    std::cout << "  ✓ Values & Operations passed!\n";
}

void test_memory_and_defer() {
    std::cout << "[TEST] Testing Memory, Arena & Defer...\n";
    bool deferred_ran = false;
    {
        VERB_DEFER(deferred_ran = true);
        assert(!deferred_ran);
    }
    assert(deferred_ran);

    // Raw memory
    void* ptr = raw_alloc(64);
    assert(ptr != nullptr);
    raw_poke64(ptr, 0x123456789ABCDEF0LL);
    int64_t val = raw_peek64(ptr);
    assert(val == 0x123456789ABCDEF0LL);
    raw_free(ptr);

    // Arena Allocator
    ArenaAllocator arena(1024);
    void* m1 = arena.allocate(128);
    void* m2 = arena.allocate(256);
    assert(m1 != nullptr && m2 != nullptr);
    assert(arena.total_bytes_allocated() >= 384);
    std::cout << "  ✓ Memory & Defer passed!\n";
}

void test_dom_and_css() {
    std::cout << "[TEST] Testing DOM and CSS Engine...\n";
    // Register CSS Rule
    StyleSheet::global().add_rule(".badge", {
        {"color", "cyan"},
        {"font-weight", "bold"}
    });
    StyleSheet::global().add_rule("#header", {
        {"border", "1px solid #fff"}
    });

    // Build DOM
    auto root = std::make_shared<VerbNode>("div");
    root->set_attr("id", Value("header"));

    auto h1 = std::make_shared<VerbNode>("h1");
    h1->set_attr("class", Value("badge"));
    h1->add_child(Value("VerbLang Engine"));

    auto btn = std::make_shared<VerbNode>("button");
    btn->add_child(Value("Run Native"));

    root->add_child(Value(h1));
    root->add_child(Value(btn));

    root->compute_styles();

    std::string html = root->to_html();
    assert(html.find("<div id=\"header\">") != std::string::npos);
    assert(html.find("VerbLang Engine") != std::string::npos);

    std::string tui = root->render_tui();
    assert(!tui.empty());
    std::cout << "  Rendered TUI Output:\n" << tui;
    std::cout << "  ✓ DOM and CSS passed!\n";
}

void test_async_event_loop() {
    std::cout << "[TEST] Testing Async Event Loop & Promise...\n";
    auto& loop = EventLoop::instance();
    bool async_done = false;

    auto promise = loop.spawn_async([&]() -> Value {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        async_done = true;
        return Value(42);
    });

    Value res = promise->await_result();
    assert(async_done);
    assert(res.as_int() == 42);
    std::cout << "  ✓ Async Event Loop passed!\n";
}

void test_stdlib_extensions() {
    std::cout << "[TEST] Testing Extended Standard Library...\n";
    // Math
    assert(builtin_abs({Value(-42)}).as_int() == 42);
    assert(builtin_min({Value(10), Value(5), Value(20)}).as_int() == 5);
    assert(builtin_max({Value(10), Value(5), Value(20)}).as_int() == 20);
    assert(builtin_floor({Value(3.7)}).as_float() == 3.0);
    assert(builtin_ceil({Value(3.2)}).as_float() == 4.0);

    // Strings
    assert(builtin_str_starts_with({Value("verblang"), Value("verb")}).is_truthy());
    assert(builtin_str_ends_with({Value("app.verb"), Value(".verb")}).is_truthy());
    assert(builtin_str_replace({Value("hello world"), Value("world"), Value("verb")}).to_string() == "hello verb");

    // Lists
    auto l = Value::make_list();
    l.append(Value("apple"));
    l.append(Value("banana"));
    assert(builtin_list_contains({l, Value("banana")}).is_truthy());
    assert(!builtin_list_contains({l, Value("orange")}).is_truthy());

    // File
    assert(builtin_file_exists({Value("include/verbrt/stdlib.hpp")}).is_truthy());

    // Assert builtin
    assert(builtin_assert({Value(true)}).is_truthy());

    std::cout << "  ✓ Extended Standard Library passed!\n";
}

int main(int argc, char** argv) {
    initialize(argc, argv);
    std::cout << "Running libverbrt native test suite\n";
    std::cout << "-----------------------------------\n";

    test_values_and_operations();
    test_memory_and_defer();
    test_dom_and_css();
    test_async_event_loop();
    test_stdlib_extensions();

    std::cout << "\nAll 5 test suites passed cleanly.\n";
    return 0;
}
