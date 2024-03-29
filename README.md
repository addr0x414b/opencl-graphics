<!-- PROJECT LOGO -->
<br />
<div align="center">
  <img src="https://i.postimg.cc/bN2NKjNv/rotating-filled-shaded-3.gif" alt="Logo" width="800">


<h3 align="center">OpenCL Graphics</h3>

  <p align="center">
    3D graphics that work using OpenCL. All major calculations are done on the GPU via OpenCL and draws onto the screen via SDL2.
</div>

## Features

Below shows some key features of the OpenCL Graphics library.

### Movable Camera

<img src="https://i.postimg.cc/4x5stPBK/moving-camera.gif" width=800>

### Triangular Clipping

<img src="https://i.postimg.cc/9Q53QQqF/triangular-clipping.gif" width=800>

### Multiple Drawing Modes

<img src="https://i.postimg.cc/HxcDLc3n/multiple-drawing-modes.gif" width=800>

### Other Features
* Flat basic lighting

### All major computations performed on the GPU via OpenCL!

<img src="https://encrypted-tbn0.gstatic.com/images?q=tbn:ANd9GcRLLiNsc1PgqcYTHKeG4r5ZTmYszhhOubJ4bg8fxzsF6umNI_HRA_1jCWzdJMmMWdVJ8ow&usqp=CAU" width=200>

## The Basic Steps:

1. Perform a while loop in which the window + graphics will run (SDL2 is used for the actual window)

2. Each iteration, send all necessary numbers to the GPU via a buffer (this is done with OpenCL)

3. Perform all calculations on the GPU in parallel

4. Send back the data in a buffer

5. Draw to the screen on SDL2 via a texture
