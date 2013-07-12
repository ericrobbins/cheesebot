/* 
 * dcc.c - dcc chat functionality
 *
 * (c) 1997 Eric Robbins
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "cheeze.h"

void add_dcc_to_list();
void send_to_all_dcc();

extern struct sockaddr_in dccsoc;

void
do_dcc(nick, user, host, to, args, flag)
char *nick;
char *user;
char *host;
char *to;
char *args;
int flag;
{
	char *thing;
	char *address;
	char *port;
	int fd;
	int p;
	unsigned long int add;

	next_arg(&args, args); /* skip CHAT */
	next_arg(&args, args); /* skip chat */
	address = next_arg(&args, args);
	port = next_arg(&args, args);
	thing = strchr(port, '\001');
	if (thing)
		*thing = '\0';

	p = atoi(port);

	sscanf(address, "%lu", &add);

	if (p < 1024)
		return;

	if (debug)
	{
		printf("!!! Trying dcc connect %s %lu port %i\n", 
			address, add, p);
		fflush(stdout);
	}

	fd = get_connect(address, p, add);

	if (fd > 0)
	{
		add_dcc_to_list(nick, user, host, fd, 1);
		send_to_all_dcc("*** %s drives a mack truck into the partyline",
			nick);
	}
	return;
}

void
initiate_dcc(nick, user, host)
char *nick;
char *user;
char *host;
{
	int fd;
	int sa;
	struct sockaddr_in sin;
	int len;

	sa = create_socket();
	if (!sa)
	{
		send_to_nick(nick, "\002Could not create DCC connection\002");
		return;
	}
	len = sizeof(sin);
	getsockname(sa, (struct sockaddr *)&sin, &len);
	send_to_server(server_fd, "PRIVMSG %s :\001DCC CHAT chat %lu %i\001",
		nick, ntohl(sin.sin_addr.s_addr), ntohs(sin.sin_port));
	if (debug)
		printf("!!! Sending DCC CHAT %s %i\n", inet_ntoa(sin.sin_addr),
			ntohs(sin.sin_port));
	add_dcc_to_list(nick, user, host, sa, 0);
	return;
}

void
close_dcc(fd)
int fd;
{
	Dcclist *this;

	for (this = dcclist; this; this = this->next)
	{
		if (this->fd == fd)
		{
			if (debug)
				printf("!!! Closing dcc from %s!%s@%s, fd %i\n",
					this->nick, this->user, this->host, this->fd);
			close(this->fd);
			this->connected = 0;
			send_to_all_dcc("*** %s was too damn cool for you and left", this->nick);
			free(this->nick);
			free(this->user);
			free(this->host);
			if (!this->next)
			{
				if (!this->prev)
				{
					dcclist = NULL;
					free(this);
					return;
				}
				else
				{
					this->prev->next = NULL;
					free(this);
					return;
				}
			}
			else
			{
				if (!this->prev)
				{
					dcclist = this->next;
					this->next->prev = NULL;
					free(this);
					return;
				}
				else
				{
					this->next->prev = this->prev;
					this->prev->next = this->next;
					free(this);
					return;
				}
			}
		}
	}
	return;
}

void
add_dcc_to_list(nick, user, host, fd, offer)
char *nick;
char *user;
char *host;
int fd;
{
	Dcclist *new;

	new = malloc(sizeof(Dcclist));

	new->nick = malloc(strlen(nick) + 1);
	strcpy(new->nick, nick);
	new->user = malloc(strlen(user) + 1);
	strcpy(new->user, user);
	new->host = malloc(strlen(host) + 1);
	strcpy(new->host, host);
	new->fd = fd;
	strcpy(new->lastbuf, "");
	new->next = dcclist;
	if (new->next)
		new->next->prev = new;
	new->prev = NULL;
	new->connected = offer;
	new->offertime = thetime;
	dcclist = new;

	if (debug)
		printf("!!! Added dcc chat to %s, fd %i\n", nick, fd);
	return;
}

int
has_dcc(nick)
char *nick;
{
	Dcclist *this;

	for (this = dcclist; this; this = this->next)
		if (!strcasecmp(nick, this->nick))
			return(1);
	return(0);
}

Dcclist *
find_dcc(fd)
int fd;
{
	Dcclist *this;

	for (this = dcclist; this; this = this->next)
		if (this->fd == fd)
			return(this);
	return(NULL);
}

void
send_to_all_dcc(pattern, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10)
char *pattern;
char *p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8, *p9, *p10;
{
	Dcclist *this;

	for (this = dcclist; this; this = this->next)
	{
		if (this->fd != -1 && this->connected)
			send_to_server(this->fd, pattern, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10);
	}
	return;
}

void
dcc_who(nick, user, host, to, stuff)
char *nick;
char *user;
char *host;     
char *to;       
char *stuff;
{       
	Dcclist *this;

	send_to_nick(nick, "\002%10s (user@host)\002", "Nick");
	for (this = dcclist; this; this = this->next)
		send_to_nick(nick, "\002%10s (%s@%s)\002", this->nick, 
			this->user, this->host);
	return;
}

void
dcc_echo(nick, user, host, to, stuff)
char *nick;
char *user;     
char *host;     
char *to;
char *stuff;
{
	return;
}
