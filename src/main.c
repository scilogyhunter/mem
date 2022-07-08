#include <stdio.h>
extern  INT  memAccessCmd (INT  iArgC, PCHAR  ppcArgV[]);
int main (int argc, char **argv)
{
    memAccessCmd(argc, argv);

    return  (0);
}
