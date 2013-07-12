/*
 * aux.c - auxillary stuff
 * 
 * (c) 1997 Eric Robbins 
 *
 */

#include <string.h>
#include "cheeze.h"

char *right();

char *
next_arg(pthing, thing)
char **pthing;
char *thing;
{
	char *next;

	if (!thing)
	{
		*pthing = NULL;
		return("");
	}

	next = strchr(thing, ' ');
	if (!next)
		*pthing = NULL;
	else
	{
		*next++ = '\0';
		while (*next == ' ')
			next++;
		*pthing = next;
	}
	return(thing);
}

char *
cluster(hostname)
char *hostname;  
{
	char	temphost[255];
	char	*host;

	if (!hostname)
		return (char *) 0;
	host = temphost;
	strcpy(result, "");
	if (strchr(hostname, '@'))
	{
	    if (*hostname == '~')
		hostname++;
	    strcpy(result, hostname);
	    *strchr(result, '@') = '\0';
	    if (strlen(result) > 9)
	    {
		result[8] = '*';
		result[9] = '\0';
	    }
	    strcat(result, "@");
	    if (!(hostname = strchr(hostname, '@')))
		return (char *) 0;
	    hostname++;
	}
	strcpy(host, hostname);

	if (*host && isdigit(*(host+strlen(host)-1)))
	{
		int i; 
		char *tmp;
		tmp = host;
		for (i=0;i<3;i++)
			tmp = strchr(tmp, '.')+1;
		*tmp = '\0';
		strcat(result, host);
		strcat(result, "*");
	}
	else
	{
		char    *tmp;
		int     num;
      
		num = 1;
		tmp = right(host, 3);
		if (strlen(tmp) == 2)
			num = 2;
		while (host && *host && (numchar(host, '.') > num))
		{
			if ((host = strchr(host, '.')) != NULL)
				host++;
			else    
				return (char *) NULL;
		}	       
		strcat(result, "*");
		if (strcasecmp(host, temphost))
			strcat(result, ".");
		strcat(result, host);
	}       
	return result;
}     

int numchar(string, c)
char *string;
char c;
{
        int num = 0;
        while (*string)
        {
                if (tolower(*string) == tolower(c))
                        num++;
                string++;
        }
        return(num);  
}

char *right(string, num)
char *string;
int num; 
{
        if (strlen(string) < num)
                return(string);
        return(string+strlen(string)-num);
}

