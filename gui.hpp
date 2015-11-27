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

#ifndef GUI_HPP
#define GUI_HPP

#include <QMainWindow>

#include "state.hpp"

class QOpenGLShaderProgram;
class QLabel;
class QSlider;
class QRadioButton;
class QPushButton;
class QSpinBox;
class QDoubleSpinBox;
class QCheckBox;
class QElapsedTimer;
class QImage;

class GLWidget;

class GUI : public QMainWindow
{
Q_OBJECT

public:
    GUI();
    ~GUI();

    void activate();

private:
    bool update_lock;
    State state;

    GLWidget* glwidget;

    QRadioButton* fractal_btn;
    QSpinBox* mandelbrot_power_spinbox;
    QSpinBox* mandelbrot_max_iter_spinbox;
    QDoubleSpinBox* mandelbrot_bailout_spinbox;
    QCheckBox* mandelbrot_smooth_checkbox;

    QRadioButton* precision_single_hw_btn;
    QRadioButton* precision_double_emu_btn;
    QRadioButton* precision_double_hw_btn;
    QRadioButton* precision_quad_emu_btn;

    QLabel* colormap_label;
    QCheckBox* colormap_reverse_checkbox;
    QSlider* colormap_start_slider;
    QCheckBox* colormap_animation_checkbox;
    QCheckBox* colormap_animation_reverse_checkbox;
    QSlider* colormap_animation_speed_slider;

    void state_to_gui();
    void gui_to_state();
    void colormap_from_img(const QImage& img);
    void update_colormap_label();

private slots:
    void navigate(__float128 x, __float128 y, __float128 zoom);
    void update();

    void colormap_from_png();
    void colormap_from_clipboard();

    void file_save();
    void file_export_png();
    void edit_copy();
    void help_about();

public slots:
    void file_open(const QString& file_name = QString());
};

#endif
