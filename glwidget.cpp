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

#include <cstring>
#include <cmath>

#include <QOpenGLShaderProgram>
#include <QElapsedTimer>
#include <QKeyEvent>
#include <QMouseEvent>

#include <quadmath.h>

#include "glwidget.hpp"


template<typename T>
static void float128_to_pair(__float128 x, T* p0, T* p1)
{
    *p0 = x;
    *p1 = x - *p0;
}

GLWidget::GLWidget() : QOpenGLWidget(), QOpenGLFunctions_3_3_Core(),
    have_arb_gpu_shader_fp64(false), have_arb_gpu_shader5(false),
    _state(),
    _mandelbrot_power(-1), _mandelbrot_max_iter(-1), _mandelbrot_bailout(-1.0f), _mandelbrot_smooth(false),
    _precision_type(precision_native_float),
    _colormap_reupload(true), _colormap_timer(new QElapsedTimer),
    _x0(NAN), _xw(NAN), _y0(NAN), _yw(NAN),
    _zoom_in(false), _zoom_out(false), _shift(false), _zoom_step(0.02Q),
    _navig_start_x(0), _navig_start_y(0), _navig_event_x(0), _navig_event_y(0)
{
    setMinimumSize(256, 256);
    setFocusPolicy(Qt::StrongFocus);
}

GLWidget::~GLWidget()
{
}

int GLWidget::heightForWidth(int w) const
{
    __float128 fractal_ar = _state.fractal.mandelbrot.xw / _state.fractal.mandelbrot.yw;
    return w / fractal_ar;
}

void GLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    have_arb_gpu_shader_fp64 = context()->hasExtension("GL_ARB_gpu_shader_fp64");
    have_arb_gpu_shader5 = context()->hasExtension("GL_ARB_gpu_shader5");
    glUniform1d = reinterpret_cast<void (*)(GLint, GLdouble)>(context()->getProcAddress("glUniform1d"));
    glUniform2d = reinterpret_cast<void (*)(GLint, GLdouble, GLdouble)>(context()->getProcAddress("glUniform2d"));

    const float p[] = {
        -1.0f, +1.0f, 0.0f,
        +1.0f, +1.0f, 0.0f,
        +1.0f, -1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f
    };
    const float t[] = {
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f
    };
    const unsigned int i[] = {
        0, 1, 3, 1, 2, 3
    };

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    GLuint position_buffer;
    glGenBuffers(1, &position_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
    glBufferData(GL_ARRAY_BUFFER, 4 * 3 * sizeof(float), p, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    GLuint texcoord_buffer;
    glGenBuffers(1, &texcoord_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, texcoord_buffer);
    glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(float), t, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);
    GLuint index_buffer;
    glGenBuffers(1, &index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), i, GL_STATIC_DRAW);

    glGenTextures(1, &_fractal_tex);
    glBindTexture(GL_TEXTURE_2D, _fractal_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenFramebuffers(1, &_fractal_fbo);

    glGenTextures(1, &_colormap_tex);
    glBindTexture(GL_TEXTURE_2D, _colormap_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    _fractal_prg = new QOpenGLShaderProgram();
    _coloring_prg = new QOpenGLShaderProgram();
    _coloring_prg->addShaderFromSourceFile(QOpenGLShader::Vertex, ":vs.glsl");
    _coloring_prg->addShaderFromSourceFile(QOpenGLShader::Fragment, ":coloring-fs.glsl");

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glDisable(GL_DEPTH_TEST);
}

void GLWidget::set_state(const State& state)
{
    _state = state;
    update();
}

void GLWidget::state_has_new_colormap()
{
    _colormap_reupload = true;
}

void GLWidget::paintGL()
{
    // Colormap animation timing. Always done at start of drawing in the hope
    // of getting regular results.
    qint64 animation_nsecs = 0;
    if (_state.colormap.animation) {
        if (!_colormap_timer->isValid())
            _colormap_timer->start();
        else
            animation_nsecs = _colormap_timer->nsecsElapsed();
    } else {
        if (_colormap_timer->isValid())
            _colormap_timer->invalidate();
    }

    // Re-initialize resources where necessary
    bool reinitialize_everything = !_fractal_prg->isLinked();
    bool rebuild_fractal_prg = false;
    if (reinitialize_everything
            || _state.fractal.mandelbrot.power != _mandelbrot_power
            || _state.fractal.mandelbrot.max_iter != _mandelbrot_max_iter
            || _state.fractal.mandelbrot.bailout != _mandelbrot_bailout
            || _state.fractal.mandelbrot.smooth != _mandelbrot_smooth
            || _state.precision.type != _precision_type) {
        rebuild_fractal_prg = true;
        _mandelbrot_power = _state.fractal.mandelbrot.power;
        _mandelbrot_max_iter = _state.fractal.mandelbrot.max_iter;
        _mandelbrot_bailout = _state.fractal.mandelbrot.bailout;
        _mandelbrot_smooth = _state.fractal.mandelbrot.smooth;
        _precision_type = _state.precision.type;
    }
    if (rebuild_fractal_prg) {
        QFile file(":fractal-fs.glsl");
        file.open(QIODevice::ReadOnly);
        QTextStream ts(&file);
        QString fs_src = ts.readAll();
        fs_src.replace("HAVE_ARB_GPU_SHADER5", have_arb_gpu_shader5 ? "1" : "0");
        fs_src.replace("FLOAT_TYPE", QString::number(_precision_type));
        fs_src.replace("MANDELBROT_POWER", QString::number(_mandelbrot_power));
        fs_src.replace("MANDELBROT_LN_POWER", QString::number(std::log(static_cast<float>(_mandelbrot_power))));
        fs_src.replace("MANDELBROT_MAX_ITERATIONS", QString::number(_mandelbrot_max_iter));
        fs_src.replace("MANDELBROT_BAILOUT", QString::number(_mandelbrot_bailout));
        fs_src.replace("MANDELBROT_SMOOTH", _mandelbrot_smooth ? "1" : "0");
        _fractal_prg->removeAllShaders();
        _fractal_prg->addShaderFromSourceFile(QOpenGLShader::Vertex, ":vs.glsl");
        _fractal_prg->addShaderFromSourceCode(QOpenGLShader::Fragment, fs_src);
        _fractal_prg->bind();
    }
    glActiveTexture(GL_TEXTURE0);
    GLint fractal_tex_width, fractal_tex_height;
    glBindTexture(GL_TEXTURE_2D, _fractal_tex);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &fractal_tex_width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &fractal_tex_height);
    if (reinitialize_everything || fractal_tex_width != width() || fractal_tex_height != height()) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width(), height(), 0, GL_RED, GL_FLOAT, NULL);
        glBindFramebuffer(GL_FRAMEBUFFER, _fractal_fbo);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _fractal_tex, 0);
    }
    if (reinitialize_everything || _colormap_reupload) {
        glBindTexture(GL_TEXTURE_2D, _colormap_tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, _state.colormap.colors.size() / 3, 1, 0,
                GL_RGB, GL_UNSIGNED_BYTE, _state.colormap.colors.data());
        _colormap_reupload = false;
    }

    // Navigate
    float new_zoom = _state.navigation.zoom;
    if (_zoom_in || _zoom_out || _shift) {
        if (_zoom_in || _zoom_out) {
            float fx = (_navig_event_x + 0.5f) / width();
            float fy = 1.0f - ((_navig_event_y + 0.5f) / height());
            __float128 zoom_dir = (_zoom_in ? +1 : -1);
            new_zoom = _state.navigation.zoom + zoom_dir * _zoom_step * _state.navigation.zoom;
            __float128 new_xw = _xw / (1.0Q + zoom_dir * _zoom_step);
            __float128 new_yw = _yw / (1.0Q + zoom_dir * _zoom_step);
            __float128 new_x0 = _x0 + fx * (_xw - new_xw);
            __float128 new_y0 = _y0 + fy * (_yw - new_yw);
            _x0 = new_x0;
            _xw = new_xw;
            _y0 = new_y0;
            _yw = new_yw;
        }
        if (_shift) {
            __float128 dx = (_navig_start_x - _navig_event_x) * _xw / width();
            __float128 dy = (_navig_event_y - _navig_start_y) * _yw / height();
            _x0 = _shift_start_x0 + dx;
            _y0 = _shift_start_y0 + dy;
            _shift = false;
        }
        _state.navigation.x = _x0 + 0.5Q * _xw;
        _state.navigation.y = _y0 + 0.5Q * _yw;
        _state.navigation.zoom = new_zoom;
        emit navigate(_state.navigation.x, _state.navigation.y, _state.navigation.zoom);
    }
    __float128 fractal_ar = _state.fractal.mandelbrot.xw / _state.fractal.mandelbrot.yw;
    __float128 viewport_ar = static_cast<__float128>(width()) / height();
    if (viewport_ar >= fractal_ar) {
        _xw = _state.fractal.mandelbrot.xw / _state.navigation.zoom;
        _yw = _xw / viewport_ar;
    } else {
        _yw = _state.fractal.mandelbrot.yw / _state.navigation.zoom;
        _xw = _yw * viewport_ar;
    }
    _x0 = _state.navigation.x - 0.5Q * _xw;
    _y0 = _state.navigation.y - 0.5Q * _yw;

    // Render the fractal into _fractal_tex
    glBindFramebuffer(GL_FRAMEBUFFER, _fractal_fbo);
    _fractal_prg->bind();
    switch (_precision_type) {
    case precision_native_float:
        glUniform1f(_fractal_prg->uniformLocation("x0"), _x0);
        glUniform1f(_fractal_prg->uniformLocation("xw"), _xw);
        glUniform1f(_fractal_prg->uniformLocation("y0"), _y0);
        glUniform1f(_fractal_prg->uniformLocation("yw"), _yw);
        break;
    case precision_native_double:
        glUniform1d(_fractal_prg->uniformLocation("x0"), _x0);
        glUniform1d(_fractal_prg->uniformLocation("xw"), _xw);
        glUniform1d(_fractal_prg->uniformLocation("y0"), _y0);
        glUniform1d(_fractal_prg->uniformLocation("yw"), _yw);
        break;
    case precision_emu_doublefloat:
        {
            float d0, d1;
            float128_to_pair(_x0, &d0, &d1);
            glUniform2f(_fractal_prg->uniformLocation("x0"), d0, d1);
            float128_to_pair(_xw, &d0, &d1);
            glUniform2f(_fractal_prg->uniformLocation("xw"), d0, d1);
            float128_to_pair(_y0, &d0, &d1);
            glUniform2f(_fractal_prg->uniformLocation("y0"), d0, d1);
            float128_to_pair(_yw, &d0, &d1);
            glUniform2f(_fractal_prg->uniformLocation("yw"), d0, d1);
        }
        break;
    case precision_emu_doubledouble:
        {
            double d0, d1;
            float128_to_pair(_x0, &d0, &d1);
            glUniform2d(_fractal_prg->uniformLocation("x0"), d0, d1);
            float128_to_pair(_xw, &d0, &d1);
            glUniform2d(_fractal_prg->uniformLocation("xw"), d0, d1);
            float128_to_pair(_y0, &d0, &d1);
            glUniform2d(_fractal_prg->uniformLocation("y0"), d0, d1);
            float128_to_pair(_yw, &d0, &d1);
            glUniform2d(_fractal_prg->uniformLocation("yw"), d0, d1);
        }
        break;
    }
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // Display a colored version of _fractal_tex
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
    _coloring_prg->bind();
    glUniform1i(_coloring_prg->uniformLocation("fractal"), 0);
    glUniform1i(_coloring_prg->uniformLocation("colormap"), 1);
    double colormap_offset = _state.colormap.start;
    if (_state.colormap.animation) {
        double animation_offset = (_state.colormap.animation_speed / 60.0) * (animation_nsecs / 1e9);
        animation_offset -= std::floor(animation_offset);
        if (_state.colormap.animation_reverse)
            colormap_offset += animation_offset;
        else
            colormap_offset -= animation_offset;
    }
    if (colormap_offset > 1.0)
        colormap_offset -= 1.0;
    else if (colormap_offset < 0.0)
        colormap_offset += 1.0;
    glUniform1i(_coloring_prg->uniformLocation("reverse"), _state.colormap.reverse ? 1 : 0);
    glUniform1f(_coloring_prg->uniformLocation("offset"), colormap_offset);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _fractal_tex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _colormap_tex);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    if (_zoom_in || _zoom_out || _shift || _state.colormap.animation)
        update();
}

void GLWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

void GLWidget::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
    case Qt::Key_Space:
        {
            State defaults;
            _state.navigation.x = defaults.navigation.x;
            _state.navigation.y = defaults.navigation.y;
            _state.navigation.zoom = defaults.navigation.zoom;
            emit navigate(defaults.navigation.x, defaults.navigation.y, defaults.navigation.zoom);
            update();
        }
        break;
    case Qt::Key_F:
    case Qt::Key_Escape:
        if (isFullScreen()) {
            setWindowFlags(Qt::Widget);
            showNormal();
            setFocus(Qt::OtherFocusReason);
        } else if (event->key() != Qt::Key_Escape) {
            setWindowFlags(Qt::Window);
            showFullScreen();
        }
        break;
    }
}

void GLWidget::mousePressEvent(QMouseEvent* event)
{
    _navig_start_x = _navig_event_x = event->pos().x();
    _navig_start_y = _navig_event_y = event->pos().y();
    if (event->buttons() & Qt::LeftButton) {
        _zoom_in = true;
        _zoom_out = false;
        _shift = false;
    } else if (event->buttons() & Qt::MidButton) {
        _zoom_in = false;
        _zoom_out = false;
        _shift_start_x0 = _x0;
        _shift_start_y0 = _y0;
        _shift = true;
    } else if (event->buttons() & Qt::RightButton) {
        _zoom_in = false;
        _zoom_out = true;
        _shift = false;
    }
    update();
}

void GLWidget::mouseReleaseEvent(QMouseEvent* /* event */)
{
    _zoom_in = false;
    _zoom_out = false;
    _shift = false;
}

void GLWidget::mouseMoveEvent(QMouseEvent* event)
{
    _navig_event_x = event->pos().x();
    _navig_event_y = event->pos().y();
    if (event->buttons() & Qt::MidButton)
        _shift = true;
    update();
}

void GLWidget::wheelEvent(QWheelEvent* /* event */)
{
}
