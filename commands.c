/*
 * commands.c - parse user commands
 *
 * (c) 1997 Eric Robbins
 *
 */

#include <stdio.h>
#include "cheeze.h"
#include "user.h"

void	do_nothing();
void	do_say();
void	do_quit();
void 	do_join();
void	do_add();
void	do_save();
void	do_access();
void	do_part();
void	do_delete();
void	do_shit();
void	do_rshit();
void 	do_channels();
void	do_users();
void	do_invite();
void	do_server();
void	do_loadusers();
void	do_loadshit();
void	do_userlist();
void	do_shitlist();
void	do_chat();
void	do_hostname();
void	do_nick();
extern  void	dcc_who();
extern	void	dcc_echo();

struct commands user_commands[] = {

/* 	  COMMAND	FUNCTION	FLAG	LEVEL	CC 		*/

	{ "ACCESS",	do_access,	0,	1,	1 },
	{ "CHAT",	do_chat,	0,	1,	1 },
	{ "CHANNELS",	do_channels,	0,	3,	1 },
	{ "USERS",	do_users,	0,	3,	1 },
	{ "ECHO",	do_say,		0,	1,	1 },
	{ "SAY",	do_say,		0,	1,	1 },
	{ "SHIT",	do_shit,	0,	2,	1 },
	{ "RSHIT",	do_rshit,	0,	2,	1 },
	{ "ADD",	do_add,		0,	3,	1 },
	{ "DELETE",	do_delete,	0,	3,	1 },
	{ "JOIN",	do_join,	0,	3,	1 },
	{ "LEAVE",	do_part,	0,	3,	1 },
	{ "INVITE",	do_invite,	0,	1,	1 },
	{ "DIE",	do_quit,	0,	3,	1 },
	{ "QUIT",	do_quit,	0,	3,	1 },
	{ "SAVE",	do_save,	0,	3,	1 },
	{ "NICK",	do_nick,	0,	3,	1 },
	{ "DCC",	do_dcc,		0,	1,	1 },
	{ "SERVER",	do_server,	0,	3,	1 },
	{ "LOADUSERS",	do_loadusers,	0,	3,	1 },
	{ "LOADSHIT",	do_loadshit,	0,	3,	1 },
	{ "USERLIST",	do_userlist,	0,	3,	1 },
	{ "SHITLIST",	do_shitlist,	0,	3,	1 },
	{ "HOSTNAME",	do_hostname,	0,	3,	1 },
	{ NULL,		do_nothing,	0,	0,	0 }

};

struct commands dcc_commands[] = {

/*	  COMMAND	FUNCTION	FLAG	LEVEL	CC		*/

	{ ".WHO",	dcc_who,	0,	1,	1 },
	{ ".ECHO",	dcc_echo,	0,	1,	1 },
	{ NULL,		do_nothing,	0,	0,	0 }

};

void
do_access(nick, user, host, to, stuff)
char *nick;
char *user;
char *host;
char *to;
char *stuff;
{
	char *uh;
	char *oldnick;
	int lev, aop, prot;

	uh = next_arg(&stuff, stuff);

	if (!uh || !*uh)
		uh = nick;

	if (strchr(uh, '!') && strchr(uh, '@'))
	{
		lev = get_userstat(uh, "", "", (*to == '#') ? to : "*", USER_LEVEL);
		aop = get_userstat(uh, "", "", (*to == '#') ? to : "*", USER_AOP);
		prot = get_userstat(uh, "", "", (*to == '#') ? to : "*", USER_PROT);
	}
	else
	{
		oldnick = uh;
		uh = get_userhost(uh, 0);
		if (!*uh)
		{
			add_to_userhost_queue(UHOST_ACCESS, nick, oldnick, "", "", 0, 0, 0);
			return;
		}
		lev = get_userstat(uh, "", "", (*to == '#') ? to : "*", USER_LEVEL);
		aop = get_userstat(uh, "", "", (*to == '#') ? to : "*", USER_AOP);
		prot = get_userstat(uh, "", "", (*to == '#') ? to : "*", USER_PROT);
	}
	send_to_nick(nick, "\002Access for %s on channel %s: level %i/aop %i/prot %i\002", uh, 
		(*to == '#') ? to : "*", lev, aop, prot); 
	return;
}

void
try_command(c, nick, user, host, to, args)
char *c;
char *nick;
char *user;
char *host;
char *to;
char *args;
{
	int i;
	int level;

	if (debug)
		printf("Trying command %s %s from %s\n", c, args, nick);

	if (*c == commandchar)
	c++;

	for (i = 0; ; i++)
	{
		if (!user_commands[i].name)
			return;
		if (!strcasecmp(user_commands[i].name, c))
			break;
	}

	level = get_userstat(nick, user, host, "", USER_LEVEL);
	if (level >= user_commands[i].level)
		user_commands[i].func(nick, user, host, to, args, user_commands[i].flag);
	else
		if (level > 0)
			send_to_nick(nick, "\002douchebag.. you dont have enough access\002");
	return;
}

void
try_dcc_command(c, nick, user, host, to, args)      
char *c;
char *nick;
char *user;
char *host;
char *to; 
char *args;
{
	int i;
	int level;

	if (debug)
		printf("Trying DCC command %s %s from %s\n", c, args, nick);

	for (i = 0; ; i++)
	{
		if (!dcc_commands[i].name)
		{
			send_to_nick(nick, "\002get help.\002");
			return;
		}
		if (!strcasecmp(dcc_commands[i].name, c))
			break;
	}

	level = get_userstat(nick, user, host, "", USER_LEVEL);
	if (level >= dcc_commands[i].level)
		dcc_commands[i].func(nick, user, host, to, args, dcc_commands[i].flag);
	else
		if (level > 0)
			send_to_nick(nick, "\002douchebag.. you dont have enough access\002");
	return;
}

void
do_hostname(nick, user, host, to, stuff)       
char *nick;
char *user;
char *host;
char *to;  
char *stuff;
{
	char *newhost;

	newhost = next_arg(&stuff, stuff);

	if (!newhost || !*newhost)
		send_to_nick(nick, "\002My hostname is: %s", myhostname);
	else
	{
		strcpy(myhostname, newhost);
		send_to_nick(nick, "\002My hostname is now: %s", myhostname);
		send_to_server(server_fd, "QUIT :\002trying new hostname..\002");
		reconnect_to_server();
	}
	return;
}

void
do_nick(nick, user, host, to, stuff)
char *nick;
char *user;
char *host;
char *to; 
char *stuff;
{
	char *newnick;

	newnick = next_arg(&stuff, stuff);

	if (!newnick || !*newnick)
		send_to_nick(nick, "\002My nickname is: %s", mynick);
	else
	{
		strcpy(mynick, newnick);
		send_to_nick(nick, "\002My nickname is now: %s", mynick);
		send_to_server(server_fd, "NICK %s", mynick);
	}
	return;
}

void
do_loadusers(nick, user, host, to, stuff)
char *nick;
char *user;
char *host;
char *to;
char *stuff;
{
	load_users();
	send_to_nick(nick, "\002Userfile %s.userlist loaded\002", mynick);
	return;
}

void
do_loadshit(nick, user, host, to, stuff)
char *nick;
char *user;
char *host;
char *to;
char *stuff;
{
	load_shit();
	send_to_nick(nick, "\002Shitlist file %s.shitlist loaded\002", mynick);
	return;
}

void
do_chat(nick, user, host, to, stuff)
char *nick;
char *user; 
char *host;
char *to;       
char *stuff;    
{               
	if (has_dcc(nick))
	{
		send_to_nick(nick, "\002You are already DCC chatting me\002");
		return;
	}

	initiate_dcc(nick, user, host);
	return;
}

void
do_nothing()
{
	return;
}

void
do_users(nick, user, host, to, stuff)
char *nick;
char *user;
char *host;
char *to;
char *stuff;
{
	char *chan;
	Channel *this;
	Userlist *users;

	chan = next_arg(&stuff, stuff);

	if (!chan || !*chan)
		chan = to;

	if (*chan != '#')
	{
		send_to_nick(nick, "\002Usage: users <channel>\002");
		return;
	}

	if (!has_dcc(nick))
	{
		send_to_nick(nick, "\002You must use DCC for this command\002");
		return;
	}

	if (!get_userstat(nick, user, host, chan, USER_LEVEL))
	{
		send_to_nick(nick, "\002You don't have enough access on channel %s\002",
			chan);
		return;
	}

	this = find_channel(chan);
	if (!this)
	{
		send_to_nick(nick, "\002I'm not on channel %s\002", chan);
		return;
	}

	send_to_nick(nick, "\002Users on channel %s:", chan);

	for (users = this->users; users; users = users->next)
		send_to_nick(nick, "\002Level: %i %s %-10s (%s@%s)\002",
			users->level, (users->modes & UMODE_O) ? "+o" : "  ",
			users->name, users->user, users->host);

	send_to_nick(nick, "\002End of list\002");	
	
	return;
}

void
do_userlist(nick, user, host, to, stuff)
char *nick;
char *user;
char *host;
char *to;
char *stuff;
{
	Accesslist *this;

	if (!has_dcc(nick))
	{
		send_to_nick(nick, "\002You must use DCC for this command\002");
		return;
	}

	send_to_nick(nick, "\002Userlist:\002");
	
	for (this = accesslist; this; this = this->next)
		send_to_nick(nick, "\002%30s (level %i/aop %i/prot %i), channel %s\002", 
			this->hostmask, this->level, this->aop, this->prot, 
			this->channel);

	send_to_nick(nick, "\002End of userlist\002");

	return;
}

void
do_shitlist(nick, user, host, to, stuff)
char *nick;
char *user;
char *host;
char *to;
char *stuff;
{
	Shitlist *this;

	if (!has_dcc(nick))
	{
		send_to_nick(nick, "\002You must use DCC for this command\002");
		return;
	}

	send_to_nick(nick, "\002Shitlist:\002");

	for (this = shitlist; this; this = this->next)
		 send_to_nick(nick, "\002%30s Channel: %12s Reason: %s\002",
			this->hostmask, this->channel ? this->channel : "*", 
			this->reason ? this->reason : "");

	send_to_nick(nick, "\002End of shitlist\002");

	return;
}

void
do_server(nick, user, host, to, stuff)
char *nick;
char *user;
char *host;
char *to;
char *stuff;
{
	char *serv;
	char *port;
	int p;

	serv = next_arg(&stuff, stuff);
	port = next_arg(&stuff, stuff);

	p = atoi(port);
	if (!p)
		p = 6667;

	send_to_server(server_fd, "QUIT :\002Trying new server: %s port %i\002",
		serv, p);

	new_server(serv, p, 1);
	return;
}

void
do_invite(nick, user, host, to, stuff)
char *nick;
char *user;
char *host;
char *to;
char *stuff;
{
	Channel *this;
	char *chan;
	char *inick;

	chan = next_arg(&stuff, stuff);

	if (*chan != '#')
	{
		inick = chan;
		chan = next_arg(&stuff, stuff);
	}
	else
		inick = nick;

	if (!chan || !*chan || *chan != '#')
	{
		send_to_nick(nick, "\002Usage: INVITE [nick] <channel>\002");
		return;
	}

	this = find_channel(chan);
	if (!this)
	{
		send_to_nick(nick, "\002I'm not on channel %s\002", chan);
		return;
	}
	send_to_server(server_fd, "INVITE %s %s", inick, chan);
	send_to_nick(nick, "\002%s has been invited to channel %s", inick, chan);
	if (this->key)
		send_to_nick(inick, "\002Key for channel %s is: %s\002", 
			chan, this->key);
	return;
}

void
do_channels(nick, user, host, to, stuff)
char *nick;
char *user;
char *host;
char *to;
char *stuff;
{
	Channel *chan;
	char modebuf[64];
	int key, limit;

	for (chan = channels; chan; chan = chan->next)
	{
		strcpy(modebuf, "+");
		limit = key = 0;

		if (chan->mode & MODE_P)
			strcat(modebuf, "p");
		if (chan->mode & MODE_S)
			strcat(modebuf, "s");
		if (chan->mode & MODE_T)
			strcat(modebuf, "t");
		if (chan->mode & MODE_I)
			strcat(modebuf, "i");
		if (chan->mode & MODE_N)
			strcat(modebuf, "n");
		if (chan->mode & MODE_L)
		{
			limit = 1;
			strcat(modebuf, "l");
		}
		if (chan->mode & MODE_K)
		{
			key = 1;
			strcat(modebuf, "k");
		}
		if (limit)
		{
			strcat(modebuf, " ");
			sprintf(modebuf, "%s %i", modebuf, chan->limit);
		}
		if (key)
		{
			strcat(modebuf, " ");
			strcat(modebuf, chan->key);
		}

		send_to_nick(nick, "\002Channel: %-15s Users: %-4i Mode: %s\002",
			chan->name, chan->num_users, modebuf);

	}
	return;
}

void
do_quit(nick, user, host, to, stuff)
char *nick;
char *user;
char *host;
char *to;
char *stuff;
{
	do_save(nick, user, host, to, stuff);
	if (stuff && *stuff)
		send_to_server(server_fd, "QUIT :%s", stuff);
	else
		send_to_server(server_fd, "QUIT :\002Killed by %s\002",
			nick);
	sleep(5);
	exit(0);
}

void
do_say(nick, user, host, to, stuff)
char *nick;
char *user;
char *host;
char *to;
char *stuff;
{
	char *n;

	n = nick;

	if (*to == '#')
		n = to;
	else if (*stuff == '#')
		n = next_arg(&stuff, stuff);
	
	send_to_server(server_fd, "PRIVMSG %s :%s", n, stuff);
	return;
}

void
do_join(nick, user, host, to, stuff)
char *nick;
char *user;
char *host;
char *to;
char *stuff;
{
	char *channel;
	char *key;

	channel = next_arg(&stuff, stuff);
	key = next_arg(&stuff, stuff);

	if (channel && *channel)
		send_to_server(server_fd, "JOIN %s %s", channel, (key && *key) ? key : "");
	else
		send_to_nick(nick, "\002I need a channel to join..\002");
	return;
}

void
do_part(nick, user, host, to, stuff)
char *nick;
char *user;
char *host;
char *to;
char *stuff;
{
	char *channel;

	channel = next_arg(&stuff, stuff);

	if (*to == '#' && (!channel || !*channel))
		send_to_server(server_fd, "PART %s", to);
	else if (*channel == '#')
		send_to_server(server_fd, "PART %s", channel);
	else
		send_to_nick(nick, "\002I need a channel to leave..\002");
	return;
}

void    
do_rshit(nick, user, host, to, stuff, flag)    
char *nick;
char *user;
char *host;
char *to; 
char *stuff;
int flag; 
{ 
        char *who;      
        char *thenick;

        who = next_arg(&stuff, stuff); 

        if (!who || !*who)
        {
                send_to_nick(nick, "\002Usage: RSHIT <nick|nick!user@host>\002");
                return;
        }

        if (strchr(who, '!') && strchr(who, '@'))
        {  
                delete_shit(who, "", "", nick);
                scan_channel(NULL);
                return;
        }
        else
        {
                thenick = who;
                who = get_userhost(who, 1);
                if (!*who)
                {
                        add_to_userhost_queue(UHOST_RSHIT, nick, thenick, "", "", 0, 0, 0);
                        return;
                }
        }       
        delete_shit(who, "", "", nick);
        scan_channel(NULL);
        return;
}

void
do_delete(nick, user, host, to, stuff, flag)
char *nick;
char *user;
char *host;    
char *to;
char *stuff;
int flag;
{
	char *who;
	char *thenick;

	who = next_arg(&stuff, stuff);

	if (!who || !*who)
	{
		send_to_nick(nick, "\002Usage: DEL <nick|nick!user@host>\002");
		return;
	}

        if (strchr(who, '!') && strchr(who, '@'))
        {
                delete_user(who, "", "", nick);
                scan_channel(NULL);
                return;
        }
        else
        {
                thenick = who;
                who = get_userhost(who, 1);
                if (!*who)
                {
                        add_to_userhost_queue(UHOST_DELETE, nick, thenick, "", "", 0, 0, 0);
                        return;
                }
        }
        delete_user(who, "", "", nick);
        scan_channel(NULL);
        return;
}

void
do_shit(nick, user, host, to, stuff, flag)
char *nick;   
char *user;
char *host;   
char *to;
char *stuff;
int flag;
{
        char *channel;
        char *who;
        char *reason;
        char *thenick;

        channel = next_arg(&stuff, stuff);
        who = next_arg(&stuff, stuff);
	if (stuff && *stuff)
		reason = stuff;
	else
		reason = "";

        if ((strcmp(channel, "*") && *channel != '#') || !*who)
        {
                send_to_nick(nick, "\002usage: SHIT <*|#channel> <nick|nick!user@host> [reason]\002");
                return;
        }

        if (strchr(who, '!') && strchr(who, '@'))
        {
                add_shit(who, "", "", channel, reason, nick);
                scan_channel(NULL);
                return;
        }
        else
        {
                thenick = who;
                who = get_userhost(who, 1);
                if (!*who)
                {
                        add_to_userhost_queue(UHOST_SHIT, nick, thenick, channel, reason, 0, 0, 0);
                        return;
                }
        }
        add_shit(who, "", "", channel, reason, nick);
        scan_channel(NULL);
        return;
}

void
do_add(nick, user, host, to, stuff, flag)
char *nick;     
char *user;
char *host;     
char *to;
char *stuff;
int flag;
{
	char *channel;
	char *who;
	char *level;
	char *prot;
	char *aop;
	char *passwd;
	char *thenick;
	int l, p, a;

	channel = next_arg(&stuff, stuff);
	who = next_arg(&stuff, stuff);
	level = next_arg(&stuff, stuff);
	aop = next_arg(&stuff, stuff);
	prot = next_arg(&stuff, stuff);
	passwd = next_arg(&stuff, stuff);

	l = atoi(level);
	p = atoi(prot);
	a = atoi(aop);

	if ((strcmp(channel, "*") && *channel != '#') || !l || !*who)
	{
		send_to_nick(nick, "\002usage: ADD <*|#channel> <nick|nick!user@host> <level> [aop] [prot] [passwd]\002");
		return;
	}

	if (get_userstat(nick, user, host, channel, USER_LEVEL) < 3)
	{
		send_to_nick(nick, "\002You don't have enough access on channel %s\002",
			channel);
		return;
	}
	if (strchr(who, '!') && strchr(who, '@'))
	{
		add_user(who, "", "", channel, l, a, p, passwd, nick);
		scan_channel(NULL);
		return;
	}
	else 
	{
		thenick = who;
		who = get_userhost(who, 1);
		if (!*who)
		{
			add_to_userhost_queue(UHOST_ADD, nick, thenick, channel, passwd, l, a, p);
			return;
		}
	}
	add_user(who, "", "", channel, l, a, p, passwd, nick);
	scan_channel(NULL);
	return;
}

void
do_save(nick, user, host, to, stuff)
char *nick;
char *user;
char *host;
char *to;
char *stuff;
{
	FILE *outfile;
	char name[80];
	Accesslist *this;
	Shitlist *shit;

	sprintf(name, "%s.userlist", mynick);
	outfile = fopen(name, "w");

	for (this = accesslist; this; this = this->next)
	{
		fprintf(outfile, "%50s %12s %i %i %i %s\n", this->hostmask,
			this->channel, this->level, this->aop, this->prot,
			(this->passwd) ? this->passwd : "");
	}
	fclose(outfile);

	sprintf(name, "%s.shitlist", mynick);
	outfile = fopen(name, "w");

	for (shit = shitlist; shit; shit = shit->next)
	{
		fprintf(outfile, "%50s %12s %s\n", shit->hostmask, 
			shit->channel, (shit->reason) ? shit->reason : "");
	}
	fclose(outfile);
	send_to_nick(nick, "\002All lists saved\002");
	return;
}
