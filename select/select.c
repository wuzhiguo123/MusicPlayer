#include "select.h"
#include <stdio.h>



void InitSelect()
{
    FD_ZERO(&READSET);
    FD_SET(0, &READSET);
    printf("SELECT INIT SUCCESS!\n");
}