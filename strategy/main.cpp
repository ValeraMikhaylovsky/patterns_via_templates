#include <iostream>
#include <variant>

namespace meta {
    template <size_t N, typename T, typename... Ts>
    struct get_type_by_index {
        using type = typename get_type_by_index<N - 1, Ts...>::type;
    };

    template <typename T, typename... Ts>
    struct get_type_by_index<0, T, Ts...> {
        using type = T;
    };

    template <size_t N, typename... Ts>
    using get_type_from_index = typename get_type_by_index<N, Ts...>::type;
}

struct blue_blink {
    void timeout(int ms) {
        std::cout << __PRETTY_FUNCTION__ << " " << ms << std::endl;
    }
};

struct green_blink {
    void timeout(int ms) {
        std::cout << __PRETTY_FUNCTION__ << " " << ms << std::endl;
    }
};

struct red_blink {
    void timeout(int ms) {
        std::cout << __PRETTY_FUNCTION__ << " " << ms << std::endl;
    }
};

template <class... Ts>
struct context {

    template <class U>
    void setStrategy(U&& strategy) {
        m_strategy = std::move(strategy);
    }

    void update(int ms) {
        std::visit([&](auto&& strategy) {
            strategy.timeout(ms);
        },
        m_strategy);
    }

private:
    using init_type = typename meta::get_type_from_index<0, Ts...>;
    std::variant<Ts...> m_strategy { init_type {} };
};

using context_t = context<blue_blink, green_blink, red_blink>;

int main(int argc, char* argv[])
{
    context_t m_ctx;
    m_ctx.update(5);
    m_ctx.setStrategy(green_blink {});
    m_ctx.update(5);
    m_ctx.setStrategy(red_blink {});
    m_ctx.update(5);

    return 0;
}
