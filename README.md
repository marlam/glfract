# glfract

This is an experimental program to explore Mandelbrot fractal set images.

The fractals are computed on the GPU using one of the following precisions:
- single precision
- extended precision based on 2x single precision
- double precision
- extended precision based on 2x double precision

The main feature are the 2x single or double precision modes, but unfortunately
these do not work with many OpenGL implementations.
* Known to work:
  * Mesa 11.0 on Intel Sandybridge Mobile
  * Mesa 11.0 llvmpipe (tested with `LIBGL_ALWAYS_SOFTWARE=y`)
* Known not to work:
  * Mesa 11.0 on Intel Bay Trail (strange artifacts)
  * NVIDIA proprietary drivers (no additional precision; apparently the
    `precise` keyword from `GL_ARB_gpu_shader5` is ignored in the fragment
    shader)

Coloring is based on user-defined color maps (e.g. created with 
[gencolormap](https://github.com/marlam/gencolormap)) and can be animated.

![GUI screen shot](https://raw.githubusercontent.com/marlam/glfract/master/screenshot.png)
