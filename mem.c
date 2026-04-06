#include <stdio.h>
#include <assert.h>

typedef unsigned char byte;

typedef unsigned short word;

typedef unsigned short address;

#define MEMSIZE (64 * 1024)

byte mem [MEMSIZE];

void test_mem ();

void b_write (address adr, byte val);

byte b_read (address adr);

word w_read (address adr);

void w_write (address adr, word val);

void load_data ();

void mem_dump(address adr, int size);



int main()
{
    load_data();

    mem_dump(0x40, 20);
    printf("\n");
    mem_dump(0x200, 0x26);

    return 0;
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

void load_data ()
{
    FILE* fp = stdin;

    unsigned int block_adr;
    unsigned int block_size;
    unsigned int value;

    while (2 == scanf("%x%x", &block_adr, &block_size))
    {
        for (unsigned int counter = 0; counter < block_size; counter++)
        {
            scanf ("%x", &value);
            b_write(block_adr + counter, value);
        }        
    }
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
    byte yound_byte = 0xb4;   // 0xa4
    byte senjor_byte = 0xa2;

    w_write (a, w);
}











