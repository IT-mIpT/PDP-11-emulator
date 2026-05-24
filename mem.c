#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

typedef unsigned char byte;

typedef unsigned short word;

typedef unsigned short address;

#define MEMSIZE (64 * 1024)
#define ostat 0177564
#define odata 0177566

byte mem [MEMSIZE];

word reg [8];

byte NZVC;

#define Z 2
#define N 3
#define C 0
#define V 1

void do_CLx (int x);
void do_SEx (int x);
void do_CCC ();
void do_SCC ();

void do_br ();
void do_beq ();
void do_bpl ();
void do_tst ();
//void do_tst ();

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
void do_clear ();


void run ();
void output ();

struct Argument
{
    word val;
    address adr;
};

typedef struct Argument Arg;

Arg ss;
Arg dd;

Arg get_mr (word w);

word nn;
word regist;
byte b;
char xx;

#define NO_ARGS 0
#define HAS_SS 1
#define HAS_DD (1 << 1)
#define HAS_NN (1 << 2)
#define HAS_R (1 << 3)
#define HAS_B (1 << 4)
#define HAS_XX (1 << 5)

struct Command
{
    unsigned short opcode;
    unsigned short mask;
    const char* name;
    void (*func_ptr)(void);
    byte args;  // has ss, dd, xx, nn, etc    
};

struct Command commands [] =       {{0060000, 0170000, "ADD", do_add, HAS_SS | HAS_DD}, 
                                    {0010000, 0070000, "MOV", do_mov, HAS_SS | HAS_DD | HAS_B},
                                    {0000000, 0177777, "HALT", do_halt, NO_ARGS},
                                    {0005200, 0177700, "INC", do_inc, HAS_DD},
                                    {0077000, 0177000, "SOB", do_sob, HAS_R | HAS_NN},
                                    {0005000, 0177700, "CLR", do_clear, HAS_DD},
                                    {0000277, 0177777, "SCC", do_SCC, NO_ARGS},
                                    {0000257, 0177777, "CCC", do_CCC, NO_ARGS},
                                    {0000400, 0177400, "BR", do_br, HAS_XX},
                                    {01400, 0177400, "BEQ", do_beq, HAS_XX},
                                    {0005700, 0077700,"TST", do_tst, HAS_DD | HAS_B},
                                    {0, 0, "BPL", do_bpl, HAS_XX},
                                    {0000000, 0000000, "unknown", do_halt, NO_ARGS} 
                                };



int main()
{
    b_write (ostat, 0xFF);
    load_data("data_input_sob.txt");
    run ();
}


void b_write (address adr, byte val)
{
    if (adr <= 7)
    {
        reg [adr] = val;
        return;
    }
    if (adr == odata)
        fprintf(stderr, "%c", val);
    mem [adr] = val;
}

byte b_read (address adr)
{
    if (adr <= 7)
    {
        return reg [adr];
    }
    return mem [adr];
}

word w_read (address adr)
{
    if (adr <= 7)
    {
        return reg [adr];
    }
    
    assert (~adr & 1);

    word w = 0;
    w = w | mem [adr + 1];
    w = w << 8;
    w = w | mem [adr];

    return w & 0xFFFF;
}

void w_write (address adr, word val)
{
    if (adr <= 7)
    {
        reg [adr] = val;
    }

    mem [adr] = (byte)(val & (0x00FF));
    mem [adr + 1] = (byte)((val & (0xFF00)) >> 8);
}

void load_data (char* file_name)
{
    FILE* fp = fopen (file_name, "r");
    assert (fp != NULL);

    unsigned int block_adr = 0;
    unsigned int block_size = 0;
    unsigned int value = 0;

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

    word w = 0;

    while (1)
    {
        w = w_read (pc);
        printf ("%06o : %06o ", pc, w);
        pc += 2;                            // DEFINE

        for (int counter = 0; counter < sizeof (commands) / sizeof (struct Command); counter++)
        {
            struct Command cmd = commands[counter];
            if ((w & cmd.mask) == cmd.opcode)
            {
                if (cmd.args & HAS_B)
                {
                    b = (w >> 15) & 1;
                }

                if (b == 0)
                    printf ("%s ", cmd.name);
                else
                    printf ("%sb ", cmd.name);

                if (cmd.args & HAS_SS)
                    ss = get_mr (w >> 6);
                if (cmd.args & HAS_DD)
                    dd = get_mr (w);
                
                if (cmd.args & HAS_R)
                {
                    regist = (w >> 6) & 07;
                    printf ("r%d ", regist);
                }

                if (cmd.args & HAS_NN)
                {
                    nn = w & 077;
                    printf ("0%o ", pc - 2 * nn);
                }

                if (cmd.args & HAS_XX)
                {
                    xx = (char)((unsigned char)(w & (0xFF)));
                    printf ("0%o", pc + 2 * xx);
                }

                printf ("\n");
                cmd.func_ptr ();
                break;
            }
        }
        printf("\n");

        b = 0;
        //output ();
        
    }
}

void do_halt ()
{
    printf ("THE END\n");
    printf ("r0: 0%o r1: 0%o r2: 0%o r3: 0%o r4: 0%o r5: 0%o r6: 0%o r7: 0%o\n", reg [0], reg [1], reg [2], reg [3], reg [4], reg [5], reg [6], reg [7]);

    exit (0);
}

void do_nothing() {}

void do_add()
{
    w_write (dd.adr, ss.val + dd.val);
}

void do_mov()
{
    if (b == 0)
        {
            w_write (dd.adr, ss.val);

            if (w_read (dd.adr) == 0)
                do_SEx (Z);
            else
                do_CLx (Z);

            if ((w_read (dd.adr) >> 15) & 1)
                do_SEx (N);
            else
                do_CLx (N);
        }


    if (b == 1)
        {
            b_write (dd.adr, ss.val);
            if (dd.adr < 8)
                reg [dd.adr] = reg [dd.adr] & 0xFF;

            if (b_read (dd.adr) == 0)
                do_SEx (Z);
            else
                do_CLx (Z);

            if ((b_read (dd.adr) >> 15) & 1)
                do_SEx (N);
            else
                do_CLx (N);
        }

    
}

void do_inc ()
{
    w_write (ss.adr, ss.val + 1);
}

void do_sob ()
{
    w_write (regist, reg [regist] - 1);
    if (reg [regist] > 0)
        pc = pc - 2 * nn; 
}

void do_clear ()
{
    w_write (dd.adr, 0);
}

Arg get_mr (word w)
{
    Arg res = {0,0};

    int r = w & 7;
    int m = (w >> 3) & 7;

    switch (m)
    {
        case 0:
            res.adr = r;
            if (b == 0)
                res.val = reg [r];
            if (b == 1)
                res.val = reg [r] & 0xFF;
            printf("r%d ", r);
            break;
        
        case 1:
            res.adr = reg [r];

            if (b == 0)
                res.val = w_read (res.adr);

            if (b == 1)
                res.val = b_read (res.adr);
            
            printf("(r%d) ", r);
            break;

        case 2:
            res.adr = reg [r];

            if (b == 0)
            {
                res.val = w_read (res.adr);
                reg [r] += 2;
            }

            if (b == 1)
            {
                res.val = b_read (res.adr);
                if (r > -1 && r < 6)
                    reg [r] += 1;
                else
                    reg [r] += 2;
            }
            
            if (r == 7)
                printf("#%o ", res.val);
            else
                printf("(r%d)+ ", r);
            break;

        case 3:
            res.adr = w_read (reg [r]);
            res.val = w_read (res.adr);

            w_write (res.adr, res.val + 1);
            reg [r] += 2;


            if (r == 7)
                printf ("@#%o ", res.adr);
            else
                printf ("@(r%d)+ ", r);

            break;

        case 4:
            if (b == 0)
                {
                    reg [r] -= 2;
                    res.adr = reg [r];
                    res.val = w_read (res.adr);
                }

            if (b == 1 && (r > -1 && r < 6))
                {
                    reg [r] -= 1;
                    res.adr = reg [r];
                    res.val = b_read (res.adr);
                }
            else
            {
                reg [r] -= 2;
                res.adr = reg [r];
                res.val = b_read (res.adr);
            }


            res.adr = reg [r];
            res.val = w_read (res.adr);
            printf("-(r%d) ", r);
            break;

        case 5:
            reg [r] -= 2;

            res.adr = w_read (reg [r]);
            res.val = w_read (res.adr);

            w_write (res.adr, res.val + 1);

            printf ("@-(r%d) ", r);
            break; 
         
        default:
            printf ("this mode will be added\n");
            break;
    }

    //printf ("%d %d\n", r, m);

    return res;
}

void do_CLx (int x)
{
    NZVC = NZVC & (0xF - (1 << x));
}

void do_SEx (int x)
{
    NZVC = NZVC | (1 << x);
}

void do_CCC ()
{
    NZVC = 0;
}

void do_SCC ()
{
    NZVC = 0xF;
}

void do_br ()
{
    pc = pc + xx * 2;
}

void do_beq ()
{
    if (NZVC & (1 << Z))
        do_br ();
}

void do_bpl ()
{
    if (!(NZVC & (1 << N)))
        do_br ();
}

void do_tst ()
{
    byte t_byte = 0;

    if (b == 1)
    {
        t_byte = b_read (dd.adr);
       
        do_CLx (C);
        do_CLx (V);

        if (t_byte == 0)
            do_SEx (Z);
        else
            do_CLx (Z);

        if ((t_byte >> 7) == 1)
            do_SEx (N);
        else
            do_CLx (N);
    }
}


