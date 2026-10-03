/*****************************************************************************.
 *
 * FILE        : FVCDOS.C
 * PROJECT     : File Version Control (FVC)
 * COMPANY     : 
 * AUTHOR      : Lsalwestfox
 * CREATED     : D09/28/2026T08:18:45-UTC
 * EDITED      : D09/28/2026T08:18:45-UTC
 * DESCRIPTION : WATCOM (R) 10.x MS-DOS 6.2 layer of the tool
 * NOTICE      : Copyright (C) 2026 Lsalwestfox
 *
 *****************************************************************************/

#include <dos.h>
#include <stdio.h>
#include <stdlib.h>

#include "FVC.C"

static int _errno7;

int get_errno()
{
   return(_errno7);
}

void *talloc(ullong amount)
{
   return(malloc(amount));
}

string8_t read_entire_file(const char *path)
{
   FILE      *file = fopen(path, "rb");
   string8_t  out;
   int        head;

   if (!file)
   {
      _errno7 = 1;
      return(NULLSTR);
   }

   head = ftell(file);
   fseek(file, 0, SEEK_END);
   out.size = ftell(file);
   fseek(file, head, SEEK_SET);
   out.str = (char*)talloc(out.size);

   fread(out.str, 1, out.size, file);
   return(out);
}

writer open_writer(const char *path)
{
   return(fopen(path, "wb"));
}

void cwrite_to(writer write, const char *str)
{
   fprintf(write, "%s", str);
}

void write_to(writer write, string8_t str)
{
   fwrite(str.str, 1, str.size, write);
}

void cprint(const char *str)
{
   printf("%s", str);
}

void print(string8_t str)
{
   printf("%.*s", str.size, str.str);
}

string8_t timestamp()
{
   struct dosdate_t date;
   struct dostime_t time;
   string8_t out;

   _dos_getdate(&date);
   _dos_gettime(&time);

   out.str = (char*)talloc(out.size = 24);
   out.str[0]  = 'D';
   out.str[1]  = '0' + date.month / 10;
   out.str[2]  = '0' + date.month % 10;
   out.str[3]  = '/';
   out.str[4]  = '0' + date.day / 10;
   out.str[5]  = '0' + date.day % 10;
   out.str[6]  = '/';
   out.str[7]  = '0' + date.year / 1000 % 10;
   out.str[8]  = '0' + date.year / 100  % 10;
   out.str[9]  = '0' + date.year / 10   % 10;
   out.str[10] = '0' + date.year        % 10;
   out.str[11] = 'T';
   out.str[12] = '0' + time.hour / 10;
   out.str[13] = '0' + time.hour % 10;
   out.str[14] = ':';
   out.str[15] = '0' + time.minute / 10;
   out.str[16] = '0' + time.minute % 10;
   out.str[17] = ':';
   out.str[18] = '0' + time.second / 10;
   out.str[19] = '0' + time.second % 10;
   out.str[20] = '-';
   out.str[21] = 'U';
   out.str[22] = 'T';
   out.str[23] = 'C';
   return(out);
}

/*****************************************************************************,
 *
 * HISTORY:
 *
 * [D09/28/2026T08:18:45-UTC] Added DOS and C89 support
 *    - Lsalwestfox
 *
 *****************************************************************************/

