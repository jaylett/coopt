/*
 * $Id: strstr.c,v 1.2 1999/06/03 13:18:41 james Exp $
 * strstr.c
 * 
 * Our implementation of strstr(), because we can't guarantee that's
 * either there or sane. Returns a pointer to the start of the substring,
 * or NULL if the substring is not found.
 * (c) Copyright James Aylett 1999
 */

#include <stdio.h>
#include <stdlib.h>

#ifndef HAVE_strstr

char *strstr(char const *haystack, char const *needle)
{
  while (haystack[0]!=0)
  {
    int i=0;
    while (haystack[i]==needle[i] && haystack[i]!=0 && needle[i]!=0)
      i++;
    if (needle[i]==0)
      return (char *)haystack;
    
    haystack++;
  }
  return (char *)(NULL);
}

#endif
