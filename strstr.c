/*
 * $Id: strstr.c,v 1.1 1999/05/18 17:55:28 james Exp $
 * strstr.c
 * 
 * Our implementation of strstr(), because we can't guarantee that's
 * either there or sane. Returns a pointer to the start of the substring,
 * or NULL if the substring is not found.
 * (c) Copyright James Aylett 1999
 */

#include <stdio.h>
#include <stdlib.h>

char *coopt_strstr(char const *haystack, char const *needle)
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
