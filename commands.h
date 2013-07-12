/* 
 * commands.h - definition for commands.. 
 *
 * (c) 1997 Eric Robbins
 *
 */

struct commands {
	char *name;
	void (*func)();
	int  flag;
	int  level;
	int  requirecc;
	};


