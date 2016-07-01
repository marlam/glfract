#version 400

/*
 * Copyright (C) 2015, 2016  Martin Lambers <marlam@marlam.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 *
 * PART 1: floating point type with basic operations
 *
 */

/* This implements basic arithmetic operations for a FLOAT type, which can be
 * one of the following:
 * - float: single precision floating point, natively supported
 * - double: double precision floating point, natively supported
 * - doublefloat: emulated extended precision based on a pair of floats
 * - doubledouble: emulated extended precision based on a pair of doubles
 * The emulated types are implemented using algorithms from the following papers:
 * - "A Floating-Point Technique for Extending the Available Precision" by T. J.
 *   Dekker
 * - "Library for Double-Double and Quad-Double Arithmetic" by Yozo Hida, Xiaoye
 *   S. Li, David H. Bailey
 * - "Extended-Precision Floating-Point Numbers for GPU Computation" by Andrew
 *   Thall
 */

// FLOAT_TYPE uses the same values as float_type in the C++ source
#if (FLOAT_TYPE == 0)
# define FLOAT float
# define FLOAT_EMU 0
#elif (FLOAT_TYPE == 1)
# define FLOAT double
# define FLOAT_EMU 0
#elif (FLOAT_TYPE == 2)
# define FLOAT vec2
# define FLOAT_EMU 1
# define BASEFLOAT float
# define BASEVEC2 vec2
# define BASEVEC4 vec4
const float SPLIT = 4097.0; // (1 << 12) + 1;
#elif (FLOAT_TYPE == 3)
# define FLOAT dvec2
# define FLOAT_EMU 1 
# define BASEFLOAT double
# define BASEVEC2 dvec2
# define BASEVEC4 dvec4
const double SPLIT = 134217729.0LF; // (1 << 27) + 1;
#endif


/* Building blocks for emulation based on pairs */

#if FLOAT_EMU

BASEVEC2 two_add(BASEFLOAT a, BASEFLOAT b)
{
    precise BASEFLOAT s = a + b;
    precise BASEFLOAT v = s - a;
    precise BASEFLOAT e = (a - (s - v)) + (b - v);
    return BASEVEC2(s, e);
}
BASEVEC2 quick_two_add(BASEFLOAT a, BASEFLOAT b) // requires abs(a) >= abs(b)
{
    precise BASEFLOAT s = a + b;
    precise BASEFLOAT e = b - (s - a);
    return BASEVEC2(s, e);
}
BASEVEC4 two_add_comp(BASEVEC2 a, BASEVEC2 b)
{
    precise BASEVEC2 s = a + b;
    precise BASEVEC2 v = s - a;
    precise BASEVEC2 e = (a - (s - v)) + (b - v);
    return BASEVEC4(s.x, e.x, s.y, e.y);
}
BASEVEC4 two_sub_comp(BASEVEC2 a, BASEVEC2 b)
{
    precise BASEVEC2 s = a - b;
    precise BASEVEC2 v = s - a;
    precise BASEVEC2 e = (a - (s - v)) - (b + v);
    return BASEVEC4(s.x, e.x, s.y, e.y);
}
BASEVEC2 split(BASEFLOAT a)
{
    precise BASEFLOAT t = SPLIT * a;
    precise BASEFLOAT b_hi = t - (t - a);
    precise BASEFLOAT b_lo = a - b_hi;
    return BASEVEC2(b_hi, b_lo);
}
BASEVEC4 split_comp(BASEVEC2 a)
{
    precise BASEVEC2 t = SPLIT * a;
    precise BASEVEC2 b_hi = t - (t - a);
    precise BASEVEC2 b_lo = a - b_hi;
    return BASEVEC4(b_hi.x, b_lo.x, b_hi.y, b_lo.y);
}
BASEVEC2 two_mul(BASEVEC2 ab)
{
    precise BASEFLOAT p = ab.x * ab.y;
    precise BASEVEC4 s = split_comp(ab);
    precise BASEFLOAT e = ((s.x * s.z - p) + s.x * s.w + s.y * s.z) + s.y * s.w;
    return BASEVEC2(p, e);
}
BASEVEC2 two_sqr(BASEFLOAT a)
{
    precise BASEFLOAT p = a * a;
    precise BASEVEC2 s = split(a);
    precise BASEFLOAT e = ((s.x * s.x - p) + BASEFLOAT(2) * s.x * s.y) + s.y * s.y;
    return BASEVEC2(p, e);
}

BASEVEC2 emu_add(BASEVEC2 a, BASEFLOAT b)
{
    precise BASEVEC2 s = two_add(a.x, b);
    s.y += a.y;
    precise BASEVEC2 r = quick_two_add(s.x, s.y);
    return r;
}
BASEVEC2 emu_add(BASEVEC2 a, BASEVEC2 b)
{
    precise BASEVEC4 st = two_add_comp(a, b);
    st.y += st.z;
    st.xy = quick_two_add(st.x, st.y);
    st.y += st.w;
    st.xy = quick_two_add(st.x, st.y);
    return st.xy;
}
BASEVEC2 emu_sub(BASEVEC2 a, BASEVEC2 b)
{
    precise BASEVEC4 st = two_sub_comp(a, b);
    st.y += st.z;
    st.xy = quick_two_add(st.x, st.y);
    st.y += st.w;
    st.xy = quick_two_add(st.x, st.y);
    return st.xy;
}
BASEVEC2 emu_mul(BASEVEC2 a, BASEFLOAT b)
{
    precise BASEVEC2 p = two_mul(BASEVEC2(a.x, b));
    p.y += a.y * b;
    precise BASEVEC2 r = quick_two_add(p.x, p.y);
    return r;
}
BASEVEC2 emu_mul(BASEVEC2 a, BASEVEC2 b)
{
    precise BASEVEC2 p = two_mul(BASEVEC2(a.x, b.x));
    p.y += dot(a, b.yx);
    precise BASEVEC2 r = quick_two_add(p.x, p.y);
    return r;
}
BASEVEC2 emu_div(BASEVEC2 a, BASEVEC2 b)
{
    precise BASEFLOAT q0 = a.x / b.x;
    precise BASEVEC2 r = emu_sub(a, emu_mul(b, q0));
    precise BASEFLOAT q1 = r.x / b.x;
    r = emu_sub(r, emu_mul(b, q1));
    precise BASEFLOAT q2 = r.x / b.x;
    r = emu_add(quick_two_add(q0, q1), q2);
    return r;
}
BASEVEC2 emu_sqr(BASEFLOAT a)
{
    precise BASEVEC2 p = two_sqr(a);
    precise BASEVEC2 r = quick_two_add(p.x, p.y);
    return r;
}
BASEVEC2 emu_sqr(BASEVEC2 a)
{
    precise BASEVEC2 p = two_sqr(a.x);
    p.y += BASEFLOAT(2) * a.x * a.y;
    precise BASEVEC2 s = quick_two_add(p.x, p.y);
    return s;
}
BASEVEC2 emu_sqrt(BASEVEC2 a)
{
    precise BASEFLOAT x = inversesqrt(a.x);
    precise BASEFLOAT ax = a.x * x;
    precise BASEFLOAT diff = emu_sub(a, emu_sqr(ax)).x;
    precise BASEFLOAT prod = diff * x * BASEFLOAT(0.5);
    precise BASEVEC2 r = two_add(prod, ax);
    return r;
}

#endif

/* Unify operations for native and emulated types. Necessary because
 * there is no operator overloading in GLSL. */

FLOAT to_FLOAT(float x)
{
#if FLOAT_EMU
    return FLOAT(x, BASEFLOAT(0));
#else
    return FLOAT(x);
#endif
}

float to_float(FLOAT x)
{
#if FLOAT_EMU
    return float(x.x);
#else
    return float(x);
#endif
}

FLOAT xabs(FLOAT a)
{
#if FLOAT_EMU
    return (a.x < BASEFLOAT(0) ? -a : a);
#else
    return abs(a);
#endif
}

int xcmp(FLOAT a, float b)
{
#if FLOAT_EMU
    if (a.x < b || (a.x == b && a.y < BASEFLOAT(0)))
        return -1;
    else if (a.x == b && a.y == 0.0)
        return 0;
    else //if (a.x > b || (a.x == b && a.y > BASEFLOAT(0.0)))
        return +1;
#else
    return (a < b ? -1 : a > b ? +1 : 0);
#endif
}

FLOAT xadd(FLOAT a, FLOAT b)
{
#if FLOAT_EMU
    return emu_add(a, b);
#else
    return a + b;
#endif
}

FLOAT xsub(FLOAT a, FLOAT b)
{
#if FLOAT_EMU
    return emu_sub(a, b);
#else
    return a - b;
#endif
}

FLOAT xmul(FLOAT a, FLOAT b)
{
#if FLOAT_EMU
    return emu_mul(a, b);
#else
    return a * b;
#endif
}

FLOAT xdiv(FLOAT a, FLOAT b)
{
#if FLOAT_EMU
    return emu_div(a, b);
#else
    return a / b;
#endif
}

FLOAT xsqr(FLOAT a)
{
#if FLOAT_EMU
    return emu_sqr(a);
#else
    return a * a;
#endif
}

FLOAT xsqrt(FLOAT a)
{
#if FLOAT_EMU
    return emu_sqrt(a);
#else
    return sqrt(a);
#endif
}


/*
 *
 * PART 2: complex math based on the FLOAT type
 *
 */

struct complex_t {
    FLOAT re, im;
};

complex_t add(complex_t a, complex_t b)
{
    return complex_t(xadd(a.re, b.re), xadd(a.im, b.im));
}

complex_t sub(complex_t a, complex_t b)
{
    return complex_t(xsub(a.re, b.re), xsub(a.im, b.im));
}

complex_t mul(complex_t a, complex_t b)
{
    return complex_t(xsub(xmul(a.re, b.re), xmul(a.im, b.im)),
            xadd(xmul(a.im, b.re), xmul(a.re, b.im)));
}

complex_t sqr(complex_t a)
{
    complex_t r;
    r.re = xsub(xsqr(a.re), xsqr(a.im));
    r.im = xmul(a.re, a.im);
    r.im = xadd(r.im, r.im);
    return r;
}

complex_t powui(complex_t a, int i) // i >= 1
{
    complex_t r = a;
    int j = 1;
    while (i >= 2 * j) {
        r = sqr(r);
        j *= 2;
    }
    for (int k = j; k < i; k++) {
        r = mul(r, a);
    }
    return r;
}

FLOAT abs_sqr(complex_t a)
{
    return xadd(xsqr(a.re), xsqr(a.im));
}

FLOAT abs(complex_t a)
{
    return xsqrt(abs_sqr(a));
}


/*
 *
 * PART 3: the fractal set
 *
 */

// MANDELBROT_POWER: >= 2
// MANDELBROT_LN_POWER: ln(MANDELBROT_POWER)
// MANDELBROT_MAX_ITERATIONS: e.g. 256
// MANDELBROT_BAILOUT: e.g. 4
// MANDELBROT_SMOOTH: 0 or 1

#define M_LN2 0.69314718055994530942

float fractal(complex_t c)
{
    int i = 0;
    complex_t z = complex_t(to_FLOAT(0), to_FLOAT(0));
    FLOAT abssqrz;
    do {
        z = add(powui(z, MANDELBROT_POWER), c);
        i++;
        abssqrz = abs_sqr(z);
    }
    while (xcmp(abssqrz, MANDELBROT_BAILOUT) < 0
            && i < MANDELBROT_MAX_ITERATIONS);
    float ret = 0.0;
    if (i < MANDELBROT_MAX_ITERATIONS) {
#if MANDELBROT_SMOOTH
        ret = float(i) - log(log(to_float(xsqrt(abssqrz))) / M_LN2) / MANDELBROT_LN_POWER;
        ret /= float(MANDELBROT_MAX_ITERATIONS - 1);
#else
        ret = float(i) / float(MANDELBROT_MAX_ITERATIONS - 1);
#endif
    }
    return ret;
}


/*
 *
 * PART 4: the main function
 *
 */

smooth in vec2 vxy;

uniform FLOAT x0;
uniform FLOAT xw;
uniform FLOAT y0;
uniform FLOAT yw;

layout(location = 0) out float fcolor;

void main(void)
{
    FLOAT re = xadd(x0, xmul(to_FLOAT(vxy.x), xw));
    FLOAT im = xadd(y0, xmul(to_FLOAT(vxy.y), yw));
    fcolor = fractal(complex_t(re, im));
}
