#include <iostream>
#include <tuple>

namespace event {
    struct open { bool state; };
    struct start{ bool state; };
}

template <class OBSERVER, class ARG>
concept IsCall = requires(OBSERVER observer, ARG&& arg) {
    observer.set(std::forward<ARG>(arg));
};

struct call_helper {
    template <class OBSERVER, class ARG> requires IsCall<OBSERVER, ARG>
    void operator()(OBSERVER& observer, ARG&& args) {
        observer.set(std::forward<ARG>(args));
    }

    template <class OBSERVER, class ARG>
    void operator()(OBSERVER&, ARG&&) {}
};

struct proxy {
    template <class E>
    void set(E&& arg);
};

template <class... OBSERVERs>
struct subject {

    static subject& instance() {
        static subject inst;
        return inst;
    }

    template <class E>
    void set(E&& arg) {
        std::apply([&](auto&... observers) {
            (call_helper {}(observers, std::forward<E>(arg)), ...);
        },
        m_observers);
    }

private:
    std::tuple<OBSERVERs...> m_observers;
};

struct observer_net {
    void set(event::open&& arg) {
        if (arg.state) {
            std::cout << __PRETTY_FUNCTION__ << this << "open: " << arg.state << std::endl;
            m_proxy.set(event::start{arg.state}); // call subject::instance().set(arg) via proxy object
        }
        else {
            m_proxy.set(event::start{arg.state}); // call subject::instance().set(arg) via proxy object
            std::cout << __PRETTY_FUNCTION__ << this << "open: " << arg.state << std::endl;
        }
    }

private:
    proxy m_proxy;
};

struct observer_codec {
    void set(event::start&& arg) {
        std::cout << __PRETTY_FUNCTION__ << this << "start: " << arg.state << std::endl;
    }
};

using subject_t = subject<observer_net, observer_codec>;

template <class E>
void proxy::set(E&& arg) {
    subject_t::instance().set(std::forward<E>(arg));
}

int main(int argc, char* argv[])
{
    subject_t::instance().set(event::open { true });
    subject_t::instance().set(event::open { false });

    return 0;
}
