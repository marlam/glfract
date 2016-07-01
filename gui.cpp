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

#include <cstring>
#include <cmath>

#include <QApplication>
#include <QGridLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QLabel>
#include <QRadioButton>
#include <QPushButton>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QMenu>
#include <QMenuBar>
#include <QImage>
#include <QPixmap>
#include <QFileDialog>
#include <QClipboard>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>

#include <quadmath.h>

#include "gui.hpp"
#include "glwidget.hpp"


GUI::GUI() : update_lock(false), state()
{
    setWindowTitle("GL Fractal Explorer");
    setWindowIcon(QIcon(":logo.png"));
    QWidget *widget = new QWidget;
    QGridLayout *layout = new QGridLayout;

    glwidget = new GLWidget;
    layout->addWidget(glwidget, 0, 1, 4, 1);
    layout->setColumnStretch(1, 1);
    layout->setRowStretch(3, 1);

    QGroupBox* fractal_box = new QGroupBox("Mandelbrot Fractal");
    QGridLayout* fractal_box_layout = new QGridLayout;
    fractal_box->setLayout(fractal_box_layout);
    QLabel* mandelbrot_power_label = new QLabel("Power:");
    fractal_box_layout->addWidget(mandelbrot_power_label, 0, 0);
    mandelbrot_power_spinbox = new QSpinBox;
    mandelbrot_power_spinbox->setRange(2, 6);
    mandelbrot_power_spinbox->setSingleStep(1);
    fractal_box_layout->addWidget(mandelbrot_power_spinbox, 0, 1);
    QLabel* mandelbrot_max_iter_label = new QLabel("Iterations:");
    fractal_box_layout->addWidget(mandelbrot_max_iter_label, 1, 0);
    mandelbrot_max_iter_spinbox = new QSpinBox;
    mandelbrot_max_iter_spinbox->setRange(1, 9999);
    mandelbrot_max_iter_spinbox->setSingleStep(1);
    fractal_box_layout->addWidget(mandelbrot_max_iter_spinbox, 1, 1);
    QLabel* mandelbrot_bailout_label = new QLabel("Bailout:");
    fractal_box_layout->addWidget(mandelbrot_bailout_label, 2, 0);
    mandelbrot_bailout_spinbox = new QDoubleSpinBox;
    mandelbrot_bailout_spinbox->setRange(4, 9999);
    mandelbrot_bailout_spinbox->setDecimals(1);
    mandelbrot_bailout_spinbox->setSingleStep(1);
    fractal_box_layout->addWidget(mandelbrot_bailout_spinbox, 2, 1);
    mandelbrot_smooth_checkbox = new QCheckBox("Smoothness");
    fractal_box_layout->addWidget(mandelbrot_smooth_checkbox, 3, 0, 1, 2);
    layout->addWidget(fractal_box, 0, 0);

    QGroupBox* precision_box = new QGroupBox("Precision");
    QGridLayout* precision_box_layout = new QGridLayout;
    precision_box->setLayout(precision_box_layout);
    precision_single_hw_btn = new QRadioButton("Single precision");
    precision_box_layout->addWidget(precision_single_hw_btn, 0, 0);
    precision_double_emu_btn = new QRadioButton("Extended precision (2x single)");
    precision_box_layout->addWidget(precision_double_emu_btn, 1, 0);
    precision_double_hw_btn = new QRadioButton("Double precision");
    precision_box_layout->addWidget(precision_double_hw_btn, 2, 0);
    precision_quad_emu_btn = new QRadioButton("Extended precision (2x double)");
    precision_box_layout->addWidget(precision_quad_emu_btn, 3, 0);
    layout->addWidget(precision_box, 1, 0);

    QGroupBox* colormap_box = new QGroupBox("Color map");
    QGridLayout* colormap_box_layout = new QGridLayout;
    colormap_box->setLayout(colormap_box_layout);
    colormap_label = new QLabel;
    colormap_box_layout->addWidget(colormap_label, 0, 0, 1, 4);
    QPushButton* colormap_from_png_btn = new QPushButton("From PNG");
    connect(colormap_from_png_btn, SIGNAL(clicked(bool)), this, SLOT(colormap_from_png()));
    colormap_box_layout->addWidget(colormap_from_png_btn, 1, 0, 1, 2);
    QPushButton* colormap_from_clipboard_btn = new QPushButton("From clipboard");
    connect(colormap_from_clipboard_btn, SIGNAL(clicked(bool)), this, SLOT(colormap_from_clipboard()));
    colormap_box_layout->addWidget(colormap_from_clipboard_btn, 1, 2, 1, 2);
    colormap_reverse_checkbox = new QCheckBox("Reverse");
    colormap_box_layout->addWidget(colormap_reverse_checkbox, 2, 0, 1, 4);
    QLabel* colormap_start_label = new QLabel("Start:");
    colormap_box_layout->addWidget(colormap_start_label, 3, 0);
    colormap_start_slider = new QSlider(Qt::Horizontal);
    colormap_start_slider->setRange(0, 100);
    colormap_box_layout->addWidget(colormap_start_slider, 3, 1, 1, 3);
    colormap_animation_checkbox = new QCheckBox("Animate");
    colormap_box_layout->addWidget(colormap_animation_checkbox, 4, 0, 1, 2);
    colormap_animation_reverse_checkbox = new QCheckBox("Reverse");
    colormap_box_layout->addWidget(colormap_animation_reverse_checkbox, 4, 2, 1, 2);
    QLabel* colormap_animation_speed_label = new QLabel("Speed:");
    colormap_box_layout->addWidget(colormap_animation_speed_label, 5, 0);
    colormap_animation_speed_slider = new QSlider(Qt::Horizontal);
    colormap_animation_speed_slider->setRange(1, 120);
    colormap_box_layout->addWidget(colormap_animation_speed_slider, 5, 1, 1, 3);
    layout->addWidget(colormap_box, 2, 0);

    widget->setLayout(layout);
    setCentralWidget(widget);

    QMenu* file_menu = menuBar()->addMenu("&File");
    QAction* file_open_act = new QAction("&Open...", this);
    file_open_act->setShortcut(QKeySequence::Open);
    connect(file_open_act, SIGNAL(triggered()), this, SLOT(file_open()));
    file_menu->addAction(file_open_act);
    QAction* file_save_act = new QAction("&Save...", this);
    file_save_act->setShortcut(QKeySequence::Save);
    connect(file_save_act, SIGNAL(triggered()), this, SLOT(file_save()));
    file_menu->addAction(file_save_act);
    file_menu->addSeparator();
    QAction* file_export_png_act = new QAction("&Export as PNG...", this);
    connect(file_export_png_act, SIGNAL(triggered()), this, SLOT(file_export_png()));
    file_menu->addAction(file_export_png_act);
    file_menu->addSeparator();
    QAction* quit_act = new QAction("&Quit...", this);
    quit_act->setShortcut(QKeySequence::Quit);
    connect(quit_act, SIGNAL(triggered()), this, SLOT(close()));
    file_menu->addAction(quit_act);
    QMenu* edit_menu = menuBar()->addMenu("&Edit");
    QAction* edit_copy_act = new QAction("&Copy", this);
    edit_copy_act->setShortcut(QKeySequence::Copy);
    connect(edit_copy_act, SIGNAL(triggered()), this, SLOT(edit_copy()));
    edit_menu->addAction(edit_copy_act);
    QMenu* help_menu = menuBar()->addMenu("&Help");
    QAction* help_about_act = new QAction("&About", this);
    connect(help_about_act, SIGNAL(triggered()), this, SLOT(help_about()));
    help_menu->addAction(help_about_act);
}

GUI::~GUI()
{
}

void GUI::activate()
{
    // force the GL widget to create and validate its GL context, and force all
    // widgets to get a valid size, before calling functions that depend on a sane
    // GUI state
    show();
    state_to_gui();
    connect(mandelbrot_power_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
    connect(mandelbrot_max_iter_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
    connect(mandelbrot_bailout_spinbox, SIGNAL(valueChanged(double)), this, SLOT(update()));
    connect(mandelbrot_smooth_checkbox, SIGNAL(toggled(bool)), this, SLOT(update()));
    connect(precision_single_hw_btn, SIGNAL(toggled(bool)), this, SLOT(update()));
    connect(precision_double_emu_btn, SIGNAL(toggled(bool)), this, SLOT(update()));
    connect(precision_double_hw_btn, SIGNAL(toggled(bool)), this, SLOT(update()));
    connect(precision_quad_emu_btn, SIGNAL(toggled(bool)), this, SLOT(update()));
    connect(colormap_reverse_checkbox, SIGNAL(toggled(bool)), this, SLOT(update()));
    connect(colormap_start_slider, SIGNAL(valueChanged(int)), this, SLOT(update()));
    connect(colormap_animation_checkbox, SIGNAL(toggled(bool)), this, SLOT(update()));
    connect(colormap_animation_reverse_checkbox, SIGNAL(toggled(bool)), this, SLOT(update()));
    connect(colormap_animation_speed_slider, SIGNAL(valueChanged(int)), this, SLOT(update()));
    connect(glwidget, SIGNAL(navigate(__float128, __float128, __float128)), this, SLOT(navigate(__float128, __float128, __float128)));
    update();
    glwidget->setFocus(Qt::OtherFocusReason);
}

void GUI::state_to_gui()
{
    update_lock = true;
    mandelbrot_power_spinbox->setValue(state.fractal.mandelbrot.power);
    mandelbrot_max_iter_spinbox->setValue(state.fractal.mandelbrot.max_iter);
    mandelbrot_bailout_spinbox->setValue(state.fractal.mandelbrot.bailout);
    mandelbrot_smooth_checkbox->setChecked(state.fractal.mandelbrot.smooth);
    switch (state.precision.type) {
    case precision_native_float:
        precision_single_hw_btn->setChecked(true);
        break;
    case precision_native_double:
        precision_double_hw_btn->setChecked(true);
        break;
    case precision_emu_doublefloat:
        precision_double_emu_btn->setChecked(true);
        break;
    case precision_emu_doubledouble:
        precision_quad_emu_btn->setChecked(true);
        break;
    }
    update_colormap_label();
    colormap_reverse_checkbox->setChecked(state.colormap.reverse);
    colormap_start_slider->setValue(state.colormap.start * 100.0f);
    colormap_animation_checkbox->setChecked(state.colormap.animation);
    colormap_animation_reverse_checkbox->setChecked(state.colormap.animation_reverse);
    colormap_animation_speed_slider->setValue(state.colormap.animation_speed);
    update_lock = false;
}

void GUI::gui_to_state()
{
    state.fractal.type = fractal_mandelbrot;
    state.fractal.mandelbrot.power = mandelbrot_power_spinbox->value();
    state.fractal.mandelbrot.max_iter = mandelbrot_max_iter_spinbox->value();
    state.fractal.mandelbrot.bailout = mandelbrot_bailout_spinbox->value();
    state.fractal.mandelbrot.smooth = mandelbrot_smooth_checkbox->isChecked();
    state.precision.type = (precision_single_hw_btn->isChecked() ? precision_native_float
            : precision_double_hw_btn->isChecked() ? precision_native_double
            : precision_double_emu_btn->isChecked() ? precision_emu_doublefloat
            : precision_emu_doubledouble);
    state.colormap.reverse = colormap_reverse_checkbox->isChecked();
    state.colormap.start = colormap_start_slider->value() / 100.0f;
    state.colormap.animation = colormap_animation_checkbox->isChecked();
    state.colormap.animation_reverse = colormap_animation_reverse_checkbox->isChecked();
    state.colormap.animation_speed = colormap_animation_speed_slider->value();
}

void GUI::update_colormap_label()
{
    int w = colormap_label->width();
    int h = colormap_label->height();
    QImage img(w, h, QImage::Format_RGB32);
    QRgb* first_scanline = reinterpret_cast<QRgb*>(img.scanLine(0));
    for (int x = 0; x < w; x++) {
        int i = (state.colormap.colors.size() / 3 - 1) / (w - 1.0f) * x;
        QRgb rgb = QColor(state.colormap.colors[3 * i + 0],
                state.colormap.colors[3 * i + 1],
                state.colormap.colors[3 * i + 2]).rgb();
        first_scanline[x] = rgb;
    }
    for (int y = 1; y < h; y++) {
        QRgb* scanline = reinterpret_cast<QRgb*>(img.scanLine(y));
        std::memcpy(scanline, first_scanline, w * sizeof(QRgb));
    }
    colormap_label->setPixmap(QPixmap::fromImage(img));
}

void GUI::update()
{
    if (!update_lock) {
        gui_to_state();
        glwidget->set_state(state);
    }
}

void GUI::navigate(__float128 x, __float128 y, __float128 zoom)
{
    state.navigation.x = x;
    state.navigation.y = y;
    state.navigation.zoom = zoom;
}

void GUI::colormap_from_img(const QImage& img)
{
    QImage cimg = img.convertToFormat(QImage::Format_RGB32);
    if (cimg.width() >= cimg.height()) {
        state.colormap.colors.resize(3 * cimg.width());
        const QRgb* scanline = reinterpret_cast<const QRgb*>(cimg.scanLine(0));
        for (int i = 0; i < cimg.width(); i++) {
            QRgb color = scanline[i];
            state.colormap.colors[3 * i + 0] = qRed(color);
            state.colormap.colors[3 * i + 1] = qGreen(color);
            state.colormap.colors[3 * i + 2] = qBlue(color);
        }
    } else {
        state.colormap.colors.resize(3 * cimg.height());
        for (int i = 0; i < cimg.height(); i++) {
            const QRgb* scanline = reinterpret_cast<const QRgb*>(cimg.scanLine(i));
            QRgb color = scanline[0];
            state.colormap.colors[3 * i + 0] = qRed(color);
            state.colormap.colors[3 * i + 1] = qGreen(color);
            state.colormap.colors[3 * i + 2] = qBlue(color);
        }
    }
    glwidget->state_has_new_colormap();
    update_colormap_label();
}

void GUI::colormap_from_png()
{
    QString name = QFileDialog::getOpenFileName(this, QString(), QString(),
            "Images (*.png *.jpg);; All files (*)");
    if (!name.isEmpty()) {
        QImage img;
        bool ok = img.load(name);
        if (!ok) {
            QMessageBox::critical(this, "Error", "Cannot load image file");
            return;
        }
        colormap_from_img(img);
        update();
    }
}

void GUI::colormap_from_clipboard()
{
    QImage img = QApplication::clipboard()->image();
    bool ok = false;
    if (!img.isNull()) {
        colormap_from_img(img);
        ok = true;
    } else {
        // try CSV text as copied by gencolormap, https://github.com/marlam/gencolormap
        QStringList lines = QApplication::clipboard()->text().split('\n');
        std::vector<unsigned char> tmp_colors;
        for (int i = 0; i < lines.size(); i++) {
            unsigned short r = 0, g = 0, b = 0;
            QStringList values = lines[i].split(',');
            if (values.size() != 3)
                continue;
            bool rok, gok, bok;
            r = values[0].toUShort(&rok);
            g = values[1].toUShort(&gok);
            b = values[2].toUShort(&bok);
            if (!rok || !gok || !bok || r > 0xff || g > 0xff || b > 0xff)
                continue;
            tmp_colors.push_back(r);
            tmp_colors.push_back(g);
            tmp_colors.push_back(b);
        }
        if (tmp_colors.size() > 0) {
            ok = true;
            state.colormap.colors = tmp_colors;
            glwidget->state_has_new_colormap();
            update_colormap_label();
        }
    }
    if (!ok) {
        QMessageBox::critical(this, "Error", "No usable image in clipboard");
        return;
    }
    update();
}

void GUI::file_open(const QString& file_name)
{
    QString name;
    if (file_name.isEmpty()) {
        name = QFileDialog::getOpenFileName(this, QString(), QString(),
                "Fractals (*.fract);; All files (*)");
    } else {
        name = file_name;
    }
    if (!name.isEmpty()) {
        state.load(name);
        glwidget->state_has_new_colormap();
        state_to_gui();
        update();
    }
}

void GUI::file_save()
{
    QString name = QFileDialog::getSaveFileName(this, QString(), QString(),
            "Fractals (*.fract);; All files (*)");
    if (!name.isEmpty()) {
        state.save(name);
    }
}

void GUI::file_export_png()
{
    QString name = QFileDialog::getSaveFileName(this, QString(), QString(),
            "PNG Images (*.png);; All files (*)");
    if (!name.isEmpty()) {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        QImage img = glwidget->grabFramebuffer();
        img.save(name, "png");
        QApplication::restoreOverrideCursor();
    }
}

void GUI::edit_copy()
{
    QImage img = glwidget->grabFramebuffer();
    QApplication::clipboard()->setImage(img);
}

void GUI::help_about()
{
    QMessageBox::about(this, "About",
                "<p><a href=\"https://github.com/marlam/glfract\">glfract</a> version 0.1</p>"
                "<p>Copyright (C) 2016 Martin Lambers<br>"
                "   This is free software under the terms of the "
                    "<a href=\"http://www.gnu.org/licenses/gpl.html\">GPL version 3</a> or later. "
                "   There is NO WARRANTY, to the extent permitted by law."
                "</p>");
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QString fractal_filename;
    if (argc == 2) {
        fractal_filename = argv[1];
    } else if (argc > 2) {
        fprintf(stderr, "Usage: %s [fractal.fract]\n", argv[0]);
        return 1;
    }

    QSurfaceFormat format;
    format.setVersion(4, 0);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);

    GUI gui;
    gui.activate();
    if (!fractal_filename.isEmpty()) {
        if (QFileInfo(fractal_filename).isReadable()) {
            gui.file_open(fractal_filename);
        } else {
            QMessageBox::critical(&gui, "Error", fractal_filename + " is not readable");
        }
    }
    return app.exec();
}
