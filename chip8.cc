#include <stdio.h>
#include <stdlib.h>
#include <time.h>
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

    // Funcion para utilizar el tiempo de la pc como semilla de 
    // números pseudorandom.
    srand(time(NULL));
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
        case 0x0000:{ 
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
                    }
        case 0x1000:{
            // goto NNN.
            pc = opcode & 0x0FFF;
        break;}
        
        case 0x2000:{
            // Llama a la subrutina que esta en la addr. NNN.
            stack[sp] = pc; // Guardo la direc. actual.
            ++sp; // Aumento el sp para "espacio en stack"
            pc = opcode & 0x0FFF; // Muevo el pc a la subrutina indicada
        break;}

        case 0x3000:{
            // if (Vx == NN) skipea la siguiente instrucción.
            if ((opcode & 0x00FF) == V[(opcode & 0x0F00)])
                pc += 4;
        break;}

        case 0x4000:{
            // If (Vx != NN) skipea la siguiente instrucción.
            if ((opcode & 0x00FF) != V[(opcode & 0x0F00) >> 8])
                pc += 4; 
        break;}

        case 0x5000:{
            // If (Vx == Vy) skipea la siguiente instrucción.
            if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
                pc += 4;  
        break;}

        case 0x6000:{
            // Setea Vx a NN.
            V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
        break;}

        case 0x7000:{
            // Añade NN a Vx (No cambia la Carry Flag).
            V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
        break;}

        case 0x8000:{
            switch (opcode & 0x000F)
            {
                case 0x0000:{
                    // Setea Vx = Vy
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
                break;}

                case 0x0001:{
                    // Vx |= Vy
                    V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
                break;}

                case 0x0002:{
                    // Vx &= Vy
                    V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
                break;}

                case 0x0003:{
                    // Vx ^= Vy 
                    V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
                break;}

                case 0x0004:{
                    // Vx += Vy <- Si hay overflow se setea a 1 VF, a 0 si no.
                    // Checkea si la suma de VX + VY > 255 (11111111) ya que los registros
                    // son de 1 byte por lo tanto solo pueden guardar 8 bits. 
                    // Si la suma es mayor a 255 se pondría en 0 por overflow.
                    // La condicion es: Vx + Vy > 255 (0xFF) => Vy > 255 (0xFF) - Vx
                    if(V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]))
                      V[0xF] = 1; // carry
                    else
                      V[0xF] = 0;
                    V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                break;}

                case 0x0005:{
                    // Vx -= Vy <- Se setea VF a 0 si hay underflow y a 1 si no.
                    // El underflow se produce cuando Vx - Vy < 0 => Vy > Vx
                    if(V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8])
                      V[0xF] = 0; // underflow
                    else
                      V[0xF] = 1;
                    V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                break;}

                case 0x0006:{
                    // Vx >>= 1 <- Guarda el bit menos significativo de Vx previo al
                    // shift en VF.
                    V[0xF] &= 0;
                    V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x0001;
                    V[(opcode & 0x0F00) >> 8] >>= 1;
                break;}

                case 0x0007:{
                    // Vx = Vy - Vx <- Se setea VF a 0 si hay underflow y a 1 si no.
                    if(V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8])
                      V[0xF] = 0; // underflow
                    else
                      V[0xF] = 1;
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                break;}

                case 0x000E:{
                    // Vx <<= 1 <- Setea VF a 1 si el bst de VX previo al shift estaba 
                    // seteado. A 0 si no estaba.
                    if(V[(opcode & 0x0F00) >> 8] & 0x8000)
                        V[0xF] = 1;
                    else 
                        V[0xF] = 0;
                break;}
                
                default:
                    printf ("Unknown opcode: 0x%X\n", opcode);
            }
        break;}

        case 0x9000:{
            // if (Vx != Vy) skipea le siguiente instrucción.
            if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
                pc += 4;  
        break;}

        case 0xA000:{
            // ANNN: Sets I to the address NNN
            // Execute opcode
            I = opcode & 0x0FFF;
            pc += 2;
        break;}

        case 0xB000:{
            // Salta a la dirección NNN + V0.
            pc += opcode & 0x0FFF + V[0];
        break;}

        case 0xC000:{
            // VX = rand() & NN 
            V[(opcode & 0x0F00) >> 8] = (rand() % 255) & (opcode & 0x00FF);
        break;}

        case 0xD000:{
            // Dibuja en las coordenadas dadas.
	          // Dibuja un sprite en las coordenadas (Vx, Vy) 
            // que tiene un ancho de 8 pixeles 
	          // y una altura de N pixeles. 
            // Cada fila de 8 pixeles es leída codificada en bits
	          // empezando de la dirección de memoria I; 
            // El valor de I no cambia luego de la 
	          // ejecución de esta instrucción. 
            // Como se describió anteriormente, VF es seteado 
	          // a 1 si algun píxel en la pantalla es cambiado de set a unset 
            // cuando el spirte
	          // es dibujado, y a 0 si eso no pasa.

	          // El estado de cada píxel es seteado usando una operación XOR bit a bit.
	          // Esto significa que compararemos el estado actual del píxel con el valor
	          // actual en la memoria. Si el estado actual es diferente del valor en
	          // la memoria, el valor del bit será 1. Si ambos valores coinciden, el valor
	          // del bit será 0.
            
            unsigned short x = V[(opcode & 0x0F00) >> 8];
            unsigned short y = V[(opcode & 0x00F0) >> 4];
            unsigned short height = opcode & 0x000F;
            unsigned short pixel;

            V[0xF] = 0;
            for (int yline = 0; yline < height; yline++)
            {
                pixel = memory[I + yline];
                for (int xline = 0; xline < 8; xline++)
                {
                    if ((pixel & (0x80 >> xline)) != 0)
                    {
                        if (gfx[(x + xline + ((y + yline) * 64))] == 1)
                            V[0xF] = 1;
                        gfx[(x + xline + ((y + yline) * 64))] ^= 1;
                    }
                }
            }
            drawFlag = true;
            pc +=2;
        break;}

        case 0xE000:{
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
        break;}
        
        case 0xF000:{
            switch (opcode & 0x00FF){
                case 0x0007:{
                    // Vx = delay_timer value;
                    V[opcode & 0x0F00 >> 8] = delay_timer;
                break;}

                case 0x000A:{
                    // Espera por una tecla presionada y la almacena.
                break;}

                case 0x0015:{
                    // delay_timer = Vx;
                    delay_timer = V[opcode & 0x0F00 >> 8];
                break;}

                case 0x0018:{
                    // sound_timer = Vx;
                    sound_timer = V[opcode & 0x0F00 >> 8];
                break;}

                case 0x001E:{
                    // Indice = Indice + VX;
                    I = I + V[opcode & 0x0F00 >> 8];
                break;}

                case 0x0029:{
                    // I = VX * largo Sprite Chip-8
                break;}

                case 0x0033:{
                    // Guarda la representacion de Vx en formato humano.
                    // poniendo las centenas en la posición de memoria I,
                    // las decenas en I + 1 y
                    // las unidades en I + 2;
                    memory[I]     = V[(opcode & 0x0F00) >> 8] / 100;
                    memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
                    memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
                    pc += 2;

                break;}

                case 0x0055:{
                    // Almacena el contenido de V0 a VX en la memoria empezando
                    // por la dirección I.
                    for (int i = 0; i < (opcode & 0x0F00 >> 8); i++)
                        memory[I + i] = V[opcode & 0x0F00 >> 8];

                break;}

                case 0x0065:{
                    // Almacena el contenido de la dirección de memoria I
                    // en los registros del V0 al Vx.
                    for (int i = 0; i < (opcode & 0x0F00 >> 8); i++)
                        V[opcode & 0x0F00 >> 8] = memory[I + i];
                break;}

                default:
                    printf ("Unknown opcode: 0x%X\n", opcode);
        }
        break;}

        default:
            printf ("Unknown opcode: 0x%X\n", opcode);
    }


}

