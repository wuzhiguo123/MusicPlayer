#ifndef _NET_H
#define _NET_H

#define IP "8.130.123.33"
#define PORT 8000
int InitSocket();
int RecSocketData(char* );
int GetMusicName(const char* singer);
void ReadSocket();
void UploadMusic();


#endif