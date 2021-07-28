// #include 
// #include   // OpenGL graphics and input
// #include "chip8.h" // Your cpu core implementation, handles opcodes */
#include <iostream>
#include <fstream>

#include "chip8.h"
#include "setup.cpp"
using namespace std;

//chip8 myChip8;

int main(int argc, char **argv){

    chip8 myChip8;

    //setupGraphics();
    //setupInput();

    /* Initialize the Chip8 system , load game into memory */
    myChip8.initialize(); /* Clears memory, registers, and screen */

    string rom_file;
    cin >> rom_file;
    myChip8.loadGame(rom_file); /* Copy program into the memory */
    cin >> rom_file;

    // /* Emulation Loop */
    // for{;;}{
    //     /* Emulates one cycle */
    //     myChip8.emulateCycle();

    //     /* If draw flag set(Clear screen or draw sprite opcode), update screen(System does not draw every cycle) */
    //     if(myChip8.drawFlag){
    //         drawGraphics();
    //     }

    //     /* Store the key press state (Press and release ) */
    //     myChip8.setKeys();
    // }

    return 0;
}