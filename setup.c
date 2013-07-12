/*
 * setup.c - initialize, read config file
 *
 * (c) 1997 Eric Robbins
 *
 */

#include <stdio.h>
#include <string.h>
#include "cheeze.h"

/* the next 2 should really be in user.c */
void
clear_shitlist()
{
	Shitlist *tmp, *next;
	
	for (tmp = shitlist; tmp;)
	{
		if (tmp->reason)
			free(tmp->reason);
		if (tmp->hostmask)
			free(tmp->hostmask);
		if (tmp->channel)
			free(tmp->channel);
		next = tmp->next;
		free(tmp);
		tmp = next;
	}
	shitlist = NULL;
	return;
}

void
clear_userlist()
{
	Accesslist *tmp, *next;
	
	for (tmp = accesslist; tmp;)
	{
		if (tmp->hostmask)
			free(tmp->hostmask);
		if (tmp->passwd)
			free(tmp->passwd);
		if (tmp->channel)
			free(tmp->channel);
		next = tmp->next;
		free(tmp);
		tmp = next;
	}
	accesslist = NULL;
	return;
}

void
load_users()
{
	FILE *infile;
	char foo[256];
	char *hostmask, *channels, *passwd, *level, *aop, *prot, *tmp;
	int l, p, a;

	clear_userlist();

	sprintf(foo, "%s.userlist", mynick);
	infile = fopen(foo, "r");
	if (!infile)
	{
		printf("Could not open userlist %s!\n", foo);
		exit(8);
	}
	while (fgets(foo, sizeof(foo), infile))
	{
		tmp = foo;
		while (isspace(*tmp))
			tmp++;

		if (strchr(tmp, '\n'))
			*strchr(tmp, '\n') = '\0';

		hostmask = next_arg(&tmp, tmp);
		channels = next_arg(&tmp, tmp);
		level = next_arg(&tmp, tmp);
		aop = next_arg(&tmp, tmp);
		prot = next_arg(&tmp, tmp);
		passwd = next_arg(&tmp, tmp);

		l = atoi(level);
		p = atoi(prot);
		a = atoi(aop);

		add_user(hostmask, "", "", channels, l, a, p, passwd, NULL);

		if (debug)
			printf("add user: %s (%i/%i/%i) on channels %s\n",
				hostmask, l, a, p, channels);
	}
	fclose(infile);
	if (debug)
		printf("Loaded userlist %s.userlist\n", mynick);
	return;
}

void
load_shit()
{
	FILE *infile;
	char foo[256];
	char *hostmask, *channels, *reason, *tmp;

	clear_shitlist();

	sprintf(foo, "%s.shitlist", mynick);
	infile = fopen(foo, "r");
	if (!infile)
	{
		printf("Could not open shitlist %s!\n", foo);
		return;
	}
	while (fgets(foo, sizeof(foo), infile))
	{
		tmp = foo;
		while (isspace(*tmp))
			tmp++;

		if (strchr(tmp, '\n'))
			*strchr(tmp, '\n') = '\0';

		hostmask = next_arg(&tmp, tmp);
		channels = next_arg(&tmp, tmp);
		
		add_shit(hostmask, "", "", channels, tmp, NULL);
	}
	fclose(infile);
	if (debug)
		printf("Loaded shitlist %s.shitlist\n", mynick);
	return;
}

void
read_config(filename)
char *filename;
{
	FILE *infile;
	char buf[256];
	char *this;
	char *command;
	char *arg;
	char *port;
	int p;

	infile = fopen(filename, "r");
	while (fgets(buf, sizeof(buf), infile))
	{
		this = buf;

		if (arg = strchr(this, '\n')) /* kill newline */
			*arg = '\0';

		while (*this && isspace(*this)) /* advance past leading whitespace */
			this++;

		if (*this == '#' || !*this) /* comment line or blank line */
			continue;

		command = next_arg(&this, this);
		arg = next_arg(&this, this);

		if (debug)
			printf("### Config file: command %s arg %s\n", command, arg);

		if (!strcasecmp(command, "nick"))
			strcpy(mynick, arg);
		else if (!strcasecmp(command, "server"))
		{
			port = next_arg(&this, this);
			p = atoi(port);
			if (p)
				new_server(arg, p, 0);
			else
				new_server(arg, 6667, 0);
		}
		else if (!strcasecmp(command, "channel"))
			add_channel(arg, 0);
		else if (!strcasecmp(command, "username"))
			strcpy(myusername, arg);
		else if (!strcasecmp(command, "ircname"))
		{
			strcpy(myircname, arg);
			strcat(myircname, " ");
			strcat(myircname, this);
		}
		else if (!strcasecmp(command, "logfile"))
			strcpy(mylogfile, arg);
		else if (!strcasecmp(command, "commandchar"))
			commandchar = *arg;
		else if (!strcasecmp(command, "hostname"))
			strcpy(myhostname, arg);

	}
	fclose(infile);
	load_users();
	load_shit();
	return;
}
