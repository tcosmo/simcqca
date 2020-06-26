# What is SimCQCA

`simcqca` is a simulator for the 2D Collatz Quasi Cellular Automaton. Please refer to the paper and the following [Example section](#examples) for more details: ... 

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

Building `simcqca` has been tested on Linux and Mac OS, if it doesn't work for you please feel free to open an [issue](https://github.com/tcosmo/simcqca/issues).

# Examples
<a href="examples"></a>

## Line mode
In Line mode, the input is a binary string. Each successive line corresponds to a new odd iteration of the Collatz process expressed in binary.
- `./simcqca --line 10001010111000110000000000001111111111111111110001111111111111111111001`
- `./simcqca --line 1001011111001000`
## Column mode
In Column mode, the input is a ternary string. Each successive column corresponds to a new iteration of the Collata process expressed in ternary.
- `./simcqca --col 12210000100011100110111112000`
## Border mode
In Border mode, the input is a parity vector. The automaton reconstructs the corresponding input line.
- `./simcqca --border 1100000000000000011111000101011011`
## Cycle mode
In cycle mode, the input is a parity vector supposed to be the support of a cycle. The automaton reconstructs the 3-adic expansion of the corresponding rational cycling over this parity vector. If the option `--cycle-line` is given, the automaton will reconstruct the 2-adic expansion of the same number (rational Collatz cyclic numbers are both 2-adic and 3-adic integers).
- `./simcqca --cycle 1000110`
- `./simcqca --cycle 1000110 --cycle-line`
### Integer cycles
You can try it on the 4 known non-zero integer cycles (1 non-negative and 3 non-positive):
- `./simcqca --cycle 101010` : constructs the cycle (1,2,1,...)
- `./simcqca --cycle 111111` : constructs the cycle (-1,-1,...)
- `./simcqca --cycle 110110` : constructs the cycle (-5,  -7, -10, -5, ...)
- `./simcqca --cycle 11110111000` : constructs the cycle (-17, -25, -37, -55, -82, -41, -61, -91, -136, -68, -34, -17, ...)

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
- `T`: whether to render text information or not. Text rendering is quite efficient (not CPU intensive) in the last versions of `simcqca` even when zoomed out far
- `K`: enables colors for bit-carry-defined cells. One color per bit/carry possibility (0,0), (0,1), (1,0), (1,1)
- `O`: outlines the origin in blue. When you are lost press `C` to center the view on the origin
- `E`: outlines all cells on the edge of the computed world in green. **Warning**: this rendering is not optimized hence potential performance issues if too many cells on the edge (too many being thousands).
## Simulation
- `N`: next simulation step (Cellular Automaton-like evolution or sequential step depending on `--seq` flag)
- `M`: runs simulation step until they are not in view anymore
- `P`: in cycle mode will run the simulation until the period of the 3-adic/2-adic expansion of the cycle is found. Will then output in the console the initial segment and period of that expansion (little endian convention)
- `R`: resets the simulation
## Selectors
In order to visually outline some pattern of your choice you can select cells on the screen:
- `SHIFT + LEFT CLICK`: toggles (i.e. selects if not selected and unselect otherwise) the clicked cell with the current selection color
- `SHIFT + RIGHT/LEFT ARROW`: changes the current selection color (2 colors)
- `SHIFT + LEFT CLICK + MOUSE MOVE`: selects all the cells hovered by the mouse (does not toggle, just selects)
- `SHIFT + RIGHT CLICK`: unselects all the cell sharing the same selection color (if any) as the clicked cell
# Advanced graphic configuration
<a name="advanceConf"></a>
In the file `src/config.h.in` the following constants have an impact on the rendering engine and its CPU/GPU performances. If you modify these values, they will be taken into account at your next `make`:
- `TARGET_FPS`: the frame per seconds rate that is enforced by the engine. Default is 80. Higher rates are more CPU/GPU intensive.   
- `VERTEX_ARRAY_MAX_SIZE`: the number of vertices which are rendered at once by the GPU. Defaulft value is `5*100*100` which is quite conservative. Advanced GPUs should be able to handle a lot more.