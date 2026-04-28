//! \file Replaces (overrides) global new and delete to collect memory usage statistics.
/** Usage: You need to link the implementation file of this header to your target. Then every
    use of new/delete will be replaced with the custom version here (even standard library calls, everything).
    So please take care that you're absolutely conscious whether you want to do this.*/

#ifndef NEW_STATISTICS_H_IUG34GH347XH38GX348GX34YG2G
#define NEW_STATISTICS_H_IUG34GH347XH38GX348GX34YG2G

#include <atomic>
#include <cstddef>
#include <ostream>

namespace mb::alloc_statistics {
struct Bytes {
    size_t value{};

    constexpr Bytes& operator+=(Bytes rhs) noexcept {
        value += rhs.value;
        return *this;
    }

    constexpr Bytes& operator-=(Bytes rhs) noexcept {
        value -= rhs.value;
        return *this;
    }

    [[nodiscard]] constexpr explicit operator bool() const noexcept {
        return value != 0;
    }

    [[nodiscard]] constexpr bool operator==(const Bytes& rhs) const noexcept {
        return value == rhs.value;
    }
};

[[nodiscard]] inline constexpr Bytes operator+(Bytes a, Bytes b) noexcept {
    a += b;
    return a;
}

[[nodiscard]] inline constexpr Bytes operator-(Bytes a, Bytes b) noexcept {
    a -= b;
    return a;
}

inline std::ostream& operator<<(std::ostream& os, Bytes b) {
    return os << b.value;
}

class Statistics {
public:
    [[nodiscard]] static Statistics& instance() {
        static Statistics stats;
        return stats;
    }

    // Called by operator new overrides; p points to the raw allocation (before user data).
    void new_call(Bytes size, void* p) noexcept;
    // Called by operator delete overrides; p points to the raw allocation (before user data).
    void delete_call(void* p) noexcept;

    [[nodiscard]] size_t new_calls() const noexcept {
        return new_calls_.load();
    }

    [[nodiscard]] size_t delete_calls() const noexcept {
        return delete_calls_.load();
    }

    [[nodiscard]] Bytes allocated_size() const noexcept {
        return Bytes{allocated_size_.load()};
    }

    [[nodiscard]] Bytes deallocated_size() const noexcept {
        return Bytes{deallocated_size_.load()};
    }

    /** \return the maximum/peak size allocated.*/
    [[nodiscard]] Bytes peak_size() const noexcept {
        return Bytes{peak_size_.load()};
    }

    void reset() noexcept {
        new_calls_ = {};
        delete_calls_ = {};
        current_size_ = {};
        peak_size_ = {};
        allocated_size_ = {};
        deallocated_size_ = {};
    }

private:
    std::atomic<size_t> new_calls_{};
    std::atomic<size_t> delete_calls_{};
    std::atomic<size_t> current_size_{};
    std::atomic<size_t> peak_size_{};
    std::atomic<size_t> allocated_size_{};
    std::atomic<size_t> deallocated_size_{};

    Statistics() = default;
};
} // namespace mb::alloc_statistics

#endif
