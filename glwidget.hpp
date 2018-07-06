/*
 * Copyright (C) 2015, 2016, 2017, 2018  Martin Lambers <marlam@marlam.de>
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

#ifndef GLWIDGET_HPP
#define GLWIDGET_HPP

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>

#include "state.hpp"

class QOpenGLShaderProgram;
class QElapsedTimer;

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
Q_OBJECT
public:
    bool have_arb_gpu_shader_fp64;
    bool have_arb_gpu_shader5;

private:
    // The state to render next
    State _state;
    // GLSL shader compile-time constants
    int _mandelbrot_power;
    int _mandelbrot_max_iter;
    float _mandelbrot_bailout;
    bool _mandelbrot_smooth;
    precision_type_t _precision_type;
    // Colormap reload flag
    bool _colormap_reupload;
    // Colormap animation timer
    QElapsedTimer *_colormap_timer;
    // Last shown fractal region
    __float128 _x0, _xw, _y0, _yw;
    // Navigation variables
    bool _zoom_in, _zoom_out, _shift;
    __float128 _zoom_step;
    __float128 _shift_start_x0, _shift_start_y0;
    int _navig_start_x, _navig_start_y;
    int _navig_event_x, _navig_event_y;
    // GL resources
    QOpenGLShaderProgram* _fractal_prg;
    QOpenGLShaderProgram* _coloring_prg;
    GLuint _fractal_fbo;
    GLuint _fractal_tex;
    GLuint _colormap_tex;
    // GL extensions that are not available via QOpenGLFunctions_3_3_Core
    void (*glUniform1d)(GLint location, GLdouble v0);
    void (*glUniform2d)(GLint location, GLdouble v0, GLdouble v1);

public:
    GLWidget();
    ~GLWidget();

    int heightForWidth(int w) const override;

    void set_state(const State& state);
    void state_has_new_colormap();

signals:
    void navigate(__float128 x, __float128 y, __float128 zoom);

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
};

#endif
