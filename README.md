# Build SimCQCA

You need to install the graphic library `SFML` (>=2.5) in order to build `simcqca` as well as the `cmake` toolchain.              
All the necessary information is here: [https://www.sfml-dev.org/download.php](https://www.sfml-dev.org/download.php).    

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