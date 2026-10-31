// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#pragma once

#include "object.h"
#include <atomic>

class RefCounted : public Object {
    CLASS_TYPE(RefCounted, Object);

private:
    mutable std::atomic<uint32_t> ref_count{0};

public:
    RefCounted() = default;
    virtual ~RefCounted() = default;

    bool reference() const {
        ref_count.fetch_add(1, std::memory_order_relaxed);
        return true;
    }

    bool unreference() const {
        if (ref_count.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            delete this;
            return true;
        }
        return false;
    }

    uint32_t get_reference_count() const {
        return ref_count.load(std::memory_order_relaxed);
    }
};

// Smart pointer intrusivo equivalente a Ref<T> de Godot
template <typename T>
class Ref {
private:
    T* reference_ptr = nullptr;

    void ref(T* p_ptr) {
        if (p_ptr == reference_ptr) return;
        unref();
        reference_ptr = p_ptr;
        if (reference_ptr) {
            reference_ptr->reference();
        }
    }

    void unref() {
        if (reference_ptr) {
            reference_ptr->unreference();
            reference_ptr = nullptr;
        }
    }

public:
    Ref() = default;

    Ref(T* p_ptr) {
        ref(p_ptr);
    }

    Ref(const Ref& p_other) {
        ref(p_other.reference_ptr);
    }

    template <typename U>
    Ref(const Ref<U>& p_other) {
        ref(p_other.ptr());
    }

    Ref(Ref&& p_other) noexcept : reference_ptr(p_other.reference_ptr) {
        p_other.reference_ptr = nullptr;
    }

    ~Ref() {
        unref();
    }

    Ref& operator=(const Ref& p_other) {
        ref(p_other.reference_ptr);
        return *this;
    }

    Ref& operator=(Ref&& p_other) noexcept {
        if (this != &p_other) {
            unref();
            reference_ptr = p_other.reference_ptr;
            p_other.reference_ptr = nullptr;
        }
        return *this;
    }

    Ref& operator=(T* p_ptr) {
        ref(p_ptr);
        return *this;
    }

    T* operator->() const { return reference_ptr; }
    T& operator*() const { return *reference_ptr; }
    T* ptr() const { return reference_ptr; }
    bool is_valid() const { return reference_ptr != nullptr; }
    bool is_null() const { return reference_ptr == nullptr; }

    bool operator==(const Ref& p_other) const { return reference_ptr == p_other.reference_ptr; }
    bool operator!=(const Ref& p_other) const { return reference_ptr != p_other.reference_ptr; }
    bool operator==(const T* p_ptr) const { return reference_ptr == p_ptr; }
    bool operator!=(const T* p_ptr) const { return reference_ptr != p_ptr; }
};