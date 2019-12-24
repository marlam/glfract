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
  * Mesa 19.3.1 llvmpipe and i965 on Intel Kaby Lake
  * Mesa 18.1.3 llvmpipe and softpipe
  * Mesa 11.0 i965 on Sandybridge Mobile
  * Mesa 11.0 llvmpipe
* Known not to work:
  * Mesa 18.1.3 i965 on Intel Kaby Lake (strange artifacts)
  * Mesa 11.0 i965 on Bay Trail (strange artifacts)
  * NVIDIA proprietary drivers, any version (no additional precision; apparently
    the driver ignores the `precise` keyword from `GL_ARB_gpu_shader5`)

Coloring is based on user-defined color maps (e.g. created with 
[gencolormap](https://marlam.de/gencolormap)) and can be animated.

![GUI screen shot](https://git.marlam.de/gitweb/?p=glfract.git;a=blob_plain;f=screenshot.png;hb=HEAD)
