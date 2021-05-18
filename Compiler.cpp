#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main (int argc, char** argv)
{
    const size_t buf_size = 256;
    char frontend_call[buf_size] = "./Frontend.out ";
    strcat (frontend_call, (argc > 1) ? argv[1] : "Examples/SqrtTripleDick.gcm");
    
    printf ("Calling frontend...\n");
    system (frontend_call);

    printf ("Calling backend...\n");
    system ("./Backend.out");

    printf ("Compiling was ended!\n");
    return 0;
}
