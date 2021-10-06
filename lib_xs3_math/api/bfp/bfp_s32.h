// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#pragma once

#include "xs3_math_types.h"



/*
    Saturation Logic:

    Where specified, the VPU-enhanced arithmetic functions apply symmetric saturation logic, which is applied
    where results would otherwise overflow.

    The resulting integer range for N-bit saturation is   -(2^(N-1))+1  to  (2^(N-1))-1.

    8-bit:      (-127, 127)
    16-bit:     (-65535, 65535)
    32-bit:     (-2147483647, 2147483647)
*/

/**
 * @page page_bfp_s32_h  bfp_s32.h
 * 
 * This header contains functions implementing arithmetic operations on 32-bit block floating-point
 * vectors.
 * 
 * Functions for initializing BFP vectors can be found in @ref page_bfp_init_h.
 * 
 * @note This header is included automatically through `bfp_math.h`.
 * 
 * @see  `bfp_s32_t`
 * 
 * @ingroup xs3_math_header_file
 */



/**
 * @brief Set all elements of a 32-bit BFP vector to a specified value.
 * 
 * The exponent of `a` is set to `exp`, and each element's mantissa is set to `b`.
 * 
 * After performing this operation, all elements will represent the same value @math{b \cdot 2^{exp}}.
 * 
 * `a` must have been initialized (see bfp_s32_init()).
 * 
 * @param[out] a         BFP vector to update
 * @param[in]  b         New value each mantissa is set to
 * @param[in]  exp       New exponent for the BFP vector
 * 
 * @ingroup bfp32_func
 */
C_API
void bfp_s32_set(
    bfp_s32_t* a,
    const int32_t b,
    const exponent_t exp);


/**
 * @brief Modify a 32-bit BFP vector to use a specified exponent.
 * 
 * This function forces BFP vector @vector{A} to use a specified exponent. The mantissa vector
 * @vector{a} will be bit-shifted left or right to compensate for the changed exponent.
 * 
 * This function can be used, for example, before calling a fixed-point arithmetic function to 
 * ensure the underlying mantissa vector has the needed Q-format. As another example, this may be
 * useful when communicating with peripheral devices (e.g. via I2S) that require sample data to
 * be in a specified format.
 * 
 * Note that this sets the _current_ encoding, and does not _fix_ the exponent permanently (i.e.
 * subsequent operations may change the exponent as usual).
 * 
 * If the required fixed-point Q-format is `QX.Y`, where `Y` is the number of fractional bits in the
 * resulting mantissas, then the associated exponent (and value for parameter `exp`) is `-Y`.
 * 
 * `a` points to input BFP vector @vector{A}, with mantissa vector @vector{a} and exponent
 * @math{a\_exp}. `a` is updated in place to produce resulting BFP vector @math{\tilde{A}} with
 * mantissa vector @math{\tilde{a}} and exponent @math{\tilde{a}\_exp}.
 * 
 * `exp` is @math{\tilde{a}\_exp}, the required exponent. @math{\Delta{}p = \tilde{a}\_exp - a\_exp}
 * is the required change in exponent.
 * 
 * If @math{\Delta{}p = 0}, the BFP vector is left unmodified.
 * 
 * If @math{\Delta{}p > 0}, the required exponent is larger than the current exponent and an
 * arithmetic right-shift of @math{\Delta{}p} bits is applied to the mantissas @vector{a}. When
 * applying a right-shift, precision may be lost by discarding the @math{\Delta{}p} least
 * significant bits.
 * 
 * If @math{\Delta{}p < 0}, the required exponent is smaller than the current exponent and a
 * left-shift of @math{\Delta{}p} bits is applied to the mantissas @vector{a}. When left-shifting,
 * saturation logic will be applied such that any element that can't be represented exactly with
 * the new exponent will saturate to the 32-bit saturation bounds.
 * 
 * The exponent and headroom of `a` are updated by this function.
 * 
 * @operation{
 * &    \Delta{}p = \tilde{a}\_exp - a\_exp
 * &    \tilde{a_k} \leftarrow sat_{32}( a_k \cdot 2^{-\Delta{}p} )   \\
 * &        \qquad\text{for } k \in 0\ ...\ (N-1)                     \\
 * &        \qquad\text{where } N \text{ is the length of } \bar{A} \text{ (in elements) }
 * }
 * 
 * @param[inout]  a     Input BFP vector @vector{A} / Output BFP vector @math{\tilde{A}}
 * @param[in]     exp   The required exponent, @math{\tilde{a}\_exp}
 * 
 * @ingroup bfp32_func
 */
C_API
void bfp_s32_use_exponent(
    bfp_s32_t* a,
    const exponent_t exp);


/** 
 * @brief Get the headroom of a 32-bit BFP vector.
 * 
 * The headroom of a vector is the number of bits its elements can be left-shifted without losing any information. It 
 * conveys information about the range of values that vector may contain, which is useful for determining how best to 
 * preserve precision in potentially lossy block floating-point operations.
 * 
 * In a BFP context, headroom applies to mantissas only, not exponents.
 * 
 * In particular, if the 32-bit mantissa vector @vector{x} has @math{N} bits of headroom, then for any element 
 * @math{x_k} of @vector{x}
 * 
 * @math{-2^{31-N} \le x_k \lt 2^{31-N}}
 * 
 * And for any element @math{X_k = x_k \cdot 2^{x\_exp}} of a complex BFP vector @vector{X}
 * 
 * @math{-2^{31 + x\_exp - N} \le X_k \lt 2^{31 + x\_exp - N} }
 * 
 * This function determines the headroom of `b`, updates `b->hr` with that value, and then returns `b->hr`.
 *
 * @param   b         BFP vector to get the headroom of
 * 
 * @returns    Headroom of BFP vector `b` 
 * 
 * @ingroup bfp32_func
 */
C_API
headroom_t bfp_s32_headroom(
    bfp_s32_t* b);


/** 
 * @brief Apply a left-shift to the mantissas of a 32-bit BFP vector.
 * 
 * Each mantissa of input BFP vector @vector{B} is left-shifted `b_shl` bits and stored in the corresponding element of
 * output BFP vector @vector{A}.
 * 
 * This operation can be used to add or remove headroom from a BFP vector.
 * 
 * `b_shl` is the number of bits that each mantissa will be left-shifted. This shift is signed and arithmetic, so 
 * negative values for `b_shl` will right-shift the mantissas.
 * 
 * `a` and `b` must have been initialized (see bfp_s32_init()), and must be the same length.
 * 
 * This operation can be performed safely in-place on `b`.
 * 
 * Note that this operation bypasses the logic protecting the caller from saturation or underflows. Output values 
 * saturate to the symmetric 32-bit range (@math{-2^{31} \lt \lt 2^{31}}). To avoid saturation, `b_shl` should be no
 * greater than the headroom of `b` (`b->hr`).
 * 
 * @operation{
 * &     a_k \leftarrow sat_{32}( \lfloor b_k \cdot 2^{b\_shl} \rfloor )     \\
 * &         \qquad\text{for } k \in 0\ ...\ (N-1)                           \\
 * &         \qquad\text{where } N \text{ is the length of } \bar{B}         \\
 * &         \qquad\text{  and } b_k \text{ and } a_k \text{ are the } k\text{th mantissas from } 
 *               \bar{B}\text{ and } \bar{A}\text{ respectively}
 * }
 * 
 * @param[out] a        Output BFP vector @vector{A}
 * @param[in]  b        Input BFP vector @vector{B}
 * @param[in]  b_shl    Signed arithmetic left-shift to be applied to mantissas of @vector{B}.
 * 
 * @ingroup bfp32_func
 */
C_API
void bfp_s32_shl(
    bfp_s32_t* a,
    const bfp_s32_t* b,
    const left_shift_t b_shl);


/** 
 * @brief Add two 32-bit BFP vectors together.
 * 
 * Add together two input BFP vectors @vector{B} and @vector{C} and store the result in BFP vector @vector{A}. 
 * 
 * `a`, `b` and `c` must have been initialized (see bfp_s32_init()), and must be the same length.
 * 
 * This operation can be performed safely in-place on `b` or `c`.
 * 
 * @operation{
 *      \bar{A} \leftarrow \bar{B} + \bar{C}  
 * }
 * 
 * @param[out] a     Output BFP vector @vector{A}
 * @param[in]  b     Input BFP vector @vector{B}
 * @param[in]  c     Input BFP vector @vector{C}
 * 
 * @ingroup bfp32_func
 */
C_API
void bfp_s32_add(
    bfp_s32_t* a, 
    const bfp_s32_t* b, 
    const bfp_s32_t* c);
    

/**
 * @brief Add a scalar to a 32-bit BFP vector.
 * 
 * Add a real scalar @math{c} to input BFP vector @vector{B} and store the result in BFP vector
 * @vector{A}. 
 * 
 * `a`, and `b` must have been initialized (see bfp_s32_init()), and must be the same length.
 * 
 * This operation can be performed safely in-place on `b`.
 * 
 * @operation{
 *      \bar{A} \leftarrow \bar{B} + c  
 * }
 * 
 * @param[out] a     Output BFP vector @vector{A}
 * @param[in]  b     Input BFP vector @vector{B}
 * @param[in]  c     Input scalar @math{c}
 * 
 * @ingroup bfp32_func
 */
C_API
void bfp_s32_add_scalar(
    bfp_s32_t* a, 
    const bfp_s32_t* b, 
    const float_s32_t c);


/** 
 * @brief Subtract one 32-bit BFP vector from another.
 * 
 * Subtract input BFP vector @vector{C} from input BFP vector @vector{C} and store the result
 * in BFP vector @vector{A}. 
 * 
 * `a`, `b` and `c` must have been initialized (see bfp_s32_init()), and must be the same length.
 * 
 * This operation can be performed safely in-place on `b` or `c`.
 * 
 * @operation{
 *      \bar{A} \leftarrow \bar{B} - \bar{C}  
 * }
 * 
 * @param[out] a     Output BFP vector @vector{A}
 * @param[in]  b     Input BFP vector @vector{B}
 * @param[in]  c     Input BFP vector @vector{C}
 * 
 * @ingroup bfp32_func
 */
C_API
void bfp_s32_sub(
    bfp_s32_t* a, 
    const bfp_s32_t* b, 
    const bfp_s32_t* c);


/**
 * @brief Multiply one 32-bit BFP vector by another element-wise.
 * 
 * Multiply each element of input BFP vector @vector{B} by the corresponding element of input BFP vector @vector{C} 
 * and store the results in output BFP vector @vector{A}.
 * 
 * `a`, `b` and `c` must have been initialized (see bfp_s32_init()), and must be the same length.
 * 
 * This operation can be performed safely in-place on `b` or `c`.
 * 
 * @operation{ 
 * &     A_k \leftarrow B_k \cdot C_k                             \\
 * &         \qquad\text{for } k \in 0\ ...\ (N-1)                \\
 * &         \qquad\text{where } N \text{ is the length of } \bar{B}\text{ and }\bar{C}
 * }
 * 
 * @param a     Output BFP vector @vector{A}
 * @param b     Input BFP vector @vector{B}
 * @param c     Input BFP vector @vector{C}
 * 
 * @ingroup bfp32_func
 */
C_API
void bfp_s32_mul(
    bfp_s32_t* a, 
    const bfp_s32_t* b, 
    const bfp_s32_t* c);


/**
 * @brief Multiply one 32-bit BFP vector by another element-wise and add the result to a third
 * vector.
 * 
 * @operation{
 * &    A_k \leftarrow A_k + B_k \cdot C_k              \\
 * &        \qquad\text{for } k \in 0\ ...\ (N-1)       \\
 * &        \qquad\text{where } N \text{ is the length of } \bar{B}\text{ and }\bar{C}
 * }
 * 
 * @param[inout]  acc   Input/Output accumulator BFP vector @vector{A}
 * @param[in]     b     Input BFP vector @vector{B}
 * @param[in]     c     Input BFP vector @vector{C}
 * 
 * @ingroup bfp32_func
 */
C_API
void bfp_s32_macc(
    bfp_s32_t* acc, 
    const bfp_s32_t* b, 
    const bfp_s32_t* c);


/**
 * @brief Multiply one 32-bit BFP vector by another element-wise and subtract the result from a
 * third vector.
 * 
 * @operation{
 * &    A_k \leftarrow A_k - B_k \cdot C_k              \\
 * &        \qquad\text{for } k \in 0\ ...\ (N-1)       \\
 * &        \qquad\text{where } N \text{ is the length of } \bar{B}\text{ and }\bar{C}
 * }
 * 
 * @param[inout]  acc   Input/Output accumulator BFP vector @vector{A}
 * @param[in]     b     Input BFP vector @vector{B}
 * @param[in]     c     Input BFP vector @vector{C}
 * 
 * @ingroup bfp32_func
 */
C_API
void bfp_s32_nmacc(
    bfp_s32_t* acc, 
    const bfp_s32_t* b, 
    const bfp_s32_t* c);


/** 
 * @brief Multiply a 32-bit BFP vector by a scalar.
 * 
 * Multiply input BFP vector @vector{B} by scalar @math{\alpha \cdot 2^{\alpha\_exp}} and store the result in output 
 * BFP vector @vector{A}.
 * 
 * `a` and `b` must have been initialized (see bfp_s32_init()), and must be the same length.
 * 
 * `alpha` represents the scalar @math{\alpha \cdot 2^{\alpha\_exp}}, where @math{\alpha} is `alpha.mant` and 
 * @math{\alpha\_exp} is `alpha.exp`.
 * 
 * This operation can be performed safely in-place on `b`.
 * 
 * @operation{
 *      \bar{A} \leftarrow \bar{B} \cdot \left(\alpha \cdot 2^{\alpha\_exp}\right)
 * }
 * 
 * @param[out] a             Output BFP vector @vector{A}
 * @param[in]  b             Input BFP vector @vector{B}
 * @param[in]  alpha        Scalar by which @vector{B} is multiplied
 * 
 * @ingroup bfp32_func
 */
C_API
void bfp_s32_scale(
    bfp_s32_t* a, 
    const bfp_s32_t* b,
    const float_s32_t alpha);


/** 
 * @brief Get the absolute values of elements of a 32-bit BFP vector. 
 * 
 * Compute the absolute value of each element @math{B_k} of input BFP vector @vector{B} and store the results in output 
 * BFP vector @vector{A}.
 * 
 * `a` and `b` must have been initialized (see bfp_s32_init()), and must be the same length.
 * 
 * This operation can be performed safely in-place on `b`.
 * 
 * @operation{ 
 * &     A_k \leftarrow \left| B_k \right|               \\
 * &         \qquad\text{for } k \in 0\ ...\ (N-1)       \\
 * &         \qquad\text{where } N \text{ is the length of } \bar{B}
 * }
 * 
 * @param[out] a     Output BFP vector @vector{A}
 * @param[in]  b     Input BFP vector @vector{B}
 * 
 * @ingroup bfp32_func
 */
C_API
void bfp_s32_abs(
    bfp_s32_t* a,
    const bfp_s32_t* b);


/** 
 * @brief Sum the elements of a 32-bit BFP vector.
 * 
 * Sum the elements of input BFP vector @vector{B} to get a result @math{A = a \cdot 2^{a\_exp}}, which is returned. The
 * returned value has a 64-bit mantissa.
 * 
 * `b` must have been initialized (see bfp_s32_init()).
 * 
 * @operation{
 * &     A \leftarrow \sum_{k=0}^{N-1} \left( B_k \right)            \\
 * &         \qquad\text{where } N \text{ is the length of } \bar{B}
 * }
 * 
 * 
 * @param[in] b         Input BFP vector @vector{B}
 * 
 * @returns  @math{A}, the sum of elements of @vector{B}
 * 
 * @ingroup bfp32_func
 */
C_API
float_s64_t bfp_s32_sum(
    const bfp_s32_t* b);


/** 
 * @brief Compute the inner product of two 32-bit BFP vectors.
 * 
 * Adds together the element-wise products of input BFP vectors @vector{B} and @vector{C} for a result 
 * @math{A = a \cdot 2^{a\_exp}}, where @math{a} is the 64-bit mantissa of the result and @math{a\_exp} is its 
 * associated exponent. @math{A} is returned.
 * 
 * `b` and `c` must have been initialized (see bfp_s32_init()), and must be the same length.
 * 
 * @operation{
 * &     a \cdot 2^{a\_exp} \leftarrow \sum_{k=0}^{N-1} \left( B_k \cdot C_k \right)     \\
 * &         \qquad\text{where } N \text{ is the length of } \bar{B}\text{ and }\bar{C}
 * }
 * 
 * @param[in]  b        Input BFP vector @vector{B}
 * @param[in]  c        Input BFP vector @vector{C}
 * 
 * @returns     @math{A}, the inner product of vectors @vector{B} and @vector{C}
 * 
 * @ingroup bfp32_func
 */
C_API
float_s64_t bfp_s32_dot(
    const bfp_s32_t* b, 
    const bfp_s32_t* c);


/** 
 * Clamp the elements of a 32-bit BFP vector to a specified range.
 * 
 * Each element @math{A_k} of output BFP vector @vector{A} is set to the corresponding element @math{B_k} of input BFP
 * vector @vector{B} if it is in the range @math{ [ L \cdot 2^{bound\_exp}, U \cdot 2^{bound\_exp} ] }, otherwise it is
 * set to the nearest value inside that range.
 * 
 * `a` and `b` must have been initialized (see bfp_s32_init()), and must be the same length.
 * 
 * This operation can be performed safely in-place on `b`.
 * 
 * @operation{
 * &     A_k \leftarrow \begin{cases}
 * &         L \cdot 2^{bound\_exp}      &   B_k \lt L \cdot 2^{bound\_exp}  \\
 * &         U \cdot 2^{bound\_exp}      &   B_k \gt U \cdot 2^{bound\_exp}  \\
 * &         B_k                         &   otherwise
 * &     \end{cases}                                     \\
 * &         \qquad\text{for } k \in 0\ ...\ (N-1)       \\
 * &         \qquad\text{where } N \text{ is the length of } \bar{B}
 * }
 * 
 * @param[out] a             Output BFP vector @vector{A}
 * @param[in]  b             Input BFP vector @vector{B}
 * @param[in]  lower_bound   Mantissa of the lower clipping bound, @math{L}
 * @param[in]  upper_bound   Mantissa of the upper clipping bound, @math{U}
 * @param[in]  bound_exp     Shared exponent of the clipping bounds
 * 
 * @ingroup bfp32_func
 */
C_API
void bfp_s32_clip(
    bfp_s32_t* a, 
    const bfp_s32_t* b, 
    const int32_t lower_bound, 
    const int32_t upper_bound, 
    const int bound_exp);


/** 
 * @brief Rectify a 32-bit BFP vector.
 * 
 * Each element @math{A_k} of output BFP vector @vector{A} is set to the corresponding element @math{B_k} of input BFP
 * vector @vector{B} if it is non-negative, otherwise it is set to @math{0}.
 * 
 * `a` and `b` must have been initialized (see bfp_s32_init()), and must be the same length.
 * 
 * This operation can be performed safely in-place on `b`.
 * 
 * @operation{
 * &     A_k \leftarrow \begin{cases}
 * &         0       &   B_k \lt 0       \\
 * &         B_k     &   otherwise
 * &     \end{cases}                     \\
 * &     \qquad\text{for } k \in 0\ ...\ (N-1)       \\
 * &     \qquad\text{where } N \text{ is the length of } \bar{B}
 * }
 * 
 * @param[out] a     Output BFP vector @vector{A}
 * @param[in]  b     Input BFP vector @vector{B}
 * 
 * @ingroup bfp32_func
 */
C_API
void bfp_s32_rect(
    bfp_s32_t* a, 
    const bfp_s32_t* b);

/** 
 * @brief Convert a 32-bit BFP vector into a 16-bit BFP vector.
 * 
 * Reduces the bit-depth of each 32-bit element @math{B_k} of input BFP vector @vector{B} to 16 bits, and stores the 
 * 16-bit result in the corresponding element @math{A_k} of output BFP vector @vector{A}.
 * 
 * `a` and `b` must have been initialized (see bfp_s32_init() and bfp_s16_init()), and must be the same length.
 * 
 * As much precision as possible will be retained.
 * 
 * @operation{
 * &     A_k \overset{16-bit}{\longleftarrow} B_k        \\
 * &         \qquad\text{for } k \in 0\ ...\ (N-1)       \\
 * &         \qquad\text{where } N \text{ is the length of } \bar{B}
 * }
 * 
 * @param[out] a     Output BFP vector @vector{A}
 * @param[in]  b     Input BFP vector @vector{B}
 * 
 * @ingroup bfp32_func
 **/
C_API
void bfp_s32_to_s16(
    bfp_s16_t* a,
    const bfp_s32_t* b);


/** 
 * @brief Get the square roots of elements of a 32-bit BFP vector.
 * 
 * Computes the square root of each element @math{B_k} of input BFP vector @vector{B} and stores the results in output 
 * BFP vector @vector{A}.
 * 
 * `a` and `b` must have been initialized (see bfp_s32_init()), and must be the same length.
 * 
 * This operation can be performed safely in-place on `b`.
 * 
 * @operation{ 
 * &     A_k \leftarrow \sqrt{B_k}                       \\
 * &         \qquad\text{for } k \in 0\ ...\ (N-1)       \\
 * &         \qquad\text{where } N \text{ is the length of } \bar{B}
 * }
 * 
 * @par Notes
 * @parblock
 * * Only the `XS3_BFP_SQRT_DEPTH_S32` (see xs3_math_conf.h) most significant bits of each result are computed.
 * 
 * * This function only computes real roots. For any @math{B_k \lt 0}, the corresponding output @math{A_k} is set to 
 *   @math{0}.
 * @endparblock
 * 
 * @param[out] a     Output BFP vector @vector{A}
 * @param[in]  b     Input BFP vector @vector{B}
 * 
 * @ingroup bfp32_func
 */
C_API
void bfp_s32_sqrt(
    bfp_s32_t* a,
    const bfp_s32_t* b);


/** 
 * @brief Get the inverses of elements of a 32-bit BFP vector.
 * 
 * Computes the inverse of each element @math{B_k} of input BFP vector @vector{B} and stores the results in output 
 * BFP vector @vector{A}.
 * 
 * `a` and `b` must have been initialized (see bfp_s32_init()), and must be the same length.
 * 
 * This operation can be performed safely in-place on `b`.
 * 
 * @operation{ 
 * &     A_k \leftarrow B_k^{-1}                     \\
 * &         \qquad\text{for } k \in 0\ ...\ (N-1)   \\
 * &         \qquad\text{where } N \text{ is the length of } \bar{B}
 * }
 * 
 * @param[out] a     Output BFP vector @vector{A}
 * @param[in]  b     Input BFP vector @vector{B}
 * 
 * @ingroup bfp32_func
 */
C_API
void bfp_s32_inverse(
    bfp_s32_t* a,
    const bfp_s32_t* b);


/** 
 * @brief Sum the absolute values of elements of a 32-bit BFP vector.
 * 
 * Sum the absolute values of elements of input BFP vector @vector{B} for a result @math{A = a \cdot 2^{a\_exp}}, where
 * @math{a} is a 64-bit mantissa and @math{a\_exp} is its associated exponent. @math{A} is returned.
 * 
 * `b` must have been initialized (see bfp_s32_init()).
 * 
 * @operation{
 * &     A \leftarrow \sum_{k=0}^{N-1} \left| A_k \right|            \\
 * &         \qquad\text{where } N \text{ is the length of } \bar{B}
 * }
 * 
 * @param[in] b         Input BFP vector @vector{B}
 * 
 * @returns  @math{A}, the sum of absolute values of elements of @vector{B}
 * 
 * @ingroup bfp32_func
 */
C_API
float_s64_t bfp_s32_abs_sum(
    const bfp_s32_t* b);


/** 
 * @brief Get the mean value of a 32-bit BFP vector.
 * 
 * Computes @math{A = a \cdot 2^{a\_exp}}, the mean value of elements of input BFP vector @vector{B}, where @math{a} is
 * the 32-bit mantissa of the result, and @math{a\_exp} is its associated exponent. @math{A} is returned.
 * 
 * `b` must have been initialized (see bfp_s32_init()).
 * 
 * @operation{
 * &     A \leftarrow \frac{1}{N} \sum_{k=0}^{N-1} \left( B_k \right)     \\
 * &         \qquad\text{where } N \text{ is the length of } \bar{B}
 * }
 * 
 * @param[in]  b        Input BFP vector @vector{B}
 * 
 * @returns  @math{A}, the mean value of @vector{B}'s elements
 * 
 * @ingroup bfp32_func
 */
C_API
float_s32_t bfp_s32_mean(
    const bfp_s32_t* b);


/** 
 * @brief Get the energy (sum of squared of elements) of a 32-bit BFP vector.
 * 
 * Computes @math{A = a \cdot 2^{a\_exp}}, the sum of squares of elements of input BFP vector @vector{B}, where @math{a} 
 * is the 64-bit mantissa of the result, and @math{a\_exp} is its associated exponent. @math{A} is returned.
 * 
 * `b` must have been initialized (see bfp_s32_init()).
 * 
 * @operation{
 * &     A \leftarrow \sum_{k=0}^{N-1} \left( B_k^2 \right)   \\
 * &         \qquad\text{where } N \text{ is the length of } \bar{B}
 * }
 * 
 * @param[in]  b        Input BFP vector @vector{B}
 * 
 * @returns  @math{A}, @vector{B}'s energy
 * 
 * @ingroup bfp32_func
 */
C_API
float_s64_t bfp_s32_energy(
    const bfp_s32_t* b);


/** 
 * @brief Get the RMS value of elements of a 32-bit BFP vector.
 * 
 * Computes @math{A = a \cdot 2^{a\_exp}}, the RMS value of elements of input BFP vector @vector{B}, where @math{a} 
 * is the 32-bit mantissa of the result, and @math{a\_exp} is its associated exponent. @math{A} is returned.
 * 
 * The RMS (root-mean-square) value of a vector is the square root of the sum of the squares of the vector's elements.
 * 
 * `b` must have been initialized (see bfp_s32_init()).
 * 
 * @operation{
 * &     A \leftarrow \sqrt{\frac{1}{N}\sum_{k=0}^{N-1} \left( B_k^2 \right) }  \\
 * &         \qquad\text{where } N \text{ is the length of } \bar{B}
 * }
 * 
 * @param[in]  b        Input BFP vector @vector{B}
 * 
 * @returns  @math{A}, the RMS value of @vector{B}'s elements
 * 
 * @ingroup bfp32_func
 */
C_API
float_s32_t bfp_s32_rms(
    const bfp_s32_t* b);


/** 
 * @brief Get the maximum value of a 32-bit BFP vector.
 * 
 * Finds @math{A}, the maximum value among elements of input BFP vector @vector{B}. @math{A} is returned by this 
 * function.
 * 
 * `b` must have been initialized (see bfp_s32_init()).
 * 
 * @operation{
 * &     A \leftarrow max\left(B_0\, B_1\, ...\, B_{N-1} \right)     \\
 * &         \qquad\text{where } N \text{ is the length of } \bar{B}
 * }
 * 
 * @param[in] b     Input vector
 * 
 * @returns     @math{A}, the value of @vector{B}'s maximum element
 * 
 * @ingroup bfp32_func
 */
C_API
float_s32_t bfp_s32_max(
    const bfp_s32_t* b);


/** 
 * @brief Get the minimum value of a 32-bit BFP vector.
 * 
 * Finds @math{A}, the minimum value among elements of input BFP vector @vector{B}. @math{A} is returned by this 
 * function.
 * 
 * `b` must have been initialized (see bfp_s32_init()).
 * 
 * @operation{
 * &     A \leftarrow min\left(B_0\, B_1\, ...\, B_{N-1} \right)     \\
 * &         \qquad\text{where } N \text{ is the length of } \bar{B}
 * }
 * 
 * @param[in] b     Input vector
 * 
 * @returns     @math{A}, the value of @vector{B}'s minimum element
 * 
 * @ingroup bfp32_func
 */
C_API
float_s32_t bfp_s32_min(
    const bfp_s32_t* b);


/** 
 * @brief Get the index of the maximum value of a 32-bit BFP vector.
 * 
 * Finds @math{a}, the index of the maximum value among the elements of input BFP vector @vector{B}. @math{a} is 
 * returned by this function.
 * 
 * If `i` is the value returned, then the maximum value in @vector{B} is `ldexp(b->data[i], b->exp)`.
 * 
 * @operation{
 * &     a \leftarrow argmax_k\left(b_k\right)           \\
 * &         \qquad\text{for } k \in 0\ ...\ (N-1)       \\
 * &         \qquad\text{where } N \text{ is the length of } \bar{B}
 * }
 * 
 * @par Notes
 * @parblock
 * * If there is a tie for maximum value, the lowest tying index is returned.
 * @endparblock
 * 
 * @param[in] b     Input vector
 * 
 * @returns     @math{a}, the index of the maximum value from @vector{B}
 * 
 * @ingroup bfp32_func
 */
C_API
unsigned bfp_s32_argmax(
    const bfp_s32_t* b);


/** 
 * @brief Get the index of the minimum value of a 32-bit BFP vector.
 * 
 * Finds @math{a}, the index of the minimum value among the elements of input BFP vector @vector{B}. @math{a} is 
 * returned by this function.
 * 
 * If `i` is the value returned, then the minimum value in @vector{B} is `ldexp(b->data[i], b->exp)`.
 * 
 * @operation{
 * &     a \leftarrow argmin_k\left(b_k\right)           \\
 * &         \qquad\text{for } k \in 0\ ...\ (N-1)       \\
 * &         \qquad\text{where } N \text{ is the length of } \bar{B}
 * }
 * 
 * @par Notes
 * @parblock
 * * If there is a tie for minimum value, the lowest tying index is returned.
 * @endparblock
 * 
 * @param[in] b     Input vector
 * 
 * @returns     @math{a}, the index of the minimum value from @vector{B}
 * 
 * @ingroup bfp32_func
 */
C_API
unsigned bfp_s32_argmin(
    const bfp_s32_t* b);


/**
 * @brief Convolve a 32-bit BFP vector with a short convolution kernel ("valid" mode).
 * 
 * Input BFP vector @vector{X} is convolved with a short fixed-point convolution kernel @vector{b}
 * to produce output BFP vector @vector{Y}. In other words, this function applies the 
 * @math{K}th-order FIR filter with coefficients given by @vector{b} to the input signal @vector{X}.
 * The convolution is "valid" in the sense that no output elements are emitted where the filter taps
 * extend beyond the bounds of the input vector, resulting in an output vector @vector{Y} with fewer
 * elements.
 * 
 * The maximum filter order @math{K} supported by this function is @math{7}.
 * 
 * `y` is the output vector @vector{Y}. If input @vector{X} has @math{N} elements, and the filter
 * has @math{K} coefficients, then @vector{Y} has @math{N-2P} elements, where 
 * @math{P = \lfloor K / 2 \rfloor}. 
 * 
 * `x` is the input vector @vector{X} with length @math{N} and elements.
 * 
 * `b_q30[]` is the vector @vector{b} of filter coefficients. The coefficients of @vector{b} are
 * encoded in a Q2.30 fixed-point format. The effective value of the @math{i}th coefficient is then
 * @math{b_i \cdot 2^{-30}}.
 * 
 * `b_length` is the length @math{K} of @vector{b} in elements (i.e. the number of filter taps).
 * `b_length` must be one of @math{ \\{ 1, 3, 5, 7 \\} }. 
 * 
 * @operation{
 * &    Y_k \leftarrow  \sum_{l=0}^{K-1} (X_{(k+l)} \cdot b_l \cdot 2^{-30} )   \\
 * &         \qquad\text{ for }k\in 0\ ...\ (N-2P)                              \\
 * &         \qquad\text{ where }P = \lfloor K/2 \rfloor 
 * }
 * 
 * @param[out]  y           Output BFP vector @vector{Y}
 * @param[in]   x           Input BFP vector @vector{X}
 * @param[in]   b_q30       Convolution kernel @vector{b}
 * @param[in]   b_length    The number of elements @math{K} in @vector{b}
 * 
 * @ingroup bfp32_func
 */
C_API
void bfp_s32_convolve_valid(
  bfp_s32_t* y,
  const bfp_s32_t* x,
  const int32_t b_q30[],
  const unsigned b_length);
  

/**
 * @brief Convolve a 32-bit BFP vector with a short convolution kernel ("same" mode).
 * 
 * Input BFP vector @vector{X} is convolved with a short fixed-point convolution kernel @vector{b} to produce output BFP vector @vector{Y}.  In other words, this function applies the @math{K}th-order FIR
 * filter with coefficients given by @vector{b} to the input signal @vector{X}.  The convolution
 * mode is "same" in that the input vector is effectively padded such that the input and output
 * vectors are the same length.  The padding behavior is one of those given by @ref pad_mode_e.
 * 
 * The maximum filter order @math{K} supported by this function is @math{7}.
 * 
 * `y` and `x` are the output and input BFP vectors @vector{Y} and @vector{X} respectively.
 * 
 * `b_q30[]` is the vector @vector{b} of filter coefficients. The coefficients of @vector{b} are
 * encoded in a Q2.30 fixed-point format. The effective value of the @math{i}th coefficient is then
 * @math{b_i \cdot 2^{-30}}.
 * 
 * `b_length` is the length @math{K} of @vector{b} in elements (i.e. the number of filter taps).
 * `b_length` must be one of @math{ \\{ 1, 3, 5, 7 \\} }. 
 *  
 * `padding_mode` is one of the values from the @ref pad_mode_e enumeration. The padding mode 
 * indicates the filter input values for filter taps that have extended beyond the bounds of the
 * input vector @vector{X}. See @ref pad_mode_e for a list of supported padding modes and associated
 * behaviors.
 * 
 * @operation{
 * &    \tilde{x}_i = \begin\{cases\}
 *           \text{determined by padding mode} & i \lt 0                                  \\
 *           \text{determined by padding mode} & i \ge N                                  \\
 *           x_i & otherwise \end\{cases\}                                                \\
 * &    y_k \leftarrow  \sum_{l=0}^{K-1} (\tilde{x}_{(k+l-P)} \cdot b_l \cdot 2^{-30} )   \\
 * &         \qquad\text{ for }k\in 0\ ...\ (N-2P)                                        \\
 * &         \qquad\text{ where }P = \lfloor K/2 \rfloor 
 * }
 * 
 * @note Unlike bfp_s32_convolve_valid(), this operation _cannot_ be performed safely in-place
 * on `x`
 * 
 * @param[out]  y               Output BFP vector @vector{Y}
 * @param[in]   x               Input BFP vector @vector{X}
 * @param[in]   b_q30           Convolution kernel @vector{b}
 * @param[in]   b_length        The number of elements @math{K} in @vector{b}
 * @param[in]   padding_mode    The padding mode to be applied at signal boundaries
 * 
 * @ingroup bfp32_func
 */
C_API
void bfp_s32_convolve_same(
  bfp_s32_t* y,
  const bfp_s32_t* x,
  const int32_t b_q30[],
  const unsigned b_length,
  const pad_mode_e padding_mode);