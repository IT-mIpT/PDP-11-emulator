#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

typedef unsigned char byte;

typedef unsigned short word;

typedef unsigned short address;

#define MEMSIZE (64 * 1024)

byte mem [MEMSIZE];

word reg [8];

#define pc reg [7]

void test_mem ();

void b_write (address adr, byte val);

byte b_read (address adr);

word w_read (address adr);

void w_write (address adr, word val);

void load_data (char* file_name);

void mem_dump(address adr, int size);

void do_halt ();

void do_add ();

void do_mov ();

void do_inc ();

void do_sob ();

void run ();

struct Command
{
    unsigned short opcode;
    unsigned short mask;
    const char* name;
    void (*func_ptr)(void);
};

struct Command commands [] =       {{0060000, 0170000, "ADD\n", do_add}, 
                                    {0010000, 0170000, "MOV\n", do_mov},
                                    {0000000, 0177777, "HALT\n", do_halt},
                                    {0005200, 0177700, "INC\n", do_inc},
                                    {0077000, 0177000, "SOB\n", do_sob}};



int main()
{
    //load_data("data_input.txt");

    //mem_dump(0x40, 20);
    //printf("\n");
    //mem_dump(0x200, 0x26);

    w_write (01000,0060000);

    run ();

    //return 0;
}


void b_write (address adr, byte val)
{
    mem [adr] = val;
}

byte b_read (address adr)
{
    return mem [adr];
}

word w_read (address adr)
{
    assert (~adr & 1);

    word w = 0;
    w = w | mem [adr + 1];
    w = w << 8;
    w = w | mem [adr];

    return w & 0xFFFF;
}

void w_write (address adr, word val)
{
    mem [adr] = (byte)(val & (0x00FF));
    mem [adr + 1] = (byte)((val & (0xFF00)) >> 8);
}

void load_data (char* file_name)
{
    FILE* fp = fopen (file_name, "r");
    assert (fp != NULL);

    unsigned int block_adr;
    unsigned int block_size;
    unsigned int value;

    while (2 == fscanf(fp, "%x%x", &block_adr, &block_size))
    {
        for (unsigned int counter = 0; counter < block_size; counter++)
        {
            fscanf (fp, "%x", &value);
            b_write(block_adr + counter, value);
        }        
    }

    fclose (fp);

}

void mem_dump(address adr, int size)
{
    for (int counter = 0; counter < size; counter += 2)
    {
        word w = w_read(adr + counter);
        printf ("%06o: %06o %04x\n", adr + counter, w, w);
    }
}

void test_mem ()
{
    address a;
    byte b0, b1, b_res;
    word w, w_res;

    //запись и чтение одного байта

    fprintf (stderr, "WRITE THEN READ ONE BYTE\n");

    a = 0;
    b0 = 0x19;
    b_write (a, b0);

    b_res = b_read (a);

    fprintf (stderr, "a = %06o, b0 = %hhx, b_res = %hhx\n", a, b0, b_res);

    // запись и чтение слова

    fprintf (stderr, "WRITE THEN READ ONE WORD\n");
    assert (b_res == b0);

    a = 2;
    w = 0x1234;
    w_write (a, w);

    w_res = w_read (a);
    
    fprintf (stderr, "a = %06o, w = %hx, w_res = %hx\n", a, w, w_res);
    assert (w_res == w);

    //запись слова, чтение младшего байта

    fprintf (stderr, "WRITE WORD THEN READ BYTES");

    a = 4;
    w = 0xa2b4;
    byte young_byte = 0xb4;   // 0xa4
    byte senjor_byte = 0xa2;


    w_write (a, w);

    byte young_res = b_read (a);
    byte senjor_res = b_read (a + 1);

    fprintf (stderr, "a = %06o, young byte = %hhx, res young byte = %hhx\n", a, young_byte, young_res);
    assert (young_byte == young_res);

    fprintf (stderr, "a = %06o, senjor byte = %hhx, res senjor byte = %hhx\n", a, senjor_byte, senjor_res);
    assert (senjor_byte == senjor_res);
}

void run ()
{
    pc = 01000;

    word w;

    while (1)
    {
        w = w_read (pc);
        printf ("%06o : %06o ", pc, w);
        pc += 2;

        for (int counter = 0; counter < sizeof (commands) / sizeof (struct Command); counter++)
        {
            if ((w & (commands [counter]).mask) == (commands [counter]).opcode)
            {
                printf ("%s ", commands [counter].name);
                (commands [counter]).func_ptr ();
            }
        }
        printf("\n");
        
    }
}

void do_halt ()
{
    printf ("THE END\n");
    exit (0);
}

void do_nothing() {}

void do_add() {}

void do_mov() {}

void do_inc () {}

void do_sob () {}









