# Build SimCQCA

You need to install the graphic library `SFML` (>=2.5) in order to build `simcqca` as well as the `cmake` toolchain.              
All the necessary information is here: [https://www.sfml-dev.org/download.php](https://www.sfml-dev.org/download.php).   
You can configure some parameters to adapt the rendering engine to your CPU/GPU configuration, see [Advanced graphic configuration](#advanceConf). 

To build `simcqca` do:

1. `git clone https://github.com/tcosmo/simcqca.git`
2. `cd simcqca`
3. `mkdir build`
4. `cd build`
5. `cmake ..`
6. `make`
7. You are good to go! Look at the examples and controls to get started!

# Examples
- `./simcqca --line 10001010111000110000000000001111111111111111110001111111111111111111001`
- `./simcqca --col 12210000100011100110111112000`

# Controls
## General
- `ESC`: quit
- `F`: outputs some performance information (FPS, vertex array size, etc..)
## Camera
- `C`: centers the view on the origin
- `CTRL + ARROWS`: translates the view
- `MOUSE WHEEL BUTTON`: translates the view following the mouse
- `CTRL + MOUSE WHEEL UP/DOWN` or `CTRL + A/Z`: zoom in and out
## Rendering
- `T`: whether to render text information or not. Text rendering is quite efficient (not CPU intensive) in the last versions of `simcqca` even when zoomed out far.
- `K`: enables colors for bit-carry-defined cells. One color per bit/carry possibility (0,0), (0,1), (1,0), (1,1).
## Simulation
- `N`: next simulation step (Cellular Automaton-like evolution or sequential step depending on `--seq` flag)
- `M`: runs simulation step until they are not in view anymore
# Advanced graphic configuration
<a name="advanceConf"></a>
In the file `src/config.h.in` the following constants have an impact on the rendering engine and its CPU/GPU performances. If you modify these values, they will be taken into account at your next `make`:
- `TARGET_FPS`: the frame per seconds rate that is enforced by the engine. Default is 80. Higher rates are more CPU/GPU intensive.    
- `VERTEX_ARRAY_MAX_SIZE`: the number of vertices which are rendered at once by the GPU. Defaulft value is `5*100*100` which is quite conservative. Advanced GPUs should be able to handle a lot more.