#include <stdio.h>
#include "chip8.hh"
// Preparamos el estado del sistema.
// Se empieza limpiando la memoria y reseteando los registros a cero.
// Mientras CHIP-8 no tiene realmete una BIOS o firmware, tiene una fontset básica 
// guardada en la memoria. Esta fontset debe ser cargada a partir de la dirección
// 0x50 == 80.
// Otra cosa importante a recordar es que el sistema espera que la aplicación
// sea cargada en la dirección de memoria 0x200. Esto significa que el pc también 
// debe ser seteado en esta dirección.

void chip8::initialize()
{
    pc = 0x200; // pc empieza en 0x200.
    opcode = 0; // Resetea el opcode actual.
    I = 0;      // Resetea el index register.
    sp = 0;     // Resetea el stack pointer.

    // Borramos el display.
    for (int i = 0; i < 64*32; i++)
        gfx[i] = 0;
    
    // Borramos el stack.
    for (int i = 0; i < 16; i++)
        stack[i] = 0;

    // Borramos la memoria.
    for (int i = 0; i < 4096; i++)
        memory[i] = 0;
    
    // Borramos los registros.
    for (int i = 0; i < 16; i++)
        V[i] = 0;
    
    // Cargamos el fontset.
    for (int i = 0; i < 80; ++i)
        memory[i] = chip8_fontset[i];
   
    // Reseteamos los timers.
    delay_timer = 0;
    sound_timer = 0;

    return;

}

void chip8::emulateCycle()
{
    // Fetch opcode.
    // Durante este paso, el sistema realizará un fetch de una opcode de la memoria
    // que se encuentra en la dirección apuntada por el contador del programa (pc).
    // En nuestro emulador CHIP-8, los datos son guardados en un array en el cual
    // cada dirección contiene un byte. Como un opcode tiene un largo de 2 bytes,
    // necesitaremos hacer fetch de 2 bytes sucesivos y combinarlos para 
    // obtener la opcode actual.

    opcode = memory[pc] << 8 || memory[pc + 1];

    // Decode opcode.
    // Ya habiendo guardado nuestra opcode actual, necesitaremos decodificarla
    // y checkear la tabla de opcodes para ver que significa.
    
    // Leemos los primeros 4 bits del opcode para ver cual es.
    // Los 12 restantes serian el "argumento".
    // opcode & 1111000000000000
    switch (opcode & 0xF000) 
    {
        case 0x0000: 
            switch (opcode)
            {
                case 0x00E0:
                    // Borramos el display.
                    for (int i = 0; i < 64*32; i++)
                    gfx[i] = 0;
                break;
                
                case 0x00EE:
                    // Return
                break;

                default:
                    // Call machine code routine at addr NNN.
            }
        break;
        
        case 0x1000:
            // goto NNN.
            pc = opcode & 0x0FFF;
        break;
        
        case 0x2000:
            // Llama a la subrutina en que esta en la addr. NNN.
            stack[sp] = pc;
            ++sp;
            pc = opcode & 0x0FFF;
        break;

        case 0x3000:
            // if (Vx == NN) skipea la siguiente instrucción.
            if ((opcode & 0x00FF) == V[(opcode & 0x0F00)])
                pc += 4;
        break;

        case 0x4000:
            // If (Vx != NN) skipea la siguiente instrucción.
            if ((opcode & 0x00FF) != V[(opcode & 0x0F00)])
                pc += 4; 
        break;

        case 0x5000:
            // If (Vx == Vy) skipea la siguiente instrucción.
            if (V[(opcode & 0x0F00)] == V[(opcode & 0x00F0)])
                pc += 4;  
        break;

        case 0x6000:
            // Setea Vx a NN.
            V[(opcode & 0x0F00)] = opcode & 0x00FF;
        break;

        case 0x7000:
            // Añade NN a Vx (No cambia la Carry Flag).
            V[(opcode & 0x0F00)] += opcode & 0x00FF;
        break;

        case 0x8000:
            switch (opcode & 0x000F)
            {
                case 0x0000:
                    // Setea Vx = Vy
                    V[(opcode & 0x0F00)] = V[(opcode & 0x00F0)];
                break;

                case 0x0001:
                    // Vx |= Vy
                    V[(opcode & 0x0F00)] |= V[(opcode & 0x00F0)];
                break;

                case 0x0002:
                    // Vx &= Vy
                    V[(opcode & 0x0F00)] &= V[(opcode & 0x00F0)];
                break;

                case 0x0003:
                    // Vx ^= Vy 
                    V[(opcode & 0x0F00)] ^= V[(opcode & 0x00F0)];
                break;

                case 0x0004:
                    // Vx += Vy <- Si hay overflow se setea a 1 VF, a 0 si no.
                break;

                case 0x0005:
                    // Vx -= Vy <- Se setea VF a 0 si hay underflow y a 1 si no.
                break;

                case 0x0006:
                    // Vx >>= 1 <- Guarda el bit menos significativo de Vx previo al
                    // shift en VF.
                break;

                case 0x0007:
                    // Vx = Vy - Vx <- Se setea VF a 0 si hay underflow y a 1 si no.
                break;

                case 0x000E:
                    // Vx <<= 1 <- Setea VF a 1 si el bst de VX previo al shift estaba 
                    // seteado. A 0 si no estaba.
                break;
                
                default:
                    printf ("Unknown opcode: 0x%X\n", opcode);
            }
        break;

        case 0x9000:
            // if (Vx != Vy) skipea le siguiente instrucción.
            if (V[(opcode & 0x0F00)] != V[(opcode & 0x00F0)])
                pc += 4;  
        break;

        case 0xA000:
            //ANNN: Sets I to the address NNN
            // Execute opcode
            I = opcode & 0x0FFF;
            pc += 2;
        break;

        case 0xB000:
            // Salta a la dirección NNN + V0.
            pc += opcode & 0x0FFF + V[0];
        break;

        case 0xC000:
            // VX = rand() & NN 
        break;

        case 0xD000:
            // Dibuja en las coordenadas dadas.
        break;

        case 0xE000:
            switch (opcode & 0x00FF)
            {
                case 0x009E:
                    // if (key() == Vx) skipea la próxima instrucción (es si se presiona
                    // la tecla que esta guardada en Vx.
                break;

                case 0x00A1:
                    // if (key() != Vx) skipea la próxima instrucción (si la tecla
                    // guardada en Vx no es presionada).
                break;

                default:
                    printf ("Unknown opcode: 0x%X\n", opcode);
            } 
        break;
        
        case 0xF000:
            switch (opcode & 0x00FF){
                case 0x0007:
                    // Vx = delay_timer value;
                break;

                case 0x000A:
                    // Espera por una tecla presionada y la almacena.
                break;

                case 0x0015:
                    // delay_timer = Vx;
                break;

                case 0x0018:
                    // sound_timer = Vx;
                break;

                case 0x001E:
                    // Indice = Indice + VX;
                break;

                case 0x0029:
                    // I = VX * largo Sprite Chip-8
                break;

                case 0x0033:
                    // Guarda la representacion de Vx en formato humano.
                    // poniendo las centenas en la posición de memoria I,
                    // las decenas en I + 1 y
                    // las unidades en I + 2;
                break;

                case 0x0055:
                    // Almacena el contenido de V0 a VX en la memoria empezando
                    // por la dirección I.
                break;

                case 0x0065:
                    // Almacena el contenido de la dirección de memoria I
                    // en los registros del V0 al Vx.
                break;

                default:
                    printf ("Unknown opcode: 0x%X\n", opcode);
        }
        break;

        default:
            printf ("Unknown opcode: 0x%X\n", opcode);
    }


}

