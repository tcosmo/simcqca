<p align="center">
<img src="https://github.com/tcosmo/simcqca/blob/master/base_conversion.png?raw=true" align="center" width="70%" style="text-align:center" alt="The Collatz process embeds a base conversion algorithm"></img>
<br/>
<strong>The Collatz process embeds a base conversion algorithm.</strong>
</p>

# What is SimCQCA

`simcqca` is a simulator for the 2D Collatz Quasi Cellular Automaton. Please refer to the paper and the following [Example section](#examples) for more details and explanations about the above image: ... 

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

## Row mode
In row mode, the input is a binary string. Each successive row corresponds to a new odd iteration of the Collatz process expressed in binary. Right click on a row and the terminal will output the corresponding binary Collatz iterate.

Press `N` to trigger one simulation step and `M` to trigger as many as can fit in the screen. Press `P` to generate enough simulation steps in order to visualise the base conversion property: columns, which are written in base 3, convert to rows, which are written in base 2. Look at you terminal which will output some information about the numbers encoded in the outlined row/column (be careful of 64 bit precision).
- `./simcqca --row 1001011111001000`
- `./simcqca --row 10001010111000110000000000001111111111111111`
## Column mode
In Column mode, the input is a ternary string. Each successive column corresponds to a new iteration of the Collatz process expressed in ternary. Right click on a column and the terminal will output the corresponding ternay Collatz iterates.

- `./simcqca --col 12210000100011100110111112000`
- `./simcqca --col 110012201`
- `./simcqca --col 0001001010010222110010`
## Border mode
In Border mode, the input is a parity vector. The automaton reconstructs the corresponding input row.
- `./simcqca --border 1100000000000000011111000101011011`
- `./simcqca --border 0011000`
## Cycle mode
In cycle mode, the input is a parity vector supposed to be the support of a cycle. The automaton reconstructs the 3-adic expansion of the corresponding rational cycling over this parity vector. If the option `--cycle-row` is given, the automaton will reconstruct the 2-adic expansion of the same number (rational Collatz cyclic numbers are both 2-adic and 3-adic integers). If the option `--cycle-both` is given the automaton will construct them both at the same time.
- `./simcqca --cycle 1000110`
- `./simcqca --cycle 1000110 --cycle-row`
- `./simcqca --cycle 00010101100`
- `./simcqca --cycle 00010101100 --cycle-row`
- `./simcqca --cycle 00010101100 --cycle-both`
### Integer cycles
You can try it on the 4 known non-zero integer cycles (1 non-negative and 3 non-positive):
- `./simcqca --cycle 101010` : constructs the cycle (1,2,1,...)
- `./simcqca --cycle 111111` : constructs the cycle (-1,-1,...)
- `./simcqca --cycle 110110` : constructs the cycle (-5,  -7, -10, -5, ...)
- `./simcqca --cycle 11110111000` : constructs the cycle (-17, -25, -37, -55, -82, -41, -61, -91, -136, -68, -34, -17, ...)

# Controls
## General
- `ESC`: quit
- `A`: outputs some performance information (FPS, vertex array size, etc..)
## Camera

We recommend the use of an **optical mouse** to manage the camera as functions of the trackpad are not supported and keyboard camera controls potentially cumbersome to use.

- `C`: centers the view on the origin
- `CTRL + SHIFT + ARROWS`: translates the view
- `MOUSE WHEEL BUTTON`: translates the view following the mouse
- `CTRL + MOUSE WHEEL UP/DOWN` or `CTRL + A/Z`: zoom in and out
## Rendering
- `T`: whether to render text information or not. Text rendering is quite efficient (not CPU intensive) in the last versions of `simcqca` even when zoomed out far
- `K`: enables colors for bit-carry-defined cells. One color per bit/carry possibility (0,0), (0,1), (1,0), (1,1)
- `O`: outlines the origin in blue. When you are lost press `C` to center the view on the origin
- `E`: outlines all cells on the edge of the computed world in green. 
- `F`: in border and cycle mode outlines the original cells of the parity vector
**Warning**: rendering for modes `E` and `F` are not optimized hence potential performance issues if used when too many cells on the edge/parity vector (too many being thousands).
## Simulation
- `N`: next simulation step (Cellular Automaton-like evolution or sequential step depending on `--seq` flag)
- `M`: runs simulation step until they are not in view anymore
- `R`: resets the simulation
- `RIGHT CLICK`: in row/column mode will finish to compute and blink the clicked row/column and will output in the terminal the number it represents (row are base 2/columns are base 3).
- `P`:    
    - In row/column modes will outline one instance of the base conversion result in the 2D CQCA. The outlined column represents a number in base 3' (most significant trit on top) and thus in base 3 by mapping each cell (bit,carry) as follows: (0,0) maps to the trit 0, (0,1) maps to the trit 1, (1,0) maps to the trit 1 and (1,1) maps to the trit 2. The outlined row represents a number in base 2 (most significant bit to the left) by simply keeping each cell's bit and ignoring carries. Those two numbers are the same. The terminal reads those numbers for you but be careful of the 64 bits precision: if the row gets too big (>64 bits) the output will look like nonsense.
    - In cycle mode, pressing `P` will run the simulation until the period of the 3-adic/2-adic expansion of the cycle is found. Will then output in the console the initial segment and period of that expansion (both little and big endian conventions).
### Specific to cycle mode
- `ALT + LEFT ARROW/RIGHT ARROW`: rotates the input parity vector to the left/right and re-runs the simulation until it is not in view anymore
## Selectors
In order to visually outline some pattern of your choice you can select cells on the screen:
- `SHIFT + LEFT CLICK`: toggles (i.e. selects if not selected and unselect otherwise) the clicked cell with the current selection color
- `SHIFT + RIGHT/LEFT ARROW`: changes the current selection color (2 colors)
- `SHIFT + RIGHT CLICK`: unselects all the cell sharing the same selection color (if any) as the clicked cell
- `CTRL + LEFT CLICK + MOUSE MOVE`: selects all the cells hovered by the mouse (does not toggle, just selects)
- `SHIFT + ALT + LEFT CLICK`: in cycle mode toggles the cell and outlines the underlying parity vector (which sets the equivalence relation on the world)

# Advanced graphic configuration
<a name="advanceConf"></a>
In the file `src/config.h.in` the following constants have an impact on the rendering engine and its CPU/GPU performances. If you modify these values, they will be taken into account at your next `make`:
- `TARGET_FPS`: the frame per seconds rate that is enforced by the engine. Default is 80. Higher rates are more CPU/GPU intensive.   
- `VERTEX_ARRAY_MAX_SIZE`: the number of vertices which are rendered at once by the GPU. Defaulft value is `5*100*100` which is quite conservative. Advanced GPUs should be able to handle a lot more.