/*
 * user.c - handles userlist and shitlist functions 
 *
 * (c) 1997 Eric Robbins
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cheeze.h"
#include "user.h"

char *make_hostmask();
char *make_clustermask();
void delete_auth();

void
add_user(nick, user, host, channels, level, aop, prot, passwd, fromnick)
char *nick;
char *user;
char *host;
char *channels;
char *passwd;
char *fromnick;
int aop;
int prot;
int level;
{
	Accesslist *new;
	char *tmp;

	tmp = make_clustermask(nick, user, host);

	new = malloc(sizeof(Accesslist));
	new->aop = aop;
	new->prot = prot;
	new->level = level;
	if (passwd && *passwd)
	{
		new->passwd = malloc(strlen(passwd) + 1);
		strcpy(new->passwd, passwd);
	}
	else
		new->passwd = NULL;
	if (channels && *channels)
	{
		new->channel = malloc(strlen(channels) + 1);
		strcpy(new->channel, channels);
	}
	else
		new->channel = NULL;
	new->hostmask = malloc(strlen(tmp) + 1);
	strcpy(new->hostmask, tmp);
	new->next = accesslist;
	accesslist = new;
	new->prev = NULL;
	if (new->next)
		new->next->prev = new;
	if (fromnick)
		send_to_nick(fromnick, "\002%s added to channel %s (level %i/aop %i/prot %i)\002",
			tmp, channels, level, aop, prot);
	free(tmp);
	return;
}

void
delete_user(nick, user, host, fromnick)
char *nick;
char *user;
char *host;
char *fromnick;
{
	Accesslist *access;
	char *tmp;

	tmp = make_hostmask(nick, user, host);

	for (access = accesslist; access; access = access->next)
	{
		if (wild_match(access->hostmask, tmp))
		{
			if (fromnick)
				send_to_nick(fromnick, "\002User %s has been deleted\002",
					tmp);
			free(tmp);
			free(access->hostmask);
			if (access->channel)
				free(access->channel);
			if (access->passwd)
				free(access->passwd);
			if (!access->next)
			{
				if (!access->prev)
				{
					accesslist = NULL;
					free(access);
					return;
				}
				else
				{
					access->prev->next = NULL;
					free(access);
					return;
				}
			}
			else
			{
				if (!access->prev)
				{
					accesslist = access->next;
					access->next->prev = NULL;
					free(access);
					return;
				}
				else
				{
					access->prev->next = access->next;
					access->next->prev = access->prev;
					free(access);
					return;
				}
			}
		}
	}
	if (fromnick)
		send_to_nick(fromnick, "\002No match for %s", tmp);
	free(tmp);
	return;
}

void
add_shit(nick, user, host, channel, reason, fromnick)
char *nick;
char *user;
char *host;
char *channel;
char *reason;
char *fromnick;
{
	char *tmp;
	Shitlist *new;

	tmp = make_clustermask(nick, user, host);

	new = malloc(sizeof(Shitlist));

	if (channel)
	{
		new->channel = malloc(strlen(channel) + 1);
		strcpy(new->channel, channel);
	}
	else
		new->channel = NULL;
	if (reason)
	{
		new->reason = malloc(strlen(reason) + 1);
		strcpy(new->reason, reason);
	}
	else
		new->reason = NULL;
	new->hostmask = malloc(strlen(tmp) + 1);
	strcpy(new->hostmask, tmp);
	new->prev = NULL;
	new->next = shitlist;
	shitlist = new;
	if (new->next)
		new->next->prev = new;
	if (fromnick)
		send_to_nick(fromnick, "\002%s shitted on channels %s\002",
			tmp, channel);
	free(tmp);
	return;
}

void
delete_shit(nick, user, host, fromnick)
char *nick;
char *user;
char *host;
char *fromnick;
{
	Shitlist *shit;
	char *tmp;

	tmp = make_hostmask(nick, user, host);

	for (shit = shitlist; shit; shit = shit->next)
	{
		if (wild_match(shit->hostmask, tmp))
		{
			if (fromnick)
				send_to_nick(fromnick, "\002user %s has been unshitted\002", tmp);
			if (shit->reason)
				free(shit->reason);
			if (shit->channel)
				free(shit->channel);
			free(shit->hostmask);
			free(tmp);
			if (!shit->next) /* last */
			{
				if (!shit->prev) /* and first */
				{
					shitlist = NULL;
					free(shit);
					return;
				}
				else
				{
					shit->prev->next = NULL;
					free(shit);
					return;
				}
			}
			else
			{
				if (!shit->prev)
				{
					shitlist = shit->next;
					shit->next->prev = NULL;
					free(shit);
					return;
				}
				else
				{
					shit->prev->next = shit->next;
					shit->next->prev = shit->prev;
					free(shit);
					return;
				}
			}
		}
	}
	if (fromnick)
		send_to_nick(fromnick, "\002no shitlist on user %s\002", tmp);
	free(tmp);
	return;
}

void
user_aop(nick, user, host, flag, fromnick)
char *nick;
char *user;
char *host;
int flag;
char *fromnick;
{
	Accesslist *luser;
	char *tmp;

	tmp = make_hostmask(nick, user, host);

	for (luser = accesslist; luser; luser = luser->next)
	{
		if (wild_match(luser->hostmask, tmp))
			luser->aop = flag;
	}
	if (fromnick)
		send_to_nick(fromnick, "\002user %s is now %sauto opped\002",
			tmp, (flag) ? "" : "not ");
	free(tmp);
	return;
}

void
user_prot(nick, user, host, flag, fromnick)
char *nick;
char *user;
char *host;
int flag;
char *fromnick;
{
        Accesslist *luser;
        char *tmp;

        tmp = make_hostmask(nick, user, host);

        for (luser = accesslist; luser; luser = luser->next)
        {
                if (wild_match(luser->hostmask, tmp))
                        luser->prot = flag;
        }
	if (fromnick)
		send_to_nick(fromnick, "\002User %s now protected at level %i\002",
			tmp, flag);
	free(tmp);
        return;
}

void
user_clvl(nick, user, host, flag, fromnick)
char *nick;
char *user;
char *host;
int flag;
char *fromnick;
{
        Accesslist *luser;
        char *tmp;

        tmp = make_hostmask(nick, user, host);

        for (luser = accesslist; luser; luser = luser->next)
        {
                if (wild_match(luser->hostmask, tmp))
                        luser->level = flag;
        }
	if (fromnick)
		send_to_nick(fromnick, "\002User %s is now level %i\002",
			tmp, flag);
	free(tmp);
        return;
}

void
add_to_userhost_queue(type, from, nick, channel, string, level, aop, prot)
char *from;
char *nick;
char *channel;
char *string;
int type;
int level;
int aop;
int prot;
{
	Userhost *new;

	new = malloc(sizeof(Userhost));

	new->type = type;
	new->fromnick = malloc(strlen(from) + 1);
	strcpy(new->fromnick, from);
	new->nick = malloc(strlen(nick) + 1);
	strcpy(new->nick, nick);
	new->channel = malloc(strlen(channel) + 1);
	strcpy(new->channel, channel);
	new->string = malloc(strlen(string) + 1);
	strcpy(new->string, string);
	new->level = level;
	new->aop = aop;
	new->prot = prot;
	new->next = userhostqueue;
	new->prev = NULL;
	userhostqueue = new;
	if (new->next)
		new->next->prev = new;
	send_to_server(server_fd, "USERHOST %s", nick);
	return;
}

void
delete_from_userhost_queue(nick, fromnick, type)
char *nick;
char *fromnick;
int type;
{
	Userhost *this;

	for (this = userhostqueue; this; this = this->next)
	{
		if (this->type == type && !strcasecmp(nick, this->nick) &&
			!strcasecmp(fromnick, this->fromnick))
		{
			free(this->nick);
			free(this->fromnick);
			free(this->channel);
			free(this->string);
			if (!this->next)
			{
				if (!this->prev)
				{
					userhostqueue = NULL;
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
					this->next->prev = NULL;
					userhostqueue = this->next;
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
add_auth(nick)
char *nick;
{
	Authlist *new;

	delete_auth(nick, 1);

	new = malloc(sizeof(Authlist));
	new->nick = malloc(strlen(nick) + 1);
	strcpy(new->nick, nick);
	new->prev = NULL;
	new->next = authlist;
	if (new->next)
		new->next->prev = new;

	return;
}

void
clear_auths()
{
	delete_auth("", 1);
	return;
}
	

void
delete_auth(nick, flag) /* if ! is_on_a_channel() */
char *nick;
int flag;
{
	Authlist *tmp, *next;

	if (is_on_a_channel(nick) && !flag)
		return;

	for (tmp = authlist; tmp;)
	{
		if (!strcasecmp(nick, tmp->nick) ||
		   (flag && !is_on_a_channel(tmp->nick)))
		{
			free(tmp->nick);
			if (!tmp->next) /* last */
			{
				if (!tmp->prev) /* and first */
				{
					authlist = NULL;
					free(tmp);
					return;
				}
				else
				{
					tmp->prev->next = NULL;
					free(tmp);
					return;
				}
			}
			else
			{
				if (!tmp->prev)
				{
					authlist = tmp->next;
					tmp->next->prev = NULL;
					next = tmp->next;
					free(tmp);
					if (*nick)
						return;
					tmp = next;
					continue;
				}
				else
				{
					tmp->prev->next = tmp->next;
					tmp->next->prev = tmp->prev;
					next = tmp->next;
					free(tmp);
					if (*nick)
						return;
					tmp = next;
					continue;
				}
			}
		}
		tmp = tmp->next;
	}
	free(tmp);
	return;
}

int
get_userstat(nick, user, host, channel, type)
char *nick;
char *user;
char *host;
char *channel;
{
	Accesslist *access;
	char *tmp;

	tmp = make_hostmask(nick, user, host);

	for (access = accesslist; access; access = access->next)
	{
		if ((wild_match(access->channel, channel) || !*channel) &&
		    wild_match(access->hostmask, tmp))
		{
			free(tmp);
			if (type == USER_LEVEL)
				return(access->level);
			if (type == USER_PROT)
				return(access->prot);
			if (type == USER_AOP)
				return(access->aop);
			return(0);
		}
	}
	free(tmp);
	return(0);
}

char *
is_shitted(nick, user, host, channel)
char *nick;
char *user;
char *host;
char *channel;
{
	Shitlist *shit;
	char *tmp;

	tmp = make_hostmask(nick, user, host);

	for (shit = shitlist; shit; shit = shit->next)
	{
		if (wild_match(shit->hostmask, tmp))
		{
			free(tmp);
			return(shit->hostmask);
		}
	}
	free(tmp);
	return(NULL);
}

char *
shit_reason(hostmask)
char *hostmask;
{
	Shitlist *shit;
	
	for (shit = shitlist; shit; shit = shit->next)
	{
		if (!strcasecmp(hostmask, shit->hostmask))
			return(shit->reason ? shit->reason : "");
	}
	return("");
}

int
is_on_a_channel(nick)
char *nick;
{
	Channel *chan;
	Userlist *user;

	for (chan = channels; chan; chan = chan->next)
	{
		for (user = chan->users; user; user = user->next)
		{
			if (!strcasecmp(nick, user->name))
				return(1);
		}
	}
	return(0);
}

void
process_user(nick, user, host, channel)
char *nick;
char *user;
char *host;
char *channel;
{
	char *tmp;

	if (!am_opped(channel))
		return;

	if (tmp = is_shitted(nick, user, host, channel))
	{
		send_to_server(server_fd, "MODE %s -o+b %s %s", channel, 
			nick, tmp);
		send_to_server(server_fd, "KICK %s %s :%s", channel, nick,
			shit_reason(tmp));
	}
	if (get_userstat(nick, user, host, channel, USER_LEVEL) &&
	    get_userstat(nick, user, host, channel, USER_AOP) &&
	    (is_authed(nick) || no_passwd(nick, user, host)) &&
	    !user_opped(nick, channel) && !tmp)
		send_to_server(server_fd, "MODE %s +o %s", channel, nick);
	return;
}

void
check_protect(type, nick, user, host, channel, target)
int type;
char *nick;
char *user;
char *host;
char *channel;
char *target;
{
	char *from;
	char *to;
	int fromlevel, tolevel;

	if (!am_opped(channel))
		return;

	from = make_hostmask(nick, user, host);
	to = get_userhost(target, 0);

	fromlevel = get_userstat(from, "", "", channel, USER_LEVEL);

	if (type == PROT_BAN)
	{
		if (fromlevel < 3 && check_ban(target))
		{
			send_mode = 1;
			strcat(outmode, "-b");
			strcat(outmodestring, target);
			strcat(outmodestring, " ");
			if (!fromlevel && !deop_user)
			{
				deop_user = 1;
				strcat(outmode, "-o");
				strcat(outmodestring, nick);
				strcat(outmodestring, " ");
			}
		}
	}

	if (type == PROT_DEOP)
	{
		tolevel = get_userstat(to, "", "", channel, USER_LEVEL);
		if (fromlevel < 3 && check_ban(to))
		{
			send_mode = 1;
			strcat(outmode, "+o");
			strcat(outmodestring, target);
			strcat(outmodestring, " ");
			if (!fromlevel && !deop_user)
			{
				deop_user = 1;
				strcat(outmode, "-o");
				strcat(outmodestring, nick);
				strcat(outmodestring, " ");
			}
		}
	}
	return;
}

int
check_ban(ban)
char *ban;
{
	Accesslist *this;

	for (this = accesslist; this; this = this->next)
	{
		if (wild_match(this->hostmask, ban) || 
			wild_match(ban, this->hostmask))
			return(this->prot);
	}
	return(0);
}

void
scan_channel(name)
char *name;
{
	Channel *chan;
	Userlist *user;
	char *tmp;

	for (chan = channels; chan; chan = chan->next)
	{
		if (!chan->am_opped) /* pointless */
			continue;
		if (!name || !strcasecmp(chan->name, name))
		{
			for (user = chan->users; user; user = user->next)
				process_user(user->name, user->user, user->host, chan->name);
		}
	}
	return;
}

int
is_authed(nick)
char *nick;
{
	Authlist *auth;

	for (auth = authlist; auth; auth = auth->next)
	{
		if (!strcasecmp(nick, auth->nick))
			return(1);
	}
	return(0);
}

char *
get_userhost(nick, type)
char *nick;
int type;
{
	Channel *chan;
	Userlist *user;

	strcpy(userhostbuf, "");

	for (chan = channels; chan; chan = chan->next)
	{
		for (user = chan->users; user; user = user->next)
		{
			if (!strcasecmp(nick, user->name))
			{
				if (type == 0)
				{
					sprintf(userhostbuf, "%s!%s@%s", user->name, 
						user->user, user->host);
					return(userhostbuf);
				}
				else
				{
					sprintf(userhostbuf, "*!*%s@%s", (*user->user == '~') ? (user->user) + 1 : user->user,
						cluster(user->host));
					return(userhostbuf);
				}
			}
		}
	}
	strcpy(userhostbuf, "");
	return(userhostbuf);
}

int
verify_auth(nick, user, host, passwd)
char *nick;
char *user;
char *host;
char *passwd;
{
	Accesslist *access;
	char *tmp;

	tmp = make_hostmask(nick, user, host);

	for(access = accesslist; access; access = access->next)
	{
		if (wild_match(access->hostmask, tmp))
		{
			if (!strcmp(access->passwd, passwd))
			{
				free(tmp);
				return(1);
			}
		}
	}
	free(tmp);
	return(0);
}

int
no_passwd(nick, user, host)
char *nick;
char *user;
char *host;
{
	Accesslist *access;
	char *tmp;
	
	tmp = make_hostmask(nick, user, host);

	for (access = accesslist; access; access = access->next)
	{
		if (wild_match(access->hostmask, tmp))
		{
			free(tmp);
			if (!access->passwd)
				return(1);
			else
				return(0);
		}
	}
	free(tmp);
	return(0);
}

char *
make_hostmask(nick, user, host)
char *nick;
char *user;
char *host;
{
	char *this;

	this = malloc(strlen(nick) + strlen(user) + strlen(host) + 3);
	if (user && host && *user && *host)
		sprintf(this, "%s!%s@%s", nick, user, host);
	else
		sprintf(this, "%s", nick);

	return(this);
}

char *
make_clustermask(nick, user, host)
char *nick;
char *user;
char *host;
{       
        char *this;
        
        this = malloc(strlen(nick) + strlen(user) + strlen(host) + 3);
	if (*user == '~')
		user++;
        if (user && host && *user && *host)
                sprintf(this, "*!*%s@%s", user, cluster(host));
        else            
                sprintf(this, "%s", nick);
                      
        return(this);
}
