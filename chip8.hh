class chip8
{

private:
    // CHIP-8 tiene 35 opcodes, cada uno de 2 bytes.
    unsigned short opcode;
    
    // CHIP-8 tiene 4KB de memoria.
    unsigned char memory[4096];
    
    // CHIP-8 tiene 15 registros de propósito general de 8 bits: V0 - VE.
    // El 16avo (VF) es usado como "carry flag".
    unsigned char V[16];
    
    // Hay un registro índice I y un program counter PC el cual puede 
    // tener valores desde 0x000 hasta 0xFFF.
    // El registro índice es un registro que almacena direcciones de memoria.
    // durante la ejecucuión del programa.
    unsigned short I;
    unsigned short pc; 
    
    // El mapa de memoria del sistema:
    // 0x000-0x1FF - CHIP-8 interpreter (contains font set in emu). <- font set es el conjunto de caracteres disponibles para mostrar.
    // 0x050-0x0A0 - Usado para la construcción de los font set en 4x5 pixeles (0-F).
    // 0x200-0xFFF - Programa ROM y RAM funcional.
    
    // El sistema gráfico: CHIP-8 tiene una instrucción que dibuja los sprites en la pantalla.
    // El dibujado se hace en modod XOR y si un pixel se apaga como resultado de un dibujo, el registro
    // VF se prende. Esto se usa para detectar colisiones.
    
    // Los gráficos de CHIP-8 son blancos y negros y la pantalla tiene un total de 2048 píxeles (64 * 32).
    // Lo podemos representar como un array que mantenga el estado del pixel (1 o 0).
    unsigned char gfx[64 * 32];
    
    // CHIP-8 no tiene interrupciones ni registros de hardware, pero tiene dos registros timer que cuentan
    // a 60 Hz. Cuando se setean arriba de cero, contarán hasta cero.
    unsigned char delay_timer;
    unsigned char sound_timer;
    
    // El buzzer del sistema sonara cuando el sound_timer llegue a cero.
    
    // Es importante recordar que el set de instrucciones de CHIP-8 tiene opcodes que permiten al programa
    // saltar a direcciones específicas o llamar a alguna subrutina. Mientras la especificación no menciona un stack,
    // será necesario implementar uno como parte del intérprete. El stack es utilizado para recordar la 
    // localización actual antes que se realice un salto. En cualquier momento que se haga un salto o llamado a una 
    // subrutina, se debe guardar el pc en el stack antes de proceder. El sistema tiene 16 niveles de stack y con el
    // objetivo de recordar cual nivel del stack es usado, se debe implementar un stack pointer (sp).
    unsigned short stack[16];
    unsigned short sp;
    
    // Finalmente, CHIP-8 posee un teclado basado en HEXA (0x0-0xF). Se puede usar un array para 
    // guardar el estado actual de la tecla.
    unsigned char key[16];

    // CHIP-8 font set. Cada número o carácter tiene 4 px de ancho y 5 px de alto.
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

    // Puede parecer un array de números random, pero observemos lo siguiente:
    
    // DEC   HEX    BIN         RESULT    DEC   HEX    BIN         RESULT
    // 240   0xF0   1111 0000    ****     240   0xF0   1111 0000    ****
    // 144   0x90   1001 0000    *  *      16   0x10   0001 0000       *
    // 144   0x90   1001 0000    *  *      32   0x20   0010 0000      *
    // 144   0x90   1001 0000    *  *      64   0x40   0100 0000     *
    // 240   0xF0   1111 0000    ****      64   0x40   0100 0000     *

    // En el ejemplo de la izquierda estamos dibujando el caracter 0. 
    // Podemos ver que consiste de 5 valores. De cada valor, usamos su representación
    // binaria para dibujar.
    // Notemos también que solo los primeros 4 bits (nibble) son usados para dibujar
    // un número o un carácter.

public:
    
    // Ciclo de emulación
    // En cada ciclo, el método emulateCycle es llamado el cual emulará un ciclo
    // del CPU del CHIP-8. Durante este ciclo, el emulador realizará una secuencia
    // fetch-decode-execute sobre una opcode.

    // Inicializa los registros y la memoria por primera vez.
    void initialize();
    
    // Ciclo de emulación: fetch/decode/update-timers.
    void emulateCycle();


};





