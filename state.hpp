/*
 * Copyright (C) 2015  Martin Lambers <marlam@marlam.de>
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

#ifndef STATE_HPP
#define STATE_HPP

#include <vector>

class QString;

typedef enum {
    fractal_mandelbrot
} fractal_type_t;

typedef enum {
    // These values are re-used in the fragment shader; keep them synchronized!
    precision_native_float = 0,
    precision_native_double = 1,
    precision_emu_doublefloat = 2,
    precision_emu_doubledouble = 3
} precision_type_t;

class State
{
public:
    // Fractal
    struct {
        fractal_type_t type;
        struct {
            int power;
            int max_iter;
            float bailout;
            bool smooth;
            __float128 x0, xw, y0, yw;
        } mandelbrot;
    } fractal;

    // Precision
    struct {
        precision_type_t type;
    } precision;

    // Color map
    struct {
        std::vector<unsigned char> colors;
        bool reverse;
        float start; // in [0,1]
        bool animation;
        bool animation_reverse;
        int animation_speed; // in cycles-per-minute, >= 1
    } colormap;

    // Navigation
    struct {
        __float128 x, y, zoom;
    } navigation;

    State();

    void save(const QString& filename) const;
    void load(const QString& filename, bool enable_double_based_precisions);
};

#endif
