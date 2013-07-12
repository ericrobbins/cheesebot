/*
 * channel.c - keeps track of channels & users 
 *
 * (c) 1997 Eric Robbins
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cheeze.h"

Channel *find_channel();

void
add_channel(channel, flag)
char *channel;
int flag;
{
	Channel *new;

	new = malloc(sizeof(Channel));

	new->name = malloc(strlen(channel) + 1);
	strcpy(new->name, channel);
	new->users = NULL;
	new->bans = NULL;
	new->key = NULL;
	new->limit = 0;
	new->mode = 0;
	new->am_opped = 0;
	new->num_users = 0;
	new->prev = NULL;
	new->next = channels;
	new->on_channel = flag;
	new->lastjoin = thetime;
	if (new->next)
		new->next->prev = new;
	channels = new;

	if (debug)
		printf("### Added new channel %s\n", channel);
	return;
}

void
delete_channel(channel, flag)
char *channel;
int flag;
{
	Channel *this;
	Banlist *bans, *bnext;
	Userlist *users, *unext;

	this = find_channel(channel);
	if (!this)
		return;

	if (this->bans)
	{
		for (bans = this->bans; bans;)
		{
			if (bans->ban)
				free(bans->ban);
			bnext = bans->next;
			free(bans);
			bans = bnext;
		}
	}
	if (this->users)
	{
		for (users = this->users; users;)
		{
			if (users->name)
				free(users->name);
			if (users->host)
				free(users->host);
			if (users->user)
				free(users->user);
			unext = users->next;
			free(users);
			users = unext;
		}
	}
	if (!flag)
	{
		this->bans = NULL;
		this->users = NULL;
		this->on_channel = 0;
		this->lastjoin = thetime;
		return;
	}
	if (this->key)
		free(this->key);
	if (this->name)
		free(this->name);

        if (debug)
                printf("### Deleted channel %s\n", channel);

	if (!this->next)
	{
		if (!this->prev)
		{
			channels = NULL;
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
			channels = this->next;
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
	return;
}

void
add_user_to_channel(channel, nick, user, host, flags)
char *channel;
char *nick;
char *user;
char *host;
int flags;
{
	Userlist *new;
	Channel *chan;

	chan = find_channel(channel);
	if (!chan)
		return;

	new = malloc(sizeof(Userlist));
	new->name = malloc(strlen(nick) + 1);
	strcpy(new->name, nick);
	new->user = malloc(strlen(user) + 1);
	strcpy(new->user, user);
	new->host = malloc(strlen(host) + 1);
	strcpy(new->host, host);
	new->modes = flags;
	new->next = chan->users;
	if (new->next)
		new->next->prev = new;
	new->prev = NULL;
	chan->users = new;
	chan->num_users++;
	if (!strcasecmp(mynick, new->name) && new->modes & UMODE_O)
		chan->am_opped = 1;
	new->level = get_userstat(nick, user, host, channel, USER_LEVEL);
	if (debug)
		printf("### Added user %s!%s@%s to channel %s\n",
			new->name, new->user, new->host, chan->name);
	return;
}

void
delete_user_from_channel(channel, nick)
char *channel;
char *nick;
{
	Channel *chan;
	Userlist *user;

        chan = find_channel(channel);
        if (!chan)
                return;

	for (user = chan->users; user; user = user->next)
	{
		if (!strcmp(user->name, nick))
			break;
	}
	if (!user)
		return;

	if (user->name)
		free(user->name);
	if (user->user)
		free(user->user);
	if (user->host)
		free(user->host);

	if (chan->users == user)
	{
		chan->users = user->next;
		if (user->next)
			user->next->prev = NULL;
	}
	else
	{
		if (user->next)
		{
			user->next->prev = user->prev;
			user->prev->next = user->next;
		}
		else
		{
			user->prev->next = NULL;
		}
	}
	free(user);
	chan->num_users--;
	if (debug)
		printf("### Deleted user %s from channel %s\n", nick, channel);
	return;
}

void
delete_from_all_channels(nick)
char *nick;
{
	Channel *chan;

        for (chan = channels; chan; chan = chan->next)
		delete_user_from_channel(chan->name, nick);

	if (debug)
		printf("### Deleted user %s from all channels\n",
			nick);
	return;
}

void
change_all_nick(oldnick, newnick)
char *oldnick;
char *newnick;
{
	Channel *chan;
	Userlist *user;
	Authlist *auth;
	Dcclist *dcc;

	for (chan = channels; chan; chan = chan->next)
	{
		for (user = chan->users; user; user = user->next)
		{
			if (!strcasecmp(user->name, oldnick))
			{
				free(user->name);
				user->name = malloc(strlen(newnick) + 1);
				strcpy(user->name, newnick);
			}
		}
	}

	for (auth = authlist; auth; auth = auth->next)
	{
		if (!strcasecmp(auth->nick, oldnick))
		{
			free(auth->nick);
			auth->nick = malloc(strlen(newnick) + 1);
			strcpy(auth->nick, newnick);
		}
	}
	for (dcc = dcclist; dcc; dcc = dcc->next)
	{
		if (!strcasecmp(dcc->nick, oldnick))
		{
			free(dcc->nick);
			dcc->nick = malloc(strlen(newnick) + 1);
			strcpy(dcc->nick, newnick);
		}
	}
	return;
}

void
add_channel_ban(channel, ban)
char *channel;
char *ban;
{
	Channel *chan;
	Banlist *new;

        chan = find_channel(channel);
        if (!chan)
                return;

	new = malloc(sizeof(Banlist));
	new->ban = malloc(strlen(ban) + 1);
	strcpy(new->ban, ban);
	new->next = chan->bans;
	new->prev = NULL;
	if (new->next)
		new->next->prev = new;
	chan->bans = new;	

	if (debug)
		printf("### Added ban to %s: %s\n", channel, ban);
	return;
}

void
delete_channel_ban(channel, theban)
char *channel;
char *theban;
{

        Channel *chan;
        Banlist *ban;

        chan = find_channel(channel);
        if (!chan)
                return;

        for (ban = chan->bans; ban; ban = ban->next)
        {
                if (!strcmp(ban->ban, theban))
                        break;
        }
        if (!ban)
                return;

        if (ban->ban)
                free(ban->ban);

        if (chan->bans == ban)
        {
                chan->bans = ban->next;
                if (ban->next)
                        ban->next->prev = NULL;
        }
        else
        {
                if (ban->next)
                {
                        ban->next->prev = ban->prev;
                        ban->prev->next = ban->next;
                }
                else
                {
                        ban->prev->next = NULL;
                }
        }
        free(ban);
        if (debug)
                printf("### Deleted ban %s from channel %s\n", theban, channel);
        return;
}

void
join_all_channels()
{
	Channel *this;

	for (this = channels; this; this = this->next)
		send_to_server(server_fd, "JOIN %s %s", this->name,
			this->key ? this->key : "");
	return;
}

Channel *
find_channel(name)
char *name;
{
	Channel *this;
	
	for (this = channels; this; this = this->next)
		if (!strcasecmp(this->name, name))
			return(this);
	return(NULL);
}

char *
getkey(channel)
char *channel;
{
	Channel *this;

	for(this = channels; this; this = this->next)
		if (!strcasecmp(this->name, channel))
			return(this->key);
	return(NULL);
}

int
am_opped(name)
char *name;
{
	Channel *this;

	for (this = channels; this; this = this->next)
		if (!strcasecmp(name, this->name))
		{
			if (this->am_opped)
				return(1);
			else
				return(0);
		}
	return(0);
}

int
user_opped(nick, channel)
char *nick;
char *channel;
{
	Channel *this;
	Userlist *user;

	for (this = channels; this; this = this->next)
	{
		if (!strcasecmp(this->name, channel))
		{
			for (user = this->users; user; user = user->next)
			{
				if (!strcasecmp(nick, user->name))
				{
					if (user->modes & UMODE_O)
						return(1);
					else
						return(0);
				}
			}
		}
	}
	return(0);
}

void
func_debug()
{
	Channel *this;
	for (this = channels; this; this = this->next)
	{
		printf("Channel: %s\n", this->name);
		printf("channel mode is: +%s%s%s%s%s%s%s %i %s\n", 
			this->mode & MODE_S ? "s" : "",
			this->mode & MODE_T ? "t" : "",
			this->mode & MODE_I ? "i" : "",
			this->mode & MODE_N ? "n" : "",
			this->mode & MODE_P ? "p" : "",
			this->mode & MODE_L ? "l" : "",
			this->mode & MODE_K ? "k" : "",
			this->mode & MODE_L ? this->limit : 0,
			this->mode & MODE_K ? this->key : "");
		printf("there are %i users\n", this->num_users);
		printf("I am %sopped\n", this->am_opped ? "" : "not ");
		printf("I am %son the channel\n", this->on_channel ? "" : "not ");
	}
	return;
}
