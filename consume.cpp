/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <iostream>
#include <thread>
#include "consume.h"

#define CHECK_EQ(GOT, EXPECT) do {                                      \
        auto got = (GOT);                                               \
        auto expect = (EXPECT);                                         \
        if (UNLIKELY(got != expect)) {                                  \
            std::cerr << "Got " << got << " expected " << expect        \
                      << " for " << #GOT << " and " << #EXPECT << '\n'; \
            abort();                                                    \
        }                                                               \
    } while (false)

template<typename T>
NEVER_INLINE dependent<T> test(const std::atomic<T> &location) {
    COMPILER_FENCE();
    dependent<T> loaded = consume_load(location);
    COMPILER_FENCE();
    // Reach into the implementation to check the value is zero. This is
    // important when creating chains because the offset provided by the
    // dependency must be zero.
    CHECK_EQ(bit_cast<unsigned>(loaded.dependency), 0u);
    return loaded;
}

template<typename T>
NEVER_INLINE dependent_ptr<T> test(const std::atomic<T*> &location) {
    COMPILER_FENCE();
    dependent_ptr<T> loaded = consume_load(location);
    COMPILER_FENCE();
    return loaded;
}

int main() {
    {
        std::atomic<uint8_t> value = 42;
        auto consumed = test(value);
        CHECK_EQ(consumed.value, 42u);
    }

    {
        std::atomic<uint16_t> value = 42;
        auto consumed = test(value);
        CHECK_EQ(consumed.value, 42u);
    }

    {
        std::atomic<uint32_t> value = 42;
        auto consumed = test(value);
        CHECK_EQ(consumed.value, 42u);
    }

    {
        std::atomic<uint64_t> value = 42;
        auto consumed = test(value);
        CHECK_EQ(consumed.value, 42u);
    }

    {
        std::atomic<float> value = 42.f;
        auto consumed = test(value);
        CHECK_EQ(consumed.value, 42.f);
    }

    {
        std::atomic<double> value = 42.;
        auto consumed = test(value);
        CHECK_EQ(consumed.value, 42.);
    }

    {
        const char* hello = "hello";
        std::atomic<const char**> ptr = &hello;
        auto consumed = test(ptr);
        CHECK_EQ((*consumed).value, hello);
    }

    {
        std::atomic<int(*)()> ptr = &main;
        auto consumed = test(ptr);
        CHECK_EQ(consumed.value(), &main);
    }

    {
        std::atomic<bool> ready = false;
        constexpr size_t num = 1024;
        uint32_t* vec = new uint32_t[num];
        std::thread t([&] () {
                for (size_t i = 0; i != num; ++i)
                    vec[i] = i * 2;
                ready.store(true, std::memory_order_release);
            });
        do {
            auto consumed = test(ready);
            if (consumed.value) {
                dependent_ptr<uint32_t> dependent_vec(vec, consumed.dependency);
                for (size_t i = 0; i != num; ++i)
                    CHECK_EQ(dependent_vec[i].value, i * 2);
                break;
            }
        } while (true);
        t.join();
        delete[] vec;
    }

    return 0;
}
