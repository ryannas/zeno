#include <cstdio>
#include "impl_cuda.h"
//#include "impl_host.h"
#include "Vector.h"
#include "HashMap.h"

using namespace fdb;

template <class T>
struct HashGrid {
    struct u64_3x21 {
        uint64_t value;

        u64_3x21() : value((uint64_t)-1l) {}

        FDB_CONSTEXPR u64_3x21(vec3S const &a) {
            uint64_t x = a[0] & 0x1ffffful;
            uint64_t y = a[1] & 0x1ffffful;
            uint64_t z = a[2] & 0x1ffffful;
            value = x | y << 21 | z << 42;
        }

        FDB_CONSTEXPR bool has_value() const {
            return (int64_t)value >= 0l;
        }

        FDB_CONSTEXPR operator vec3S() const {
            uint64_t x = value & 0x1ffffful;
            uint64_t y = (value >> 21) & 0x1ffffful;
            uint64_t z = (value >> 42) & 0x1ffffful;
            return vec3S(x, y, z);
        }
    };

    HashMap<u64_3x21, T> m_table;

    struct View {
        HashMap<u64_3x21, T> m_view;

        template <class Kernel>
        inline void parallel_foreach(Kernel kernel, ParallelConfig cfg = {512, 2}) const {
            m_view.parallel_foreach([=] FDB_DEVICE (u64_3x21 key, T &value) {
                vec3S coord(key);
                kernel(std::as_const(coord), value);
            }, cfg);
        }

        View(HashGrid const &parent)
            : m_view(parent.view())
        {}
    };

    inline View view() const {
        return *this;
    }
};

int main() {
#if 1
    HashMap<int, int> a;
    a.reserve(4099);
    {
        auto av = a.view();
        parallel_for(4097, [=] FDB_DEVICE (int i) {
            i = (114514 * i) + 31415;
            av.emplace(i, i * 2 + 1);
        });

        av.parallel_foreach([=] FDB_DEVICE (int k, int &v) {
            if (k * 2 + 1 != v) {
                printf("error: %d != %d\n", k * 2 + 1, v);
            } else {
                printf("ok\n");
            }
        });
    }

#else
    Vector<int> a;
    a.resize(5, 40);
    {
        auto av = a.view();
        parallel_for(a.size(), [=] FDB_DEVICE (size_t i) {
            printf("- %ld %d\n", i, av[i]);
            av[i] = 42;
        });
    }
    a.resize(8, 4);
    {
        auto av = a.view();
        parallel_for(a.size(), [=] FDB_DEVICE (size_t i) {
            printf("+ %ld %d\n", i, av[i]);
        });
    }

#endif

    synchronize();
    return 0;
}
