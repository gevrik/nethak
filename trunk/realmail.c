#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

#define SENDMAIL "/usr/sbin/sendmail"
#define TO "gevrik@gmail.com"
#define SUBJECT "Netrunners MUD"

void emailf(char *txt)
{
  FILE *mail;
  char buf[MAX_INPUT_LENGTH];

  sprintf(buf, "%s -t -oi", SENDMAIL);
  if ( ( mail = popen(buf,"w") ) == NULL )
  {
    bug("Error opening sendmail");
    return;
  }

  fprintf( mail, "To: %s\n", TO);
  fprintf( mail, "Subject: %s\n\n", SUBJECT);
  fprintf( mail, "%s", txt);

  pclose(mail);
}

void do_email(CHAR_DATA *ch, char *argument)
{
  emailf(argument);
  return;
}
