/*
 * struct.h - structs. duh.
 *
 * (c) 1997 Eric Robbins
 *
 */

#define MODE_N	1
#define MODE_T	2
#define MODE_S  4
#define	MODE_P	8
#define MODE_K	16
#define	MODE_L	32
#define	MODE_I	64

#define UMODE_O 1
#define UMODE_V 2

#define USER_LEVEL 1
#define USER_PROT 2
#define USER_AOP 3

#define LEVEL_USER 1
#define LEVEL_MASTER 2
#define LEVEL_OWNER 3
#define LEVEL_MAX 3

#define PROT_BAN 1
#define PROT_DEOP 2

struct server_stuff {
	char *text;
	void (*func)();
	};

struct channellist {
	char *name;
	char mode;
	char *key;
	int  limit;
	short am_opped;
	short num_users;
	short on_channel;
	long lastjoin;
	struct banlist *bans;
	struct userlist *users;
	struct channellist *next;
	struct channellist *prev;
	};

struct userlist {
	char *name;
	char *user;
	char *host;
	char modes;
	int level;
	struct userlist *next;
	struct userlist *prev;
	};

struct banlist {
	char *ban;
	struct banlist *next;
	struct banlist *prev;
	};

struct accesslist {
	char *hostmask;
	char *passwd; /* shit reason in shitlist */
	char *channel;
	int level;
	int aop;
	int prot;
	struct accesslist *next;
	struct accesslist *prev;
	};

struct shitlist {
	char *hostmask;
	char *reason;
	char *channel;
	struct shitlist *next;
	struct shitlist *prev;
	};

struct userhostlist {
	char *nick;
	char *fromnick;
	char *channel;
	char *string;
	int level;
	int aop;
	int prot;
	int type;
	struct userhostlist *next;
	struct userhostlist *prev;
	};

struct authlist {
	char *nick;
	struct authlist *next;
	struct authlist *prev;
	};

struct dcclist {
	char *nick;
	char *user;
	char *host;
	int fd;
	int bot;
	int connected;
	long offertime;
	char lastbuf[2048]; /* cuz I can */
	struct dcclist *next;
	struct dcclist *prev;
	};

struct serverlist {
	char *name;
	int port;
	struct serverlist *next;
	};

typedef struct authlist Authlist;
typedef struct accesslist Accesslist;
typedef struct shitlist Shitlist;
typedef struct userlist Userlist;
typedef struct banlist Banlist;
typedef struct channellist Channel;
typedef struct userhostlist Userhost;
typedef struct dcclist Dcclist;
typedef struct serverlist Serverlist;
