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
## Rendering
- `T`: whether to render text information or not. You should disable text rendering when a lot of cells are drawn for performance reasons.
Text rendering will automatically be disabled when you have zoomed out to far.
## Simulation
- `N`: next simulation step (Cellular Automaton-like evolution or sequential step depending on `--seq` flag)