/*
 * parse.c - parse shit from the server 
 *
 * (c) 1997 Eric Robbins
 *
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cheeze.h"
#include "user.h"

/* some advance declarations */

void in_privmsg();
void in_join();
void in_part();
void in_mode();
void in_kick();
void in_nick();
void in_signoff();
void in_notice();
void in_invite();
void in_nothing();
void in_who();
void in_userhost();
void in_banned();
void in_banlist();
void in_channelmode();
void in_endwho();
void in_endbanlist();
void in_nouserline();
void in_nickinuse();
void in_welcome();
void in_klined();

/* I put these in order of expected frequency to reduce processing
time in going through the list */

struct server_stuff text_messages[] = {
	{ "PRIVMSG",	in_privmsg 	},
	{ "JOIN",	in_join 	},
	{ "PART",	in_part 	},
	{ "MODE",	in_mode 	},
	{ "KICK",	in_kick 	},
	{ "NICK",	in_nick		},
	{ "QUIT",	in_signoff 	},
	{ "NOTICE",	in_notice 	},
	{ "INVITE",	in_invite	},
	{ NULL,		in_nothing	}
};

struct server_stuff numeric_messages[] = {
	{ "352",	in_who		},
	{ "302",	in_userhost	},
	{ "471",	in_banned	},
	{ "473",	in_banned	},
	{ "474",	in_banned	},
	{ "475",	in_banned	},
	{ "476",	in_banned	},
	{ "367",	in_banlist	},
	{ "324",	in_channelmode	},
	{ "315",	in_endwho	},
	{ "368",	in_endbanlist	},
	{ "461",	in_nouserline	},
	{ "433",	in_nickinuse	},
	{ "001",	in_welcome	},
	{ "463",	in_klined	},
	{ "464",	in_klined	},
	{ "465",	in_klined	},
	{ NULL,		in_nothing	}
};

void
in_nothing()
{
	return;
}

void
in_privmsg(fromnick, fromuser, fromhost, args)
char *fromnick;
char *fromuser;
char *fromhost;
char *args;
{
	char *garbage;
	char *thestring;
	int i;

	garbage = next_arg(&args, args); /* to */
	thestring = next_arg(&args, args); /* string */
	if (*thestring == ':')
		thestring++;
	if (*thestring == commandchar)
	{
		try_command(thestring, fromnick, fromuser, fromhost, garbage, args);
		return;
	}
	if (*thestring == '\001' && !strcasecmp(garbage, mynick))
	{
		thestring++;
		try_command(thestring, fromnick, fromuser, fromhost, garbage, args);
		return;
	}
	if (debug && !dcc_command)
		printf("<%s:%s> %s %s\n", fromnick, garbage, thestring, 
			(args && *args) ? args : "");
	if (dcc_command)
	{
		if (*thestring == '.')
		try_dcc_command(thestring, fromnick, fromuser, fromhost, garbage, args);
		else
			send_to_all_dcc("<%s> %s %s", fromnick, thestring, (args && *args) ? args : "");
	}
        return;
}

void
in_join(fromnick, fromuser, fromhost, args)
char *fromnick;
char *fromuser;
char *fromhost;
char *args;
{
	if (*args == ':')
		args++;
	if (!strcmp(fromnick, mynick))
	{
		delete_channel(args, 1);
		add_channel(args, 1);
		send_to_server(server_fd, "WHO %s\nMODE %s\nMODE %s +b\n",
			args, args, args);
	}
	else
	{
		delete_user_from_channel(args, fromnick);
		add_user_to_channel(args, fromnick, fromuser, fromhost, 0);
		process_user(fromnick, fromuser, fromhost, args);

	}
	if (debug)
		printf("*** %s (%s@%s) has joined channel %s\n", fromnick, 
		    fromuser, fromhost, args);
        return;
}

void
in_part(fromnick, fromuser, fromhost, args)
char *fromnick;
char *fromuser;
char *fromhost;
char *args;
{
	if (*args == ':')
		args++;
	if (!strcmp(fromnick, mynick))
	{
		delete_channel(args, 1);
		clear_auths();
	}
	else
	{
		delete_user_from_channel(args, fromnick);
		delete_auth(fromnick, 0);
	}
	if (debug)
		printf("*** %s has left channel %s\n", fromnick, args);
        return;
}

void
in_mode(fromnick, fromuser, fromhost, args)
char *fromnick;
char *fromuser;
char *fromhost;
char *args;
{
	char *target;
	char *mode;
	char *string;
	char *plusminus;
	int adding = 0;
	int was_op;
	Channel *this;
	Userlist *user;
	Banlist *ban;

	target = next_arg(&args, args);

	if (!strcmp(target, mynick))
		return;
	
	this = find_channel(target);
	if (!this)
	{
		if (debug)
		{
			printf("BZZZT! bad coding.. channel %s not found\n", target);
			printf("*** Mode change \"%s\" on channel %s by %s\n",
				args, target, fromnick);
		}
		return;
	}

	if (debug)
		printf("*** Mode change \"%s\" on channel %s by %s\n", args, 
			target, fromnick);

	strcpy(outmode, "");
	strcpy(outmodestring, "");
	send_mode = 0;
	deop_user = 0;

	plusminus = next_arg(&args, args);

    while (*plusminus)
    {
	if (debug)
		printf("parsing mode %s%c %s\n", adding ? "+" : "-", *plusminus, 
			(strchr("olbk", *plusminus) ? args : ""));
	switch (*plusminus++)
	{
		case '+':
			adding = 1;
			break;
		case '-':
			adding = 0;
			break;
		case 'o':
			string = next_arg(&args, args);
			for (user = this->users; user; user = user->next)
			{
				if (!strcmp(user->name, string))
					break;
			}
			if (!user)
				break;
			if (adding)
				user->modes |= UMODE_O;
			else 
			{
				check_protect(PROT_DEOP, fromnick, fromuser, fromhost, 
					this->name, string);
				user->modes &= ~UMODE_O;
			}
			if (!strcasecmp(mynick, user->name))
			{
				was_op = this->am_opped;
				this->am_opped = (adding) ? 1 : 0;
				if (!was_op)
					scan_channel(this->name);
			}
			break;
		case 'b':
			string = next_arg(&args, args);
			if (adding)
			{
				check_protect(PROT_BAN, fromnick, fromuser, fromhost,
					this->name, string);
				add_channel_ban(target, string);
			}
			else
				delete_channel_ban(target, string);
			break;
		case 'l':
			if (!adding)
			{
				this->mode &= ~MODE_L;
				this->limit = 0;
				break;
			}
			this->mode |= MODE_L;
			string = next_arg(&args, args);
			this->limit = atoi(string);
			break;
		case 'k':
			string = next_arg(&args, args);
			if (adding)
			{
				this->mode |= MODE_K;
				if (this->key)
					free(this->key);
				this->key = malloc(strlen(string) + 1);
				strcpy(this->key, string);
			}
			else
			{
				this->mode &= ~MODE_K;
				if (this->key)
					free(this->key);
				this->key = NULL;
			}
			break;
		case 'p':
			if (adding)
				this->mode |= MODE_P;
			else
				this->mode &= ~MODE_P;
			break;
		case 's':
			if (adding)
				this->mode |= MODE_S;
			else
				this->mode &= ~MODE_S;
			break;
		case 't':	
			if (adding)
				this->mode |= MODE_T;
			else
				this->mode &= ~MODE_T;
			break;
		case 'i':
			if (adding)
				this->mode |= MODE_I;
			else
				this->mode &= ~MODE_I;
			break;
		case 'n':
			if (adding)
				this->mode |= MODE_N;
			else
				this->mode &= ~MODE_N;
			break;
		default:
			if (debug)
				printf("unknown mode %c\n", *plusminus);
			break;
	}
    }
	if (send_mode)
		send_to_server(server_fd, "MODE %s %s %s", this->name, outmode, outmodestring);
        return;
}

void
in_kick(fromnick, fromuser, fromhost, args)
char *fromnick;
char *fromuser;
char *fromhost;
char *args;
{
	char *target;
	char *channel;

	channel = next_arg(&args, args);
	target = next_arg(&args, args);
	
	if (!strcasecmp(target, mynick))
	{
		send_to_server(server_fd, "JOIN %s %s", channel,
			getkey(channel) ? getkey(channel) : "");
		delete_channel(channel, 0);
		clear_auths();
	}
	else
	{
		delete_user_from_channel(channel, target);
		delete_auth(fromnick, 0);
	}
	if (debug)
		printf("*** %s has been kicked off channel %s by %s (%s)\n",
			target, channel, fromnick, args + 1);
        return;
}

void
in_nick(fromnick, fromuser, fromhost, args)
char *fromnick;
char *fromuser;
char *fromhost;
char *args;
{
	if (*args == ':')
		args++;
	change_all_nick(fromnick, args);
	if (debug)
		printf("*** %s is now known as %s\n", fromnick, args);
	return;
}

void
in_signoff(fromnick, fromuser, fromhost, args)
char *fromnick;
char *fromuser;
char *fromhost;
char *args;
{
	delete_from_all_channels(fromnick);
	delete_auth(fromnick, 1);
	if (debug)
		printf("*** Signoff: %s (%s)\n", fromnick, args);
        return;
}

void
in_notice(fromnick, fromuser, fromhost, args)
char *fromnick;
char *fromuser;
char *fromhost;
char *args;
{
	if (debug)
		printf("-%s- %s\n", fromnick, args);
        return;
}

void
in_invite(fromnick, fromuser, fromhost, args)
char *fromnick;
char *fromuser;
char *fromhost;
char *args;
{
	next_arg(&args, args);
	if (*args == ':')
		args++;
	if (get_userstat(fromnick, fromuser, fromhost, args, USER_LEVEL))
		send_to_server(server_fd, "JOIN %s", args);
	if (debug)
		printf("*** %s invites you to channel %s\n",
			fromnick, args);
        return;
}

void
in_who(fromnick, fromuser, fromhost, args)
char *fromnick;
char *fromuser;
char *fromhost;
char *args;
{
	char *channel, *nick, *user, *host, *mode;
	int op = 0;

	next_arg(&args, args);
	channel = next_arg(&args, args);
	user = next_arg(&args, args);
	host = next_arg(&args, args);
	next_arg(&args, args);
	nick = next_arg(&args, args);
	mode = next_arg(&args, args);
	if (strchr(mode, '@'))
		op += 1;
	if (strchr(mode, '+'))
		op += 2;
	delete_user_from_channel(channel, nick);
	add_user_to_channel(channel, nick, user, host, op);
	if (debug)
		printf("Added user %s!%s@%s to channel %s\n", nick,
			user, host, channel);
	return;
}

void
in_userhost(fromnick, fromuser, fromhost, args)
char *fromnick;
char *fromuser;
char *fromhost;
char *args;
{
	char *n, *u, *h;
	char *host;
	Userhost *uhqueue, *next;
	char buf[256];

	next_arg(&args, args);

	if (*args == ':')
		args++;

	if (!*args)
		return; /* user not on */

	n = args;
	u = n;
	while (*u && *u != '*' && *u != '=')
		u++;
	*u++ = '\0';
	while (*u && (*u == '=' || *u == '+' || *u == '-'))
		u++;
	h = strchr(u, '@');
	*h++ = '\0';
	host = next_arg(&h, h);
	if (debug)
		printf("### Userhost %s: %s@%s\n", n, u, host);

	for (uhqueue = userhostqueue; uhqueue; )
	{
		if (!strcasecmp(uhqueue->nick, n))
		{
			next = uhqueue->next;
			switch (uhqueue->type)
			{
				case UHOST_ADD:
					add_user(n, u, host, uhqueue->channel, 
						uhqueue->level, uhqueue->aop,
						uhqueue->prot, uhqueue->string,
						uhqueue->fromnick);
					break;
				case UHOST_DELETE:
					delete_user(n, u, host, uhqueue->fromnick);
					break;
				case UHOST_SHIT:
					add_shit(n, u, host, uhqueue->channel,
						uhqueue->string, uhqueue->fromnick);
					break;
				case UHOST_UNSHIT:
					delete_shit(n, u, host, uhqueue->fromnick);
					break;
				case UHOST_BAN:
				case UHOST_UNBAN: /* no funcs yet */
					break;
				case UHOST_PROT:
				case UHOST_RPROT:
					user_prot(n, u, host, uhqueue->prot, uhqueue->fromnick);
					break;
				case UHOST_AOP:
				case UHOST_RAOP:
					user_aop(n, u, host, uhqueue->aop, uhqueue->fromnick);
					break;
				case UHOST_CLVL:
					user_clvl(n, u, host, uhqueue->level, uhqueue->fromnick);
					break;
				case UHOST_ACCESS:
					sprintf(buf, "%s!%s@%s", n, u, host);
					do_access(uhqueue->fromnick, "", "", "", buf);
					break;
				default:
					break;
			}
			delete_from_userhost_queue(uhqueue->nick, uhqueue->fromnick, uhqueue->type);
			uhqueue = next;
		}
		else
			uhqueue = uhqueue->next;
	}
	return;
}

void
in_banned(fromnick, fromuser, fromhost, args)
char *fromnick;
char *fromuser;
char *fromhost;
char *args;
{
	if (debug)
		printf("*** Cannot join channel %s\n", args);
        return;
}

void
in_banlist(fromnick, fromuser, fromhost, args)
char *fromnick;
char *fromuser;
char *fromhost;
char *args;
{
	Channel *this;
	Banlist *bans;
	char *channel;
	char *string;
	
	next_arg(&args, args);
	channel = next_arg(&args, args);
	string = next_arg(&args, args);

	for (this = channels; this; this = this->next)
	{
		if (!strcasecmp(this->name, channel))
			break;
	}
	if (!this)
		return;
	bans = malloc(sizeof(Banlist));
	bans->ban = malloc(strlen(string) + 1);
	strcpy(bans->ban, string);
	bans->next = this->bans;
	bans->next->prev = bans;
	bans->prev = NULL;
	this->bans = bans;
        return;
}

void
in_channelmode(fromnick, fromuser, fromhost, args)
char *fromnick;
char *fromuser;
char *fromhost;
char *args;
{
	next_arg(&args, args);
	in_mode(fromnick, fromuser, fromhost, args);
        return;
}

void
in_endwho(fromnick, fromuser, fromhost, args)
char *fromnick;
char *fromuser;
char *fromhost;
char *args;
{
	if (debug)
		printf("*** End of who: %s\n", args);
        return;
}

void
in_endbanlist(fromnick, fromuser, fromhost, args)
char *fromnick;
char *fromuser;
char *fromhost;
char *args;
{
	if (debug)
		printf("*** End of banlist: %s\n", args);
        return;
}

void
in_nouserline(fromnick, fromuser, fromhost, args)
char *fromnick;
char *fromuser;
char *fromhost;
char *args;
{
	send_to_server(server_fd, "USER %s %s %s :%s", myusername,
		"hithere", "serverman", myircname);
        return;
}

void
in_nickinuse(fromnick, fromuser, fromhost, args)
char *fromnick;
char *fromuser;
char *fromhost;
char *args;
{
	char *hi;
 
	hi = mynick;
	while (*hi && *hi == '_')
		hi++;
	*hi = '_';    
	send_to_server(server_fd, "NICK %s", mynick);

	if (debug)
		printf("*** Nickname in use, now %s\n", mynick);
        return;
}

void
in_welcome(fromnick, fromuser, fromhost, args)
char *fromnick;
char *fromuser;
char *fromhost;
char *args;
{
	connected = 1;
	send_to_server(server_fd, "MODE %s +i", mynick);
	join_all_channels();
	if (debug)
		printf("*** Connected to server: %s\n", args);
        return;
}

void
in_klined(fromnick, fromuser, fromhost, args)
char *fromnick;
char *fromuser;
char *fromhost;
char *args;
{
	if (debug)
		printf("*** Banned from server: %s\n", args);
        return;
}
