#pragma once

#include <type_traits>
#include <concepts>
#include <variant>

namespace meta_fsm {

namespace meta {

    template <typename T, typename... Ts>
    struct unique : std::type_identity<T> {};

    template <typename... Ts, typename U, typename... Us>
    struct unique<std::variant<Ts...>, U, Us...>
                            : std::conditional_t<(std::is_same_v<U, Ts> || ...)
                            , unique<std::variant<Ts...>, Us...>
                            , unique<std::variant<Ts..., U>, Us...>> {};

    template <typename... Ts>
    using unique_variant = typename unique<std::variant<>, Ts...>::type;


    template <class... Ts>
    struct type_pack {};

    template<std::size_t N, typename T, typename... Ts>
    struct get_type_by_index {
        using type = typename get_type_by_index<N - 1, Ts...>::type;
    };

    template<typename T, typename... Ts>
    struct get_type_by_index<0, T, Ts...> {
        using type = T;
    };

    template<std::size_t N, typename... Ts>
    using get_type_from_index = typename get_type_by_index<N, Ts...>::type;

    template <class T>
    constexpr bool contains(type_pack<>) noexcept {
        return false;
    }

    template <class T, class... Ts>
    constexpr bool contains(type_pack<Ts...>) noexcept {
        return (... || std::is_same<T, Ts>::value);
    }

    template <typename T, typename... Ts>
    constexpr std::size_t find() noexcept {
        constexpr bool is_true[] = {std::is_same_v<T, Ts>...};
        for (std::size_t i {0}; i < sizeof...(Ts); ++i) {
            if (is_true[i])
                return i;
        }
        return sizeof...(Ts);
    }

    template <class T, class... Ts>
    struct count_of {
        static constexpr std::size_t value = (static_cast<std::size_t>(std::is_same<T, Ts>::value) + ...);
    };

    template <class... Ts>
    struct no_dublicates {
        static constexpr bool value = ((count_of<Ts, Ts...>::value == 1) && ...);
    };

    template <typename T, typename... Ts>
    struct non_void : std::type_identity<T> {};

    template <typename... Ts, typename U, typename... Us>
    struct non_void<type_pack<Ts...>, U, Us...>
                            : std::conditional_t<(std::is_same_v<void, U>)
                            , non_void<type_pack<Ts...>, Us...>
                            , non_void<type_pack<Ts..., U>, Us...>> {};

    template <typename... Ts>
    using non_void_type_pack = typename non_void<type_pack<>, Ts...>::type;

}


struct base_state {};

template<class Child>
struct state : base_state {
    using internal_transitions = void;
};

template<class T>
concept IsState = std::is_base_of_v<base_state, T>;

template<class S, class E, class F>
concept CallOnEntry = requires(S state, E event, F fsm) {
    state.on_entry(event, fsm);
};

template<class S, class F>
concept CallShortOnEntry = requires(S state, F fsm) {
    state.on_entry(fsm);
};

template<class S, class E, class F>
concept CallOnExit = requires(S state, E event, F fsm) {
    state.on_exit(event, fsm);
};

template<class S, class F>
concept CallShortOnExit = requires(S state, F fsm) {
    state.on_exit(fsm);
};

struct call_on_entry {
    template<class State, class FSM, class E> requires CallOnEntry<State, E, FSM>
    void operator()(State &state, const E &event, FSM &fsm) const noexcept { state.on_entry(event, fsm); }

    template<class State, class FSM, class E>
    void operator()(State &state, const E &e, FSM &fsm) const noexcept {}

    template<class State, class FSM> requires CallShortOnEntry<State, FSM>
    void operator()(State &state, FSM &fsm) const noexcept { state.on_entry(fsm); }

    template<class State, class FSM>
    void operator()(State &state, FSM &fsm) const noexcept {}
};

struct call_on_exit {
    template<class State, class FSM, class E> requires CallOnExit<State, E, FSM>
    void operator()(State &state, const E &event, FSM &fsm) const noexcept { state.on_exit(event, fsm); }

    template<class State, class FSM, class E>
    void operator()(State &state, const E &e, FSM &fsm) const noexcept {}

    template<class State, class FSM> requires CallShortOnExit<State, FSM>
    void operator()(State &state, FSM &fsm) const noexcept { state.on_exit(fsm); }

    template<class State, class FSM>
    void operator()(State &state, FSM &fsm) const noexcept {}
};

template<class A, class E, class F, class S, class D>
concept CallActionLong = requires(A action, E event, F fsm, S src, D dst) {
    action(event, fsm, src, dst);
};

template<class A, class E, class F>
concept CallActionShort = requires(A action, E event, F fsm) {
    action(event, fsm);
};

template <class A>
struct call_action {
    template<class E, class F, class S, class D> requires CallActionLong<A, E, F, S, D>
    void operator()(const E &event, F &fsm, S &src, D &dst) noexcept {
        A{}(event, fsm, src, dst);
    }

    template<class E, class F, class S, class D> requires CallActionShort<A, E, F>
    void operator()(const E &event, F &fsm, S&, D&) noexcept {
        A{}(event, fsm);
    }

    template<class E, class F, class S, class D>
    void operator()(const E&, F&, S&, D&) noexcept {}
};

template<class G, class E, class F, class S>
concept CallGuard = requires(G guard, E event, F fsm, S src) {
    { guard(event, fsm, src) } -> std::convertible_to<bool>;
};

template<class G, class E, class F>
concept CallGuardShort = requires(G guard, E event, F fsm) {
    { guard(event, fsm) } -> std::convertible_to<bool>;
};

template  <class G>
struct call_guard {
    template<class E, class F, class S> requires CallGuard<G, E, F, S>
    bool operator()(const E &event, const F &fsm, const S &src) const noexcept {
        return G{}(event, fsm, src);
    }

    template<class E, class F, class S> requires CallGuardShort<G, E, F>
    bool operator()(const E &event, const F &fsm, const S &) const noexcept {
        return G{}(event, fsm);
    }

    template<class E, class F, class S>
    bool operator()(const E &, const F &, const S &) const noexcept {
        return true;
    }
};

template  <class G>
struct not_ {
    template<class E, class F, class S> requires CallGuard<G, E, F, S>
    bool operator()(const E &event, const F &fsm, const S &src) const noexcept {
        return !G{}(event, fsm, src);
    }

    template<class E, class F, class S> requires CallGuardShort<G, E, F>
    bool operator()(const E &event, const F &fsm, const S &) const noexcept {
        return !G{}(event, fsm);
    }

    template<class E, class F, class S>
    bool operator()(const E &, const F &, const S &) const noexcept {
        return false;
    }
};

template <class... G>
struct and_ {
    template<class E, class F, class S> requires ((CallGuard<call_guard<G>, E, F, S>) && ...)
    bool operator()(const E &event, const F &fsm, const S &src) const noexcept {
        return (call_guard<G>{}(event, fsm, src) && ...);
    }

    template<class E, class F, class S>
    bool operator()(const E &, const F &, const S &) const noexcept {
        return true;
    }
};

template <>
struct and_<> {
    template <class... Args>
    bool operator()(Args&&...) const noexcept {
        return false;
    }
};

template <class... G>
struct or_ {
    template<class E, class F, class S> requires ((CallGuard<call_guard<G>, E, F, S>) && ...)
    bool operator()(const E &event, const F &fsm, const S &src) const noexcept {
        return (call_guard<G>{}(event, fsm, src) || ...);
    }

    template<class E, class F, class S>
    bool operator()(const E &, const F &, const S &) const noexcept {
        return true;
    }
};

template <>
struct or_<> {
    template <class... Args>
    bool operator()(Args&&...) const noexcept {
        return true;
    }
};

struct none {};

struct transition_base {};

template <IsState Source, class Event, IsState Target, class Action = none, class Guard = none>
struct tr : transition_base
{
    using source_t = Source;
    using event_t  = Event;
    using target_t = Target;
    using action_t = Action;
    using guard_t  = Guard;
    using source_tr_t = typename source_t::internal_transitions;
    using target_tr_t = typename target_t::internal_transitions;
    using tag_t = meta::type_pack<source_t, event_t>;

    static_assert(!std::is_same_v<Source, Target>, "source state and target state is same!");
};

template <class Event, class Action, class Guard = none>
struct in : transition_base {
    using event_t  = Event;
    using action_t = Action;
    using guard_t  = Guard;
    using source_t = void;
    using target_t = void;
    using source_tr_t = void;
    using target_tr_t = void;
    using tag_t = meta::type_pack<event_t, action_t>;
};

template<typename T>
concept IsTransition = std::is_base_of_v<transition_base, T>;

template<IsTransition ...T>
struct transition_table {
    static_assert(sizeof...(T) > 0, "transition_table must be not empty!");
    static_assert(meta::no_dublicates<typename T::tag_t...>::value, "transition table contains duplicates");
    struct empty_state : state<empty_state> {};
    using states_t = meta::unique_variant<typename T::source_t..., typename T::target_t..., empty_state>;
    using all_states_t = meta::type_pack<typename T::source_t..., typename T::target_t...>;
    using events_t = meta::type_pack<typename T::event_t...>;
    static constexpr std::size_t count = sizeof...(T);
    using internal_transitions = meta::non_void_type_pack<typename T::source_tr_t..., typename T::target_tr_t...>;

    template <class Tag>
    static inline constexpr std::size_t index_of() noexcept {
        constexpr bool bs[] = {std::is_same_v<Tag, typename T::tag_t>...};
        for (std::size_t index {0}; index < sizeof...(T); ++index) {
            if (bs[index])
                return index;
        }
        return sizeof...(T);
    }

    template <class E>
    static inline constexpr std::size_t internal_index_of() noexcept {
        constexpr bool bs[] = {(std::is_same_v<E, typename T::event_t>)...};
        for (std::size_t index {0}; index < sizeof...(T); ++index) {
            if (bs[index])
                return index;
        }
        return sizeof...(T);
    }

    template<std::size_t I>
    struct get_transition_type {
        using type = meta::get_type_from_index<I, T...>;
    };

    template <class U>
    static constexpr bool contains_in_table(meta::type_pack<>&&) noexcept {
        return false;
    }

    template <class E, class... Es>
    static constexpr bool contains_in_table(meta::type_pack<Es...> &&) noexcept {
        return (... || contains<E>(typename Es::events_t{}));
    }

    template<class E>
    struct has_event {
        static constexpr bool value = meta::contains<E>(events_t{}) || contains_in_table<E>(internal_transitions{});
    };
};

enum class result {
    refuse,
    done
};

template<class T> requires requires { typename T::transitions; typename T::initial_state; }
struct state_machine : T {
    using transitions_t = typename T::transitions;
    using initial_state_t = typename T::initial_state;
    using events_t = typename transitions_t::events_t;

    state_machine(const state_machine&) = delete;
    state_machine& operator=(const state_machine&) = delete;

    state_machine() noexcept : T{} {
        std::visit([&, fsm = static_cast<T&>(*this)](auto &t_current) {
            call_on_entry{}(t_current, fsm);
        }, m_current);
    }

    template <typename... Args>
    constexpr explicit state_machine(Args&&... args) noexcept : T{std::forward<Args>(args)...} {
        std::visit([&, fsm = static_cast<T&>(*this)](auto &t_current) {
            call_on_entry{}(t_current, fsm);
        }, m_current);
    }

    ~state_machine() {
        std::visit([&, fsm = static_cast<T&>(*this)](auto &t_current) {
            call_on_exit{}(t_current, fsm);
        }, m_current);
    }

    template<class E>
    result process_event(E &&event) noexcept {
        static_assert(transitions_t::template has_event<E>::value, "unknown event type, this type is missing from the transition table!");
        auto t_result {result::refuse};
        T &fsm = static_cast<T&>(*this);
        std::visit([&](auto &t_source) noexcept {
            using source_t = std::decay_t<decltype(t_source)>;
            using event_t = std::decay_t<decltype(event)>;
            constexpr auto tr_index = transitions_t::template index_of<meta::type_pack<source_t,event_t>>();
            if constexpr (tr_index < transitions_t::count) {
                using transition_t = typename transitions_t::template get_transition_type<tr_index>::type;
                using guard_t  = typename transition_t::guard_t;
                using action_t = typename transition_t::action_t;
                using target_t = typename transition_t::target_t;

                if (!call_guard<guard_t>{}(std::forward<E>(event), fsm, t_source))
                    return;

                call_on_exit{}(t_source, std::forward<E>(event), fsm);
                m_current = typename transitions_t::empty_state{};
                target_t t_target{};
                call_action<action_t>{}(std::forward<E>(event), fsm, t_source, t_target);
                m_current = std::move(t_target);
                call_on_entry{}(*std::get_if<target_t>(&m_current), std::forward<E>(event), fsm);
                t_result = result::done;
            }
            else {
                using internal_transitions_t = typename std::decay_t<decltype(t_source)>::internal_transitions;
                if constexpr (!std::is_same_v<internal_transitions_t, void>) {
                    using event_t = std::decay_t<decltype(event)>;
                    constexpr auto in_index = internal_transitions_t::template internal_index_of<event_t>();
                    if constexpr (in_index < internal_transitions_t::count) {
                        using transition_t = typename internal_transitions_t::template get_transition_type<in_index>::type;
                        using action_t = typename transition_t::action_t;
                        using guard_t  = typename transition_t::guard_t;
                        if (call_guard<guard_t>{}(std::forward<E>(event), fsm, t_source)) {
                            call_action<action_t>{}(std::forward<E>(event), fsm, t_source, t_source);
                            t_result = result::done;
                        }
                    }
                }
            }
        }, m_current);
        return t_result;
    }

    template <IsState State>
    [[nodiscard]] constexpr bool is_in_state() const noexcept {
        static_assert(meta::contains<State>(typename transitions_t::all_states_t{}), "the state is missing from the transitions table");
        return std::holds_alternative<State>(m_current);
    }

private:
    typename transitions_t::states_t m_current {initial_state_t{}};
};


}


