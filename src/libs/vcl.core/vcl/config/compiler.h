/*
 * This file is part of the Visual Computing Library (VCL) release under the
 * MIT license.
 *
 * Copyright (c) 2014 Basil Fierz
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */ 
#pragma once

#include <type_traits>

#include <vcl/config/config.h>

// Check library configuration
#if defined __ICC || defined __ICL
#	define VCL_COMPILER_ICC
#   if (__INTEL_COMPILER < 1700)
#       warning "Minimum supported version is ICC 17. Good luck."
#   endif
#elif defined _MSC_VER && !defined __clang__
#	define VCL_COMPILER_MSVC
// Microsoft compiler versions
// MSVC++ 10.0 _MSC_VER == 1600 (Visual Studio 2010)
// MSVC++ 11.0 _MSC_VER == 1700 (Visual Studio 2012)
// MSVC++ 12.0 _MSC_VER == 1800 (Visual Studio 2013)
// MSVC++ 14.0 _MSC_VER == 1900 (Visual Studio 2015)
// MSVC++ 14.1 _MSC_VER == 1910 (Visual Studio 2017)
#   if (_MSC_VER < 1900)
#       warning "Minimum supported version is MSVC 2015. Good luck."
#   endif
#elif defined __clang__
#	define VCL_COMPILER_CLANG
#   if (__clang_major__ < 3) || (__clang_major__ == 3 && __clang_minor__ < 5) 
#       warning "Minimum supported version is Clang 3.5. Good luck."
#   endif
#elif defined __GNUC__
#	define VCL_COMPILER_GNU
#   if (__GNUC__ < 5)
#       warning "Minimum supported version is GCC 5. Good luck."
#   endif
#endif

// Identify system ABI
#if defined VCL_COMPILER_ICC \
 || defined VCL_COMPILER_MSVC \
 || defined VCL_COMPILER_CLANG \
 || defined VCL_COMPILER_GNU
#	if defined(_WIN32)
#		define VCL_ABI_WINAPI
#		if defined(_WIN64)
#			define VCL_ABI_WIN64
#		else
#			define VCL_ABI_WIN32
#		endif // _WIN64
#	endif // _WIN32

#	if defined(__unix) && __unix == 1
#		define VCL_ABI_POSIX
#	endif // __unix
#endif

// Identify CPU instruction set
#if defined VCL_COMPILER_ICC

// The Intel compiler only supports Intel64 and x86 instruction sets
#	if defined _M_X64 || defined __x86_64 || defined __x86_64__
#		define VCL_ARCH_X64
#	else
#		define VCL_ARCH_X86
#	endif

#elif defined VCL_COMPILER_MSVC
#	if defined _M_IX86
#		define VCL_ARCH_X86
#	endif

#	if defined _M_X64
#		define VCL_ARCH_X64
#	endif

#	if defined _M_ARM
#		define VCL_ARCH_ARM
#	endif

#elif defined VCL_COMPILER_GNU || defined VCL_COMPILER_CLANG
#	if defined __i686__
#		define VCL_ARCH_X86
#	endif

#	if defined __x86_64
#		define VCL_ARCH_X64
#	endif
#endif

// Identify supported compiler features
#if defined VCL_COMPILER_MSVC

// Force inline
#	define VCL_STRONG_INLINE __forceinline

// Enter the debugger
#	define VCL_DEBUG_BREAK __debugbreak()

#	define VCL_CALLBACK __stdcall

#elif defined (VCL_COMPILER_GNU) || defined (VCL_COMPILER_CLANG)

// Inlining
#	define VCL_STRONG_INLINE inline

#	define VCL_DEBUG_BREAK __builtin_trap()

#	define VCL_CALLBACK __attribute__ ((__stdcall__))

#	if defined(_MSC_VER) && defined(VCL_COMPILER_CLANG)
#		define __ENABLE_MSVC_VECTOR_TYPES_IMP_DETAILS
#	endif // defined(_MSC_VER) && defined(VCL_COMPILER_CLANG)

// Add missing definition for max_align_t for compatibility with older clang version (3.4, 3.5)
#if defined(VCL_COMPILER_CLANG)
#   if __STDC_VERSION__ >= 201112L || __cplusplus >= 201103L
#       if !defined(__CLANG_MAX_ALIGN_T_DEFINED) || __has_feature(modules)
            typedef struct {
            long long __clang_max_align_nonce1
                __attribute__((__aligned__(__alignof__(long long))));
            long double __clang_max_align_nonce2
                __attribute__((__aligned__(__alignof__(long double))));
            } max_align_t;
#       endif
#   endif
#endif

#elif defined (VCL_COMPILER_ICC)

#	define VCL_STRONG_INLINE __forceinline
#	define VCL_DEBUG_BREAK
#	define VCL_CALLBACK

#else // No compiler found
#	define VCL_STRONG_INLINE inline
#	define VCL_DEBUG_BREAK
#	define VCL_CALLBACK
#endif


////////////////////////////////////////////////////////////////////////////////
// Evaluate compiler feature support
////////////////////////////////////////////////////////////////////////////////

// alignas/alignof
#if defined (VCL_COMPILER_MSVC)
#	if (_MSC_VER <= 1800)
#		include <xkeycheck.h>
#		if defined alignof
#			undef alignof
#		endif
//#		define alignof(x) __alignof(decltype(*static_cast<std::remove_reference<std::remove_pointer<(x)>::type>::type*>(0)))
#		define alignof(x) __alignof(x)

#		define alignas(x) __declspec(align(x))
#	endif // _MSC_VER
#else defined (VCL_COMPILER_GNU) || defined (VCL_COMPILER_CLANG)
#endif

// constexpr
#if defined (VCL_COMPILER_MSVC)
#	if (_MSC_VER < 1900)
#		define VCL_CPP_CONSTEXPR_11 0
#		define VCL_CPP_CONSTEXPR_14 0
#		define VCL_CPP_CONSTEXPR_11
#		define VCL_CPP_CONSTEXPR_14
#	elif (_MSC_VER <= 1900)
#		define VCL_HAS_CPP_CONSTEXPR_11 1
#		define VCL_HAS_CPP_CONSTEXPR_14 0
#		define VCL_CPP_CONSTEXPR_11 constexpr
#		define VCL_CPP_CONSTEXPR_14
#	elif (_MSC_VER <= 1910)
#		define VCL_HAS_CPP_CONSTEXPR_11 1
#		define VCL_HAS_CPP_CONSTEXPR_14 1
#		define VCL_CPP_CONSTEXPR_11 constexpr
#		define VCL_CPP_CONSTEXPR_14 constexpr
#	endif
#elif defined (VCL_COMPILER_GNU) || defined (VCL_COMPILER_CLANG) || defined (VCL_COMPILER_ICC)
#	define VCL_HAS_CPP_CONSTEXPR_11 1
#	define VCL_HAS_CPP_CONSTEXPR_14 1
#	define VCL_CPP_CONSTEXPR_11 constexpr
#	define VCL_CPP_CONSTEXPR_14 constexpr
#else
#	define VCL_HAS_CPP_CONSTEXPR_11 0
#	define VCL_HAS_CPP_CONSTEXPR_14	0
#	define VCL_CPP_CONSTEXPR_11
#	define VCL_CPP_CONSTEXPR_14
#endif

// noexcept
#if defined (VCL_COMPILER_MSVC)
#	if (_MSC_VER <= 1800)
#		define noexcept _NOEXCEPT
#		define VCL_NOEXCEPT_PARAM(param)
#	else
#		define VCL_NOEXCEPT_PARAM(param) noexcept(param)
#	endif // _MSC_VER
#elif defined (VCL_COMPILER_GNU) || defined (VCL_COMPILER_CLANG) || defined (VCL_COMPILER_ICC)
#	define VCL_NOEXCEPT_PARAM(param) noexcept(param)
#else
#	define VCL_NOEXCEPT_PARAM(param)
#endif

// thread_local
#if defined (VCL_COMPILER_MSVC)
#	if (_MSC_VER <= 1900)
#		define thread_local __declspec(thread)
#	endif // _MSC_VER <= 1900
#elif defined (VCL_COMPILER_GNU)
#	if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 8)
#		define thread_local __thread
#	endif
#elif defined (VCL_COMPILER_CLANG)
#	if __clang_major__ < 3 || (__clang_major__ == 3 && __clang_minor__ < 3)
#		define thread_local __thread
#	endif
#endif

////////////////////////////////////////////////////////////////////////////////
// Evaluate standard library support
////////////////////////////////////////////////////////////////////////////////

// chrono
#if defined (VCL_COMPILER_MSVC)
#	if _MSC_VER >= 1700
#		define VCL_STL_CHRONO
#	endif
#else defined (VCL_COMPILER_GNU) || defined (VCL_COMPILER_CLANG)
#	if __cplusplus >= 201103L
#		define VCL_STL_CHRONO
#	endif
#endif

// any
#if defined (VCL_COMPILER_MSVC)
#	if (_MSC_VER >= 1910 && _MSVC_LANG > 201402)
#		define VCL_STL_ANY
#	endif
#else defined (VCL_COMPILER_GNU) || defined (VCL_COMPILER_CLANG)
#endif

////////////////////////////////////////////////////////////////////////////////
// Configure SIMD
////////////////////////////////////////////////////////////////////////////////
#if (defined(VCL_ARCH_X86) || defined(VCL_ARCH_X64))

#	ifdef VCL_VECTORIZE_AVX2
#		ifndef VCL_VECTORIZE_AVX
#			define VCL_VECTORIZE_AVX
#		endif
#	endif 
#	ifdef VCL_VECTORIZE_AVX
#		ifndef VCL_VECTORIZE_SSE4_2
#			define VCL_VECTORIZE_SSE4_2
#		endif
#	endif

#	ifdef VCL_VECTORIZE_SSE4_2
#		ifndef VCL_VECTORIZE_SSE4_1
#			define VCL_VECTORIZE_SSE4_1
#		endif
#	endif
#	ifdef VCL_VECTORIZE_SSE4_1
#		ifndef VCL_VECTORIZE_SSSE3
#			define VCL_VECTORIZE_SSSE3
#		endif
#	endif
#	ifdef VCL_VECTORIZE_SSSE3
#		ifndef VCL_VECTORIZE_SSE3
#			define VCL_VECTORIZE_SSE3
#		endif
#	endif
#	ifdef VCL_VECTORIZE_SSE3
#		ifndef VCL_VECTORIZE_SSE2
#			define VCL_VECTORIZE_SSE2
#		endif
#	endif
#	ifdef VCL_VECTORIZE_SSE2
#		ifndef VCL_VECTORIZE_SSE
#			define VCL_VECTORIZE_SSE
#		endif
#	endif

#	if defined VCL_VECTORIZE_AVX
		extern "C"
		{
#		ifdef VCL_VECTORIZE_AVX2
#			include <immintrin.h>
#		else
#			include <immintrin.h>
#		endif
		}
#	elif defined(VCL_VECTORIZE_SSE)
		extern "C"
		{
#		ifdef VCL_VECTORIZE_SSE4_2
#			include <nmmintrin.h>
#		elif defined VCL_VECTORIZE_SSE4_1
#			include <smmintrin.h>
#		elif defined VCL_VECTORIZE_SSSE3
#			include <tmmintrin.h>
#		elif defined VCL_VECTORIZE_SSE3
#			include <pmmintrin.h>
#		elif defined VCL_VECTORIZE_SSE2
#			include <emmintrin.h>
#			include <xmmintrin.h>
#			include <mmintrin.h>
#		endif
		}
#	endif // defined(VCL_VECTORIZE_SSE)

#elif defined VCL_ARCH_ARM && defined VCL_VECTORIZE_NEON
#	include <arm_neon.h>
#endif

// Implement missing standard function
#if defined (VCL_COMPILER_MSVC)

// Support for fmin/fmax with low overhead
#	if (_MSC_VER < 1800)
namespace std
{
#		if (defined(VCL_VECTORIZE_AVX) || defined(VCL_VECTORIZE_SSE))
	inline float fmin(float x, float y)
	{
		float z;
		_mm_store_ss(&z, _mm_min_ss(_mm_set_ss(x), _mm_set_ss(y)));
		return z;
	}
	inline double fmin(double x, double y)
	{
		double z;
		_mm_store_sd(&z, _mm_min_sd(_mm_set_sd(x), _mm_set_sd(y)));
		return z;
	}
	inline float fmax(float x, float y)
	{
		float z;
		_mm_store_ss(&z, _mm_max_ss(_mm_set_ss(x), _mm_set_ss(y)));
		return z;
	}
	inline double fmax(double x, double y)
	{
		double z;
		_mm_store_sd(&z, _mm_max_sd(_mm_set_sd(x), _mm_set_sd(y)));
		return z;
	}
#		endif
}
#	endif // _MSC_VER < 1800
#endif
