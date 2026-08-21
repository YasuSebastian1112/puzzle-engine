// ========================================================================= //
// Puzzle Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ========================================================================= //

#pragma once
#include <iostream>

#ifndef _FORCE_INLINE_
    #if defined(_MSC_VER)
        #define _FORCE_INLINE_ __forceinline
    #elif defined(__GNUC__) || defined(__clang__)
        #define _FORCE_INLINE_ inline __attribute__((always_inline))
    #else
        #define _FORCE_INLINE_ inline
    #endif
#endif


void core_init();