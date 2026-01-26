#ifndef __SELECT__H
#define __SELECT__H
#include <sys/select.h>
void InitSelect();
void MySelect();
fd_set READSET;

#endif