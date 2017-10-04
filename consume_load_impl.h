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

#ifndef consume_load_impl_h
#define consume_load_impl_h

template<typename T>
inline dependent_ptr<T> consume_load(const std::atomic<T*>& atom)
{
    return dependent_ptr<T>(atom.load(std::memory_order_relaxed));
}

template<typename T>
inline dependent<T> consume_load(const std::atomic<T>& atom)
{
    return dependent<T>(atom.load(std::memory_order_relaxed));
}

template<typename T>
inline dependent_ptr<T> consume_load(dependent_ptr<T*> dep)
{
    static_assert(sizeof(dep) == sizeof(std::atomic<T**>), "The cast below relies on this fact");
    std::atomic<T**> *atom = reinterpret_cast<std::atomic<T**>*>(&dep);
    return dependent_ptr<T>(atom->load(std::memory_order_relaxed));
}

template<typename T>
inline dependent<T> consume_load(dependent_ptr<T> dep)
{
    static_assert(sizeof(dep) == sizeof(std::atomic<T*>), "The cast below relies on this fact");
    std::atomic<T*> *atom = reinterpret_cast<std::atomic<T*>*>(&dep);
    return dependent<T>(atom->load(std::memory_order_relaxed));
}

template<typename T>
inline dependent_ptr<T> consume_load(T** loc, dependency dep)
{
    return consume_load(dependent_ptr<T*>(loc, dep));
}

template<typename T>
inline dependent<T> consume_load(T* loc, dependency dep)
{
    return consume_load(dependent_ptr<T>(loc, dep));
}

#endif
