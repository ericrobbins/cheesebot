/*
 * cheeze.h - the cheeziest!@#
 *
 * (c) 1997 Eric Robbins
 * 
 */

#include "struct.h"
#include "commands.h"

#define BUFFER_SIZE 2048

extern Channel *channels;
extern Shitlist *shitlist;
extern Accesslist *accesslist;
extern Authlist *authlist;
extern Userhost *userhostqueue;
extern Dcclist *dcclist;

extern char *fromnick;
extern char *fromhost;
extern char *fromuser;

extern char mynick[];
extern char myusername[];
extern char myircname[];
extern char myserver[];
extern char mychannel[];
extern char mylogfile[];
extern char myhostname[];
extern char result[];
extern char userhostbuf[];
extern char outmode[];
extern char outmodestring[];

extern char commandchar;

extern int server_fd;
extern int debug;
extern int connected;
extern int send_mode;
extern int deop_user;
extern int dcc_command;
extern long thetime;

extern char *next_arg();
extern void read_config();
extern void send_to_server();
extern void send_to_nick();
extern char *getkey();
extern char *cluster();
extern int  wild_match();
extern Channel* find_channel();
extern void delete_auth();
extern void clear_auths();
extern void try_command();
extern char *get_userhost();
extern void do_dcc();
extern Dcclist *find_dcc();

extern void add_to_userhost_queue();
