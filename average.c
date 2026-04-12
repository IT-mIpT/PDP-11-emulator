#include <stdarg.h>
#include <stdio.h>


float average (int num, ...)
{
    va_list ptr;
    
    va_start (ptr, num);
    int total_sum = 0;

    for (int counter = 0; counter < num; counter++)
    {
        total_sum += va_arg (ptr, int);
    }

    va_end (ptr);

    return (float)total_sum / num;
}

int main ()
{
    printf ("%f\n",average (4, 1, 2, 3, 4));
}
