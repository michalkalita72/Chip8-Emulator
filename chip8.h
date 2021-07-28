
#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include <iomanip>

#include <chrono>

/* System Memory MAP
0x000-0x1FF - Chip 8 interpreter (contains font set in emu)
0x050-0x0A0 - Used for the built in 4x5 pixel font set (0-F)
0x200-0xFFF - Program ROM and work RAM
*/ 



#define NNN 0x2F0

class chip8{

    unsigned short pc;
    unsigned short I; /* Index Register */
    unsigned short opcode;

    unsigned short stack[16];
    unsigned short sp;  

    unsigned char V[16];   /* Registers */
    char memory[4096];
    unsigned char gfx[64*32];  /* Display */

    bool drawFlag; /* Indicate whether to draw, 0x00E0, 0xDXYN*/

    unsigned char delay_timer;
    unsigned char sound_timer;

    unsigned char chip8_fontset[80] =
    { 
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    public:
    void initialize();
    void loadGame(std::string filename);
    void disp_clear();
    void emulateCycle();
};

void chip8::initialize(){
    /* Initialize registers and memory once */
    pc     = 0x200;
    opcode = 0;
    I      = 0;
    sp     = 0; 

    // Clear display
    disp_clear();	
    // Clear stack
    for(int i = 0; i < 16; ++i){
        stack[i] = 0;
    }
    // Clear registers V0-VF
    for(int i = 0; i < 16; ++i){
        V[i] = 0;
    }
    // Clear memory
    for(int i = 0; i < 0x1000; ++i){
        memory[i] = 0;
    }

    // Load fontset 
    for(int i = 0; i < 80; ++i){
        memory[0x50 + i] = chip8_fontset[i];
    }
    //Reset Timers
    delay_timer = 0;
    sound_timer = 0;

}

void chip8::loadGame(std::string filename){

    std::ifstream rom(filename, std::ios::binary);

    if(!rom) {
      std::cout << "Cannot open file!" << std::endl;
   }

    //std::streampos begin,end;
    int size;
    if (rom.is_open()) { 
        /* ok, proceed with output */ 
        /* ios::end	offset counted from the end of the stream */
        rom.seekg(0, std::ios_base::end);
        size = rom.tellg();
        rom.seekg(0, std::ios::beg);
        rom.read(&memory[0x200], size);
        //std::cout << "Size: " << size << std::endl;
    }

    if (rom)
      std::cout << "all characters read successfully. READ: " << rom.gcount() << std::endl;
    else
      std::cout << "error: only " << rom.gcount() << " could be read" << std::endl;
    rom.close();
    if(!rom.good()) {
        std::cout << "Error occurred at reading time!" << std::endl;
    }

    for(int i = 0; i < size; ++i){
        std::cout << std::hex << std::setw(2) << (int)(memory[0x200 + i] & 0xFF) << " ";
        if(i % 16 == 0 & i != 0){
            std::cout << std::endl;
        }
    }
    
    return;
}

void chip8::disp_clear(){
    // set the draw flag in the 0x00E0 Opcode
    for(int i = 0; i < 64*32; ++i){
        gfx[i] = 0;
    }
}

void chip8::emulateCycle(){
    /* Fetch Opcode */
    opcode = memory[pc] << 8 | memory[pc + 1];

    /* Decode Opcode */
    switch(opcode & 0xF000){

        case 0x0000: // 0x00E0 clears the screen
            switch(opcode & 0x000F){
                case 0x0000: // 0x00E0: Clears the screen
                    drawFlag = true;
                    disp_clear();
                break;

                case 0x000E: // 0x00EE: Returns from subroutine
                    // Execute opcode
                    return;
                break;

                default:
                    printf("Unknown opcode [0x0000]: 0x%X\n", opcode);
            }
        break;

        case 0x1000: // 0x1NNN: Jumps to address NNN
            stack[sp] = pc;
            pc = opcode & 0x0FFF;
        break;

        // Fill in Opcodes
        case 0x2000:  // 0x2NNN: calls subroutine at address NNN
            stack[sp] = pc;
            ++sp;
            pc = opcode & 0x0FFF;
        break;

        case 0x0004: // 0x8XY4: adds value of VY to VX, VF is set to 1 if carry bit, 0 if not
            if(V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]))
                V[0xF] = 1; //carry
            else
                V[0xF] = 0;
            V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
            pc += 2;
        break;


        case 0xA00: // ANNN: Sets I to the address NNN
            I = opcode & 0x0FFF;
            pc += 1;
        break;

        case 0xD000: //0xDXYN: draws sprites onto display
        {
            unsigned short x = V[(opcode & 0x0F00) >> 8];
            unsigned short y = V[(opcode & 0x00F0 >> 4)];
            unsigned short height = opcode & 0x000F;
            unsigned short pixel;

            V[0x0F] = 0;
            /* i is for row, j is for column */
            for(int i = 0; i < height; ++i){
                pixel = memory[I + i];
                for(int j = 0; j < 8; ++j){
                    if(pixel & (0x80 >> i) != 0){ // pixel in sprite is off
                        if(gfx[x + i + ((y + j) * 64)] == 1){
                            V[0xF] = 1;
                        }
                        gfx[x + i + ((y+j)*64)] ^= 1; // change the pixel
                    }
                }
            }
            drawFlag = true;
            pc += 2;
        }
        break;

        case 0x0033: //0xFX33: stores binary version of VX at the addresss I, I+1, and I+2
            memory[I]     = V[(opcode & 0x0F00) >> 8] / 100;
            memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
            memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
            pc += 2;
        break;

        default:
            printf("Unknown opcode: 0x%X\n", opcode);
    }

    // Updata Timers
    if(delay_timer > 0)
        --delay_timer;

    if(sound_timer > 0){
        if(sound_timer == 1)
            printf("BEEP!\n");
        --sound_timer;
    }
    /* Execute Opcode */

    /* Update Timers */
}