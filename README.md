# glfract

This is an experimental program to explore Mandelbrot fractal set images.

The fractals are computed on the GPU using one of the following precisions:

- single precision
- extended precision based on 2x single precision
- double precision
- extended precision based on 2x double precision

The main purpose of this program is to test the 2x single or double precision
modes. The 2x single precision mode produces results that are comparable with
double precision mode, but uses only single precision machine instructions and
is therefore often faster (depending on the GPU).

Unforunately the 2x precision modes do not work with all OpenGL implementations.
The main problem seems to be that some drivers do not correctly implement the
`precise` keyword from the `GL_ARB_gpu_shader5` extension.

* Known to work:
  * Mesa 20.1.9 on AMD Radeon RX 5600 XT
  * Mesa 19.3.1 llvmpipe and i965 on Intel Kaby Lake
  * Mesa 18.1.3 llvmpipe and softpipe
  * Mesa 11.0 i965 on Sandybridge Mobile
  * Mesa 11.0 llvmpipe
  * NVIDIA proprietary drivers 440.59
* Known not to work:
  * Mesa 20.1.9 llvmpipe (no additional precision)
  * NVIDIA proprietary drivers older than ca. 440.x
  * Mesa 18.1.3 i965 on Intel Kaby Lake (strange artifacts)
  * Mesa 11.0 i965 on Bay Trail (strange artifacts)

Coloring is based on user-defined color maps (e.g. created with 
[gencolormap](https://marlam.de/gencolormap)) and can be animated.

![GUI screen shot](https://git.marlam.de/gitweb/?p=glfract.git;a=blob_plain;f=screenshot.png;hb=HEAD)
