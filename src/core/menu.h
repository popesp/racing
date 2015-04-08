#ifndef MENU
#define	MENU

#define	MENU_FLAG_STARTGAME		0x01
#define	MENU_FLAG_SETTINGS		0x02
#define	MENU_FLAG_CREDITS		0x04
#define	MENU_FLAG_EXITGAME		0x08
#define	MENU_FLAG_SHUTDOWN		0x10
#define MENU_FLAG_INCREDITS		0x20
#define MENU_FLAG_INSETTINGS	0x40
#define MENU_FLAG_SOUND			0x80
#define MENU_FLAG_NUMAI			0x1b
#define MENU_FLAG_NUMLAPS		0x10
#define MENU_FLAG_DIFFICULTY	0x20

int menu_startup(struct game* game);

#endif
