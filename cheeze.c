/*
 * cheeze.c - the cheeziest
 *
 * (c) 1997 Eric Robbins
 *
 */

#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
/* #include "cheeze.h" */
#include "struct.h"

#define BUFFER_SIZE 2048

char 	mynick[10];	/* Nick to use */
char	myserver[80];
char 	mychannel[80];
char	myusername[80];
char	myhostname[80];
char	myircname[80];
char	mylogfile[80];
char 	in[BUFFER_SIZE];	/* read from server */
char 	out[BUFFER_SIZE];	/* send to server */
char	processline[BUFFER_SIZE * 2]; /* thingy */
char 	userhostbuf[256];
char	result[BUFFER_SIZE];
char	outmode[16];
char	outmodestring[BUFFER_SIZE];
int	send_mode = 0;
int	deop_user = 0;
char	commandchar = '%';
int	server_fd;		/* socket */
int	connected = 0;
int	dcc_command = 0;
time_t	thetime;
time_t	lasttime;

Channel *channels = NULL;
Shitlist *shitlist = NULL;
Accesslist *accesslist = NULL;
Authlist *authlist = NULL;
Userhost *userhostqueue = NULL;
Dcclist *dcclist = NULL;
Serverlist *serverlist = NULL;

Serverlist *currentserver = NULL;

char *fromnick;
char *fromuser;
char *fromhost;

#ifdef DEBUG
int	debug = 1;
#else
int	debug = 0;
#endif

#define CONFIGFILE "cheeze.cf"

int 	get_connect();
void	connect_to_server();
void	reconnect_to_server();
void	send_to_nick();
void	send_to_server();
int	read_from_server();
void	do_register();
void	parse_from_server();
void	log_it();
void	io_loop();
void 	process_line();

extern struct server_stuff text_messages[];
extern struct server_stuff numeric_messages[];
extern char *next_arg();
extern void func_debug();
extern Dcclist* find_dcc();

int 
get_connect(hostname, port, address)
char *hostname;
int port;
unsigned long int address; /* cuz I'm lazy */
{
	struct sockaddr_in sa;
	struct sockaddr_in localsa;
	struct hostent	  *he;
	int		  soc;
	int	var = 1;
	long int something;
	struct timeval timev;

	bzero(&sa, sizeof(sa));
	if (address)
	{
		sa.sin_addr.s_addr = htonl(address);
		if (debug)
		{
			printf("!!! get_connect(%s, %i)\n",
				inet_ntoa(sa.sin_addr), port);
			fflush(stdout);
		}
		goto doit;
	}

	he = gethostbyname(hostname);

	if (!he)
		return(-1);

	bcopy(he->h_addr, (char *)&sa.sin_addr, he->h_length);
doit:
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	soc = socket(AF_INET, SOCK_STREAM, 0);
	if (soc == -1)
		return(-1);

	he = gethostbyname(myhostname);

	if (he) /* virtualhost */
	{
		bzero(&localsa, sizeof(localsa));
		localsa.sin_port = 0;
		localsa.sin_family = AF_INET;
		bcopy(he->h_addr, (char *)&localsa.sin_addr, he->h_length);
		bind(soc, (struct sockaddr *)&localsa, sizeof(localsa));
	}

	timev.tv_sec = 15;
	timev.tv_usec = 0;

	/* set timeout on read/write to 15 seconds and the reuse flag */

	setsockopt(soc, SOL_SOCKET, SO_REUSEADDR, &var, sizeof(int));
	setsockopt(soc, SOL_SOCKET, SO_SNDTIMEO, &timev, sizeof(struct timeval));
	setsockopt(soc, SOL_SOCKET, SO_RCVTIMEO, &timev, sizeof(struct timeval));
	
	if (connect(soc, (struct sockaddr *)&sa, sizeof(sa)) == -1)
	{
		close(soc);
		return(-1);
	}
	return(soc);
}

int
create_socket()
{
	struct hostent *hp;
	struct timeval timev;
	struct sockaddr_in dccsoc;
	int sock;
	int var = 1;

	hp = gethostbyname(myhostname);

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
		return(0);

	if (debug)
		printf("!!! Got socket %i\n", sock);

	bzero(&dccsoc, sizeof(dccsoc));
	dccsoc.sin_family = AF_INET;
	dccsoc.sin_port = 0;
	bcopy(hp->h_addr, &dccsoc.sin_addr, hp->h_length);

	if (debug)
		printf("!!! Machine IP: %s\n", inet_ntoa(dccsoc.sin_addr));

	timev.tv_sec = 15;
	timev.tv_usec = 0;

	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &var, sizeof(int));
	setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timev, sizeof(struct timeval));
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timev, sizeof(struct timeval));
	if (bind(sock, (struct sockaddr *)&dccsoc, sizeof(struct sockaddr_in)) == -1)
	{
		perror("bind()");
		return(0);
	}
	if (listen(sock, 1) == -1)
	{
		printf("listen() fail");
		return(0);
	}

	return(sock);
}

void
new_server(name, port, flag)
char *name;
int port;
int flag;
{
	Serverlist *new;

	for (new = serverlist; new; new = new->next)
	{
		if (!strcasecmp(new->name, name))
			break;
	}
	if (new)
	{
		currentserver = new;
		if (flag)
			connect_to_server();
		return;
	}
	else
	{
		new = malloc(sizeof(Serverlist));
		new->name = malloc(strlen(name) + 1);
		strcpy(new->name, name);
		new->port = port;
		new->next = serverlist;
		serverlist = new;
		currentserver = new;
		if (flag)
			connect_to_server();
		return;
	}
}

void 
connect_to_server()
{
	connected = 0;
	if (!currentserver)
		currentserver = serverlist;
	while (1)
	{
		if (server_fd > 0)
			close(server_fd);
		server_fd = get_connect(currentserver->name, currentserver->port, 0);
		if (server_fd < 1)
			currentserver = currentserver->next;
		else
			break;
		if (!currentserver)
			currentserver = serverlist;
	}
	connected = 1;
	do_register();
}

void 
reconnect_to_server()
{
	if (server_fd != -1)
	{
		shutdown(server_fd, 2);
		close(server_fd);
	}
	currentserver = currentserver->next;
	connect_to_server();
}

void
send_to_nick(nick, pattern, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10)
char *nick;
char *pattern;
char *p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8, *p9, *p10;
{
	Dcclist *this;
	static char formatbuf[64];

	for (this = dcclist; this; this = this->next)
		if (!strcasecmp(this->nick, nick) && this->connected)
			break;

	if (!this)
	{
		sprintf(formatbuf, "NOTICE %s :%s", nick, pattern);
		send_to_server(server_fd, formatbuf, p1, p2, p3, p4, p5, p6, p7,
			p8, p9, p10);
	}
	else
		send_to_server(this->fd, pattern, p1, p2, p3, p4, p5, p6, p7, p8, 
			p9, p10);
	return;
}

void 
send_to_server(fd, pattern, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10)
int fd;
char *pattern;
char *p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8, *p9, *p10;
{
	int success;

	sprintf(out, pattern, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10);
	strcat(out, "\n");
	success = write(fd, out, strlen(out));
	if (success < 1)
	{
		if (errno != EINTR)
		{
			if (fd == server_fd)
			{
				connected = 0;
				reconnect_to_server();
			}
			else
				close_dcc(fd);
		}
		else
			write(fd, out, strlen(out));
	}
	return;
}

int 
read_from_server(fd)
int fd;
{
	int i;

	bzero(in, sizeof(in));

	i = read(fd, in, sizeof(in) - 1);
	if (i == -1)
	{
		if (fd == server_fd)
			connected = 0;
		return(i);
	}
	if (i == 0 && errno != EINTR)
	{
		if (fd == server_fd)
			connected = 0;
		return(i);
	}
	in[i] = '\0';
/*
	if (debug)
		printf("read(): %s", in);
*/
	return(1);
}

void 
do_register()
{
	send_to_server(server_fd, "USER %s %s %s  :%s", myusername, 
		myhostname, currentserver->name, myircname);
	send_to_server(server_fd, "NICK %s", mynick);
/*
	send_to_server(server_fd, "MODE %s +i", mynick);
	send_to_server(server_fd, "JOIN %s", mychannel);
*/
	return;
}

void 
parse_from_server()
{
	char *line, *nextline;

	strcat(processline, in);
	line = processline;
	while (nextline = strchr(line, '\n'))
	{
		*nextline++ = '\0';
		/*nextline++; */
		process_line(line);
		line = nextline;
		if (!*line)
			break;
	}
	bzero(in, sizeof(in));
	if (*line) /* extra shit */
		strcpy(in, line);
	else
		strcpy(in, "");
	bzero(processline, sizeof(processline));
	strcpy(processline, in);
	return;
}

void
parse_dcc(fd)
int fd;
{
	char *line, *nextline;
	Dcclist *this;
	static char theline[512];

	this = find_dcc(fd);

	if (debug)
		printf("Parsing dcc input: %s\n", in);

	strcat(this->lastbuf, in);
	line = this->lastbuf;
	while (nextline = strchr(line, '\n'))
	{
		*nextline++ = '\0';
		sprintf(theline, ":%s!%s@%s PRIVMSG %s :%s", this->nick, 
			this->user, this->host, mynick, line);
		if (debug)
			printf("doing line: %s\n", theline);
		line = nextline;
		process_line(theline);
		if (!*line)
			break;
	}
	if (*line)
		strcpy(in, line);
	else
		strcpy(in, "");
	bzero(this->lastbuf, sizeof(this->lastbuf));
	strcpy(this->lastbuf, in);
	return;
}

void
process_line(theline)
char *theline;
{
	char *tmp, *type;
	int i;

	if (tmp = strchr(theline, '\r'))
		*tmp = '\0';

	if (debug)
		printf("### Processing line %s\n", theline);

	if (!strncmp("PING ", theline, 5))
	{
		send_to_server(server_fd, "PONG %s", mynick);
		return;
	}

	if (!strncmp("ERROR ", theline, 6))
	{
		reconnect_to_server();
		log_it();
		return;
	}

	/* FIX THIS IT DOES NOTHING AT ALL */
	tmp = theline;
	if (*tmp == ':')
		tmp++;

	fromnick = tmp;
	fromuser = strchr(fromnick, '!');
	if (fromuser)
	{
		*fromuser++ = '\0';
		fromhost = strchr(fromuser, '@');
		if (fromhost)
			*fromhost++ = '\0';
		else
			fromhost = fromuser;
	}
	else
		fromuser = fromhost = fromnick;

	tmp = strchr(fromhost, ' ');
	if (!tmp) /* something is kind of fucked */
		return;
	*tmp++ = '\0';
	type = next_arg(&tmp, tmp);

	if (isdigit(*type))
	{
		for (i = 0; numeric_messages[i].text; i++)
		{
			if (!strcasecmp(type, numeric_messages[i].text))
			{
				numeric_messages[i].func(fromnick, fromuser, 
					fromhost, tmp);
				return;
			}
		}
		return; /* unknown type */
	}
	else
	{
                for (i = 0; text_messages[i].text; i++) 
                {
                        if (!strcasecmp(type, text_messages[i].text))
                        {       
                                text_messages[i].func(fromnick, fromuser,
                                        fromhost, tmp);
                                return;
                        }
                }
		return; /* unknown */
	}
	return;
}

void 
log_it()
{
	FILE *logfile;

	logfile = fopen(mylogfile, "a");

	if (logfile)
	{
		fprintf(logfile, "%s%s\n", ctime(&thetime), in);
		fclose(logfile);
	}
	return;
}

void
housekeeping()
{
	Channel *this;
	Dcclist *dccs, *next;

	/* clear old dccs not accepted */
	for (dccs = dcclist; dccs; )
	{
		next = dccs->next;
		if (!dccs->connected && (thetime - dccs->offertime) > 300)
		{
			send_to_nick(dccs->nick, "\002DCC Chat closed\002");
			close_dcc(dccs->fd);
		}
		dccs = next;
	}

	/* join channels we're not on and should be */
	for (this = channels; this; this = this->next)
	{
		if (!this->on_channel && (thetime - this->lastjoin) > 30)
		{
			send_to_server(server_fd, "JOIN %s %s", this->name,
				this->key ? this->key : "");
			this->lastjoin = thetime;
			if (debug)
				printf("### Trying to join channel %s\n", this->name);
		}
	}

	if (thetime - lasttime > 600)
		reconnect_to_server();
	
	return;
}

void 
io_loop()
{
	struct timeval timeout;
	struct sockaddr_in dccsa;
	fd_set fdset;
	int connected;
	int oldfd;
	int fuckoff;
	Dcclist *this;

#ifndef DEBUG
	if (fork())
		exit(0);
	else
	{
/* broken linux/sysv crap */
#if defined(__linux__) || defined(SYSV)
		setpgrp();
#else
		setpgrp(0, getpid());
#endif
		close(0);
		close(1);
		close(2);
	}
#endif

	timeout.tv_sec = 10;
	timeout.tv_usec = 0;

	connect_to_server();

	lasttime = time(NULL);

	while(1)
	{
		thetime = time(NULL);

/* broken shit linux again */
#ifdef __linux__
		timeout.tv_sec = 10;
		timeout.tv_usec = 0;
#endif
		FD_ZERO(&fdset);
		FD_SET(server_fd, &fdset);
		for (this = dcclist; this; this = this->next)
		{
			if (this->fd != -1)
				FD_SET(this->fd, &fdset);
		}
		if (debug)
			FD_SET(0, &fdset);
		if (select(FD_SETSIZE, &fdset, NULL, NULL, &timeout) > 0)
		{
			if (debug)
			{
				if (FD_ISSET(0, &fdset))
				{
					read_from_server(0);
					send_to_server(server_fd, in);
				}
			}
			if (FD_ISSET(server_fd, &fdset))
			{
				lasttime = thetime;
				connected = read_from_server(server_fd);
				dcc_command = 0;
				if (connected > 0)
					parse_from_server();
				else
					reconnect_to_server();
			}
			for (this = dcclist; this; this = this->next)
			{
				if (this->connected && FD_ISSET(this->fd, &fdset))
				{
					connected = read_from_server(this->fd);
					dcc_command = 1;
					if (connected > 0)
						parse_dcc(this->fd);
					else
						close_dcc(this->fd);
				}
				else if (FD_ISSET(this->fd, &fdset))
				{
					oldfd = this->fd;
					fuckoff = sizeof(dccsa);
					this->fd = accept(this->fd, 
						(struct sockaddr *)&dccsa, &fuckoff);
					this->connected = 1;
					close(oldfd);
					send_to_all_dcc("*** %s drives a mack truck into the partyline", this->nick);
					if (debug)
						printf("!!! Accepted DCC from %s (port %i)\n", inet_ntoa(dccsa.sin_addr), this->fd);
				}
			}
		}
		housekeeping();

	}
}

void
sig_segv()
{
	send_to_server(server_fd, "QUIT :\002feel the power of the SEGV...\002");
	sleep(2);
	exit(8);
}

void
sig_bus()
{
	send_to_server(server_fd, "QUIT :\002another one rides the SIGBUS...\002");
	sleep(2);
	exit(8);
}

main(argc, argv)
char **argv;
int argc;
{
	signal(SIGINT, func_debug);
	signal(SIGHUP, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGSEGV, sig_segv);
	signal(SIGBUS, sig_bus);

	if (argc == 2)
	{
		strncpy(mynick, argv[1], 9);
		gethostname(myhostname, sizeof(myhostname));
		read_config(CONFIGFILE);
		io_loop();
	}
	else
		printf("Usage: %s <nick>\n", argv[0]);
	exit(0);
}
