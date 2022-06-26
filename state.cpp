/*
 * Copyright (C) 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022
 * Martin Lambers <marlam@marlam.de>
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

#include "state.hpp"

#include <cstring>
#include <cmath>

#include <QSettings>
#include <QString>

#include <quadmath.h>


// Generated with gencolormap https://github.com/marlam/gencolormap
// Sequential palette with parameters
// hue=270 warmth=0.95 contrast=1 saturation=1 brightness=1
static unsigned char default_colormap[] = {
    247, 255, 155, 247, 255, 156, 246, 255, 156, 246, 255, 156, 246, 255, 157, 245, 255, 157, 245, 255, 158, 244, 255,
    158, 244, 255, 158, 244, 255, 159, 243, 255, 159, 243, 255, 159, 242, 255, 160, 242, 255, 160, 242, 255, 160, 241,
    255, 161, 241, 255, 161, 240, 255, 162, 240, 255, 162, 240, 255, 162, 239, 255, 163, 239, 255, 163, 238, 255, 163,
    238, 254, 164, 238, 254, 164, 237, 253, 164, 237, 253, 165, 236, 252, 165, 236, 252, 166, 235, 251, 166, 235, 251,
    166, 235, 250, 167, 234, 250, 167, 234, 249, 167, 233, 248, 168, 233, 248, 168, 232, 247, 168, 232, 247, 169, 231,
    246, 169, 231, 246, 169, 231, 245, 170, 230, 245, 170, 230, 244, 170, 229, 244, 171, 229, 243, 171, 228, 243, 171,
    228, 242, 172, 227, 242, 172, 227, 241, 172, 226, 240, 173, 226, 240, 173, 225, 239, 173, 225, 239, 174, 224, 238,
    174, 224, 238, 174, 223, 237, 175, 223, 236, 175, 222, 236, 175, 222, 235, 176, 221, 235, 176, 221, 234, 176, 220,
    233, 177, 220, 233, 177, 219, 232, 177, 219, 232, 178, 218, 231, 178, 218, 230, 178, 217, 230, 179, 217, 229, 179,
    216, 229, 179, 216, 228, 180, 215, 227, 180, 215, 227, 180, 214, 226, 181, 214, 225, 181, 213, 225, 181, 213, 224,
    182, 212, 224, 182, 211, 223, 182, 211, 222, 182, 210, 222, 183, 210, 221, 183, 209, 220, 183, 209, 220, 184, 208,
    219, 184, 208, 218, 184, 207, 218, 185, 206, 217, 185, 206, 216, 185, 205, 216, 185, 205, 215, 186, 204, 214, 186,
    204, 213, 186, 203, 213, 187, 202, 212, 187, 202, 211, 187, 201, 211, 188, 201, 210, 188, 200, 209, 188, 199, 208,
    188, 199, 208, 189, 198, 207, 189, 197, 206, 189, 197, 206, 189, 196, 205, 190, 196, 204, 190, 195, 203, 190, 194,
    203, 191, 194, 202, 191, 193, 201, 191, 192, 200, 191, 192, 200, 192, 191, 199, 192, 191, 198, 192, 190, 197, 192,
    189, 196, 193, 189, 196, 193, 188, 195, 193, 187, 194, 194, 187, 193, 194, 186, 192, 194, 185, 192, 194, 185, 191,
    195, 184, 190, 195, 183, 189, 195, 183, 188, 195, 182, 188, 196, 181, 187, 196, 180, 186, 196, 180, 185, 196, 179,
    184, 197, 178, 183, 197, 178, 182, 197, 177, 182, 197, 176, 181, 198, 176, 180, 198, 175, 179, 198, 174, 178, 198,
    173, 177, 198, 173, 176, 199, 172, 175, 199, 171, 174, 199, 170, 174, 199, 170, 173, 200, 169, 172, 200, 168, 171,
    200, 167, 170, 200, 167, 169, 201, 166, 168, 201, 165, 167, 201, 164, 166, 201, 164, 165, 201, 163, 164, 202, 162,
    163, 202, 161, 162, 202, 161, 161, 202, 160, 160, 203, 159, 159, 203, 158, 158, 203, 157, 157, 203, 157, 156, 203,
    156, 155, 204, 155, 154, 204, 154, 153, 204, 153, 152, 204, 153, 151, 205, 152, 150, 205, 151, 149, 205, 150, 148,
    205, 149, 147, 206, 148, 146, 206, 148, 145, 206, 147, 144, 206, 146, 143, 206, 145, 142, 207, 144, 140, 207, 143,
    139, 207, 143, 138, 207, 142, 137, 208, 141, 136, 208, 140, 135, 208, 139, 134, 208, 138, 132, 209, 137, 131, 209,
    136, 130, 209, 136, 129, 209, 135, 128, 210, 134, 126, 210, 133, 125, 210, 132, 124, 211, 131, 123, 211, 130, 121,
    211, 129, 120, 212, 128, 119, 212, 127, 118, 212, 127, 116, 213, 126, 115, 213, 125, 114, 213, 124, 112, 214, 123,
    111, 214, 122, 109, 214, 121, 108, 215, 120, 107, 215, 119, 105, 216, 118, 104, 216, 117, 102, 217, 116, 101, 217,
    115, 99, 218, 114, 98, 218, 114, 96, 219, 113, 94, 219, 112, 93, 220, 111, 91, 221, 110, 89, 221, 109, 88, 222, 108,
    86, 223, 107, 84, 224, 106, 82, 224, 105, 80, 225, 104, 78, 226, 103, 76, 227, 102, 74, 228, 101, 71, 229, 100, 69,
    231, 99, 66, 232, 98, 64, 233, 97, 61, 235, 96, 58, 236, 95, 54, 238, 95, 51, 240, 94, 47, 241, 93, 42, 244, 92, 37,
    246, 91, 30, 248, 90, 21, 251, 89, 7, 254, 88, 0, 253, 87, 0, 250, 86, 0, 247, 85, 0, 244, 83, 0, 241, 82, 0, 238,
    81, 0, 235, 80, 0, 232, 79, 0, 229, 78, 0, 225, 76, 0, 222, 75, 0, 219, 74, 0, 216, 73, 0, 213, 72, 0, 210, 70, 0,
    207, 69, 0, 203, 68, 0, 200, 67, 0, 197, 65, 0, 194, 64, 0, 191, 63, 0, 187, 62, 0, 184, 61, 0, 181, 59, 0, 178, 58,
    0, 175, 57, 0, 171, 56, 0, 168, 54, 0, 165, 53, 0, 162, 52, 0, 158, 51, 0, 155, 49, 0, 152, 48, 0, 148, 47, 0, 145,
    46, 0, 142, 44, 0, 139, 43, 0, 135, 42, 0, 132, 41, 0, 129, 39, 0, 125, 38, 0, 122, 37, 0, 119, 35, 0, 115, 34, 0,
    112, 33, 0, 109, 32, 0, 106, 30, 0, 102, 29, 0, 99, 28, 0, 96, 27, 0, 92, 25, 0, 89, 24, 0, 86, 23, 0, 82, 21, 0,
    79, 20, 0, 75, 18, 0, 71, 17, 0, 66, 15, 0, 61, 13, 0, 56, 10, 0, 50, 8, 0, 43, 5, 0, 34, 3, 0, 22, 0, 0, 0
};

static size_t default_colormap_size = sizeof(default_colormap) / sizeof(unsigned char);


State::State()
{
    fractal.type = fractal_mandelbrot;
    fractal.mandelbrot.power = 2;
    fractal.mandelbrot.max_iter = 170;
    fractal.mandelbrot.bailout = 4.0f;
    fractal.mandelbrot.smooth = true;
    fractal.mandelbrot.x0 = -2.5Q;
    fractal.mandelbrot.xw = 1.0Q - fractal.mandelbrot.x0;
    fractal.mandelbrot.y0 = -1.25Q;
    fractal.mandelbrot.yw = 1.25Q - fractal.mandelbrot.y0;
    precision.type = precision_native_float;
    colormap.colors.insert(colormap.colors.begin(), default_colormap, default_colormap + default_colormap_size);
    colormap.reverse = true;
    colormap.start = 0.0f;
    colormap.animation = false;
    colormap.animation_reverse = false;
    colormap.animation_speed = 20;
    navigation.x = -0.75Q;
    navigation.y = 0.0Q;
    navigation.zoom = 1.0Q;
}

void State::save(const QString& filename) const
{
    char buf[64];
    QSettings settings(filename, QSettings::IniFormat);

    settings.beginGroup("fractal");
    switch (fractal.type) {
    case fractal_mandelbrot:
        settings.setValue("type", "mandelbrot");
        break;
    }
    settings.beginGroup("mandelbrot");
    settings.setValue("power", fractal.mandelbrot.power);
    settings.setValue("max_iter", fractal.mandelbrot.max_iter);
    settings.setValue("bailout", QString::number(fractal.mandelbrot.bailout));
    settings.setValue("smooth", fractal.mandelbrot.smooth);
    quadmath_snprintf(buf, sizeof(buf), "%.36Qg", fractal.mandelbrot.x0);
    settings.setValue("x0", buf);
    quadmath_snprintf(buf, sizeof(buf), "%.36Qg", fractal.mandelbrot.xw);
    settings.setValue("xw", buf);
    quadmath_snprintf(buf, sizeof(buf), "%.36Qg", fractal.mandelbrot.y0);
    settings.setValue("y0", buf);
    quadmath_snprintf(buf, sizeof(buf), "%.36Qg", fractal.mandelbrot.yw);
    settings.setValue("yw", buf);
    settings.endGroup();
    settings.endGroup();

    settings.beginGroup("precision");
    switch (precision.type) {
    case precision_native_float:
        settings.setValue("type", "native_float");
        break;
    case precision_native_double:
        settings.setValue("type", "native_double");
        break;
    case precision_emu_doublefloat:
        settings.setValue("type", "emu_doublefloat");
        break;
    case precision_emu_doubledouble:
        settings.setValue("type", "emu_doubledouble");
        break;
    }
    settings.endGroup();

    settings.beginGroup("colormap");
    settings.setValue("n", static_cast<unsigned int>(colormap.colors.size()) / 3);
    for (size_t i = 0; i < colormap.colors.size() / 3; i++) {
        settings.setValue(QString("color") + QString::number(i),
                QString::number(colormap.colors[3 * i + 0]) + ","
                + QString::number(colormap.colors[3 * i + 1]) + ","
                + QString::number(colormap.colors[3 * i + 2]));
    }
    settings.setValue("reverse", colormap.reverse);
    settings.setValue("start", QString::number(colormap.start));
    settings.setValue("animation", colormap.animation);
    settings.setValue("animation_reverse", colormap.animation_reverse);
    settings.setValue("animation_speed", colormap.animation_speed);
    settings.endGroup();

    settings.beginGroup("navigation");
    quadmath_snprintf(buf, sizeof(buf), "%.36Qg", navigation.x);
    settings.setValue("x", buf);
    quadmath_snprintf(buf, sizeof(buf), "%.36Qg", navigation.y);
    settings.setValue("y", buf);
    quadmath_snprintf(buf, sizeof(buf), "%.36Qg", navigation.zoom);
    settings.setValue("zoom", buf);
    settings.endGroup();
}

void State::load(const QString& filename, bool enable_double_based_precisions)
{
    QSettings settings(filename, QSettings::IniFormat);
    State defaults;
    QString tmp;

    settings.beginGroup("fractal");
    fractal.type = defaults.fractal.type;
    tmp = settings.value("type").toString();
    if (tmp == "mandelbrot")
        fractal.type = fractal_mandelbrot;
    settings.beginGroup("mandelbrot");
    fractal.mandelbrot.power = settings.value("power", QString::number(defaults.fractal.mandelbrot.power)).toInt();
    fractal.mandelbrot.max_iter = settings.value("max_iter", QString::number(defaults.fractal.mandelbrot.max_iter)).toInt();
    fractal.mandelbrot.bailout = settings.value("bailout", QString::number(defaults.fractal.mandelbrot.bailout)).toFloat();
    fractal.mandelbrot.smooth = settings.value("smooth", QString::number(defaults.fractal.mandelbrot.smooth)).toBool();
    fractal.mandelbrot.x0 = defaults.fractal.mandelbrot.x0;
    tmp = settings.value("x0").toString();
    if (!tmp.isEmpty())
        fractal.mandelbrot.x0 = strtoflt128(qPrintable(tmp), 0);
    fractal.mandelbrot.xw = defaults.fractal.mandelbrot.xw;
    tmp = settings.value("xw").toString();
    if (!tmp.isEmpty())
        fractal.mandelbrot.xw = strtoflt128(qPrintable(tmp), 0);
    fractal.mandelbrot.y0 = defaults.fractal.mandelbrot.y0;
    tmp = settings.value("y0").toString();
    if (!tmp.isEmpty())
        fractal.mandelbrot.y0 = strtoflt128(qPrintable(tmp), 0);
    fractal.mandelbrot.yw = defaults.fractal.mandelbrot.yw;
    tmp = settings.value("yw").toString();
    if (!tmp.isEmpty())
        fractal.mandelbrot.yw = strtoflt128(qPrintable(tmp), 0);
    settings.endGroup();
    settings.endGroup();

    settings.beginGroup("precision");
    precision.type = defaults.precision.type;
    tmp = settings.value("type").toString();
    if (tmp == "native_float")
        precision.type = precision_native_float;
    else if (tmp == "native_double" && enable_double_based_precisions)
        precision.type = precision_native_double;
    else if (tmp == "emu_doublefloat")
        precision.type = precision_emu_doublefloat;
    else if (tmp == "emu_doubledouble" && enable_double_based_precisions)
        precision.type = precision_emu_doubledouble;
    settings.endGroup();

    settings.beginGroup("colormap");
    unsigned int n = settings.value("n", "1").toUInt();
    if (n > 1024)
        n = 1024;
    colormap.colors.resize(3 * n);
    for (size_t i = 0; i < colormap.colors.size() / 3; i++) {
        unsigned char r = 0, g = 0, b = 0;
        tmp = settings.value(QString("color") + QString::number(i)).toString();
        QStringList collist = tmp.split(',');
        if (collist.size() == 3) {
            r = collist[0].toInt();
            g = collist[1].toInt();
            b = collist[2].toInt();
        }
        colormap.colors[3 * i + 0] = r;
        colormap.colors[3 * i + 1] = g;
        colormap.colors[3 * i + 2] = b;
    }
    colormap.reverse = settings.value("reverse", QString(defaults.colormap.reverse ? "true" : "false")).toBool();
    colormap.start = settings.value("start", QString::number(defaults.colormap.start)).toFloat();
    colormap.animation = settings.value("animation", QString(defaults.colormap.animation ? "true" : "false")).toBool();
    colormap.animation_reverse = settings.value("animation_reverse", QString(defaults.colormap.animation_reverse ? "true" : "false")).toBool();
    colormap.animation_speed = settings.value("animation_speed", QString::number(defaults.colormap.animation_speed)).toInt();
    settings.endGroup();

    settings.beginGroup("navigation");
    navigation.x = defaults.navigation.x;
    tmp = settings.value("x").toString();
    if (!tmp.isEmpty())
        navigation.x = strtoflt128(qPrintable(tmp), 0);
    navigation.y = defaults.navigation.y;
    tmp = settings.value("y").toString();
    if (!tmp.isEmpty())
        navigation.y = strtoflt128(qPrintable(tmp), 0);
    navigation.zoom = defaults.navigation.zoom;
    tmp = settings.value("zoom").toString();
    if (!tmp.isEmpty())
        navigation.zoom = strtoflt128(qPrintable(tmp), 0);
    settings.endGroup();
}
