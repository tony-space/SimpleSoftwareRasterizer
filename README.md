# Simple Software Rasterizer
It's an exercise to consolidate knowledge of homogeneous coordinates and rasterization algorithms.

This project is not intended to be the fastest and the best software renderer ever. If you need one please consider using [OpenSWR](https://www.openswr.org/) or [WARP](https://docs.microsoft.com/en-us/windows/win32/direct3darticles/directx-warp).

![](screenshots/screenshot1.jpg)
![](screenshots/screenshot2.jpg)

## How to run

There are two ways to do that: 

* Using a standalone command. Run `build-and-run.bat`.
* Using Visual Studio. To generate VS2019 solution run `gen-VS2019-x64.bat`. Then compile and run the app.

## Architecture

There are only two modules:

* `librasterizer` is a static library that does most of the job.
* `host-gdi` is an executable that is in charge to create a window and output the result to it.

## Supported features

* Clipping in the homogeneous clip space (before perspective division).
* Tiled rasterization.
* Perspective-correct interpolation of vertex attributes.
* Per-pixel lighting via Lambertian BRDF.
* Gamma correction.

### Clipping

The clipping process works with a custom set of planes. The clipper may leave the triangle as is or chop it onto multiple triangles as follows:
![](screenshots/screenshot4.jpg)

The clipper is semi-parallel. It processes mesh triangles in parallel.
However, when it comes to emitting a new triangle, the clipper waits for a spin-lock.

### Tiled rasterization
A naive rasterizer takes a bounding box of a triangle and starts testing pixel coverage inside that box.
In the case of a thin but long triangle, most pixels fail the coverage test, so only a few of them perform rasterization.

Another problem is several triangles trying to cover the same pixel.
A naive rasterizer cannot process these triangles in parallel due to potential data race in depth and color buffers.

![](https://developer.nvidia.com/sites/default/files/akamai/gameworks/images/lifeofatriangle/fermipipeline_raster.png)

Image by Nvidia. [Source](https://developer.nvidia.com/content/life-triangle-nvidias-logical-pipeline).

Tiled rasterization is an optimization that increases the general level of parallelism. All triangles are scheduled to the tiles they cover. After the work assignment, these tiles start working parallel without sharing data, meaning there's no risk of a data race. Under the hood, the tiles are serial machines.

### Gamma correction

Gamma correction is essential when it comes to light computation. It gets done in linear space because the light is essentially linear.
However, monitors are non-linear. The generated image must be additionally post-processed before display.

![](screenshots/screenshot3.jpg)
Left: gamma corrected image. Right: linear image.

## Limitations

* GDI-based. `librasterizer` is OS-agnostic itself. But I had no much time to play with [SDL](https://www.libsdl.org/) or an alternative.
* No antialiasing.
* No mipmap levels.
* No texture filtering.
* After all, it's a simple thing.

## Third-Party libraries

The project uses [GLM library](https://github.com/g-truc/glm).