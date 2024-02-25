#include <iostream>
#include <tuple>

template<class... Args>
struct static_event {
    constexpr static_event(Args&... args) : m_objects{args...}
    {}

    template<class... Vars>
    void operator()(Vars... vars) {
        std::apply([&](auto&... tuples){ (tuples(vars...), ...); }, m_objects);
    }

private:
    std::tuple<Args&...> m_objects;
};

struct display_t {
    void operator()(float celsius) {
        std::cout << __PRETTY_FUNCTION__ << celsius << std::endl;
    }
};

struct thermostat_t {
    void operator()(float celsius) {
        std::cout << __PRETTY_FUNCTION__ << celsius << std::endl;
    }
};

struct log_t {
    void operator()(float celsius) {
        std::cout << __PRETTY_FUNCTION__ << celsius << std::endl;
    }
};

int main()
{
    display_t display;
    thermostat_t thermostat;
    log_t log;

    static_event event{display, thermostat, log};

    event(32.5f);
    event(33.5f);
    event(35.5f);

    return 0;
}
