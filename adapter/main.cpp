#include <algorithm>
#include <array>
#include <iostream>

template<class Container>
struct add_algorithms: Container {

    template<class F>
    auto &for_each(F &&f)
    {
        auto p = static_cast<Container *>(this);
        std::for_each(p->begin(), p->end(), f);
        return *this;
    }

    template<class F>
    auto &for_each_n(F &&f)
    {
        auto p = static_cast<Container *>(this);
        auto first = p->begin();
        auto last = p->end();
        std::size_t i{0};
        for (; first != last; ++first, ++i)
            f(*first, i);
        return *this;
    }

    auto &reverse()
    {
        auto p = static_cast<Container *>(this);
        auto first = p->begin();
        auto last = p->end();
        std::reverse(first, last);
        return *this;
    }

    template<class F>
    auto &then(F &&f)
    {
        f(*this);
        return *this;
    }
};


template<class T, std::size_t N>
using my_array = add_algorithms<std::array<T, N>>;


int main(int argc, char *argv[])
{
    my_array<int, 6> t_array{0, 1, 2, 3, 4, 5};

    t_array.for_each_n([](auto value, auto index) {
        std::cout << "value: " << value << ", index: " << index << std::endl;
    })
    .reverse()
    .then([](auto &) {
        std::cout << "reverse" << std::endl;
    })
    .for_each_n([](auto value, auto index) {
        std::cout << "value: " << value << ", index: " << index << std::endl;
    });

    return 0;
}
