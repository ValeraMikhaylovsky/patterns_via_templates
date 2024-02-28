#include <iostream>
#include <tuple>
#include <string_view>

struct proc_text_run {
    bool operator()(std::string_view arg) {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
        return arg.compare("run") == 0;
    }
};

struct proc_text_stop {
    bool operator()(std::string_view arg) {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
        return arg.compare("stop") == 0;
    }
};


struct proc_text_busy {
    bool operator()(std::string_view arg) {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
        return arg.compare("busy") == 0;
    }
};


template<class... OBJs>
struct chain_of_responsibility {

    bool process(std::string_view arg) {
        return std::apply([&](auto&... tuples){
            return (tuples(arg) || ...);
        }, m_objects);

    }

private:
    std::tuple<OBJs...> m_objects;
};

using chain_t = chain_of_responsibility<proc_text_run, proc_text_stop, proc_text_busy>;

int main()
{
    chain_t object;

    object.process("run");
    std::cout << std::endl;
    object.process("stop");
    std::cout << std::endl;
    object.process("busy");
    std::cout << std::endl;
    object.process("empty");

    return 0;
}
