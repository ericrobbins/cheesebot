/*
 * Written By Douglas A. Lewis <dalewis@cs.Buffalo.EDU>
 *
 * This file is in the public domain.
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>

static	int	total_explicit;

#define RETURN_FALSE -1
#define RETURN_TRUE count

u_char lower_tab[256] = 
{
  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
 64, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111,
112,113,114,115,116,117,118,119,120,121,122, 91, 92, 93, 94, 95,
 96, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111,
112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,
128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
144,145,145,147,148,149,150,151,152,153,154,155,156,157,158,159,
160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,
208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,
224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,
240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255
};

#undef tolower		/* don't want previous version. */
#define tolower(x) lower_tab[x]

int
_wild_match(mask, string)
     u_char *mask, *string;
{
  register u_char *m = mask, *n = string, *ma = NULL, *na = NULL, temp;
  int just = 0;
  u_char *pm = NULL, *pn = NULL;
  int lp_c = 0, count = 0, last_count = 0;

  while(1)
    {

      switch (*m)
	{
	case '*':
	  goto ultimate_lameness;
	  break;
	case '%':
	  goto ultimate_lameness2;
	  break;
	case '?':
	  m++;
	  if (!*n++) return RETURN_FALSE;
	  break;
	case 0:
	  if (!*n)
	    return RETURN_TRUE;
	case '\\':
	  if ((*m=='\\') && (m[1]=='*') || (m[1]=='?')) m++;
	default:
	  if (tolower(*m) != tolower(*n))
	    return RETURN_FALSE;
	  else
	    {
	      count++;
	      m++;
	      n++;
	    }
	}
    }

  while(1)
    {

      switch (*m)
	{
	case '*':
ultimate_lameness:
	  ma = ++m;
	  na = n;
	  just = 1;
	  last_count = count;
	  break;
	case '%':
ultimate_lameness2:
	  pm = ++m;
	  pn = n;
	  lp_c = count;
	  if (*n == ' ') pm = NULL;
	  break;
	case '?':
	  m++;
	  if (!*n++) return RETURN_FALSE;
	  break;
	case 0:
	  if (!*n || just)
	    return RETURN_TRUE;
	case '\\':
	  if ((*m=='\\') && (m[1]=='*') || (m[1]=='?')) m++;
	default:
	  just = 0;
	  if (tolower(*m) != tolower(*n))
	    {
	      if (!*n) return RETURN_FALSE;
	      if (pm)
		{
		  m = pm;
		  n = ++pn;
		  count = lp_c;
		  if (*n == ' ') pm = NULL;
		}
	      else
		if (ma)
		  {
		    m = ma;
		    n = ++na;
		    if (!*n) return RETURN_FALSE;
		    count = last_count;
		  }
		else return RETURN_FALSE;
	    }
	  else
	    {
	      count++;
	      m++;
	      n++;
	    }
	}
    }
}

int
match(pattern, string)
	char	*pattern, *string;
{
/* -1 on false >= 0 on true */
  return ((_wild_match(pattern, string)>=0)?1:0);
}

int
wild_match(pattern, str)
	char	*pattern,
		*str;
{
	/* assuming a -1 return of false */
	return _wild_match(pattern, str) + 1;
}
