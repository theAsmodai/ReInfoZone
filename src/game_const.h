#pragma once

enum
{
#ifdef _WIN32
	EXTRAOFFSET_PLAYER = 0
#else
	EXTRAOFFSET_PLAYER = 5
#endif
};

enum
{
#ifdef _WIN32
	EXTRAOFFSET_ITEM = 0
#else
	EXTRAOFFSET_ITEM = 4
#endif
};

enum CsTeams : size_t
{
	CS_TEAM_UNASSIGNED,
	CS_TEAM_T,
	CS_TEAM_CT,
	CS_TEAM_SPECTATOR
};

enum
{
	OBS_NONE,
	OBS_CHASE_LOCKED,
	OBS_CHASE_FREE,
	OBS_ROAMING,
	OBS_IN_EYE,
	OBS_MAP_FREE,
	OBS_MAP_CHASE
};

typedef enum : size_t
{
	Menu_OFF,
	Menu_ChooseTeam,
	Menu_IGChooseTeam,
	Menu_ChooseAppearance,
	Menu_Buy,
	Menu_BuyPistol,
	Menu_BuyRifle,
	Menu_BuyMachineGun,
	Menu_BuyShotgun,
	Menu_BuySubMachineGun,
	Menu_BuyItem,
	Menu_Radio1,
	Menu_Radio2,
	Menu_Radio3,
	Menu_ClientBuy,
	Menu_InfoZone = 'iz'

} _Menu;

#define MENU_KEY_1			(1<<0)
#define MENU_KEY_2			(1<<1)
#define MENU_KEY_3			(1<<2)
#define MENU_KEY_4			(1<<3)
#define MENU_KEY_5			(1<<4)
#define MENU_KEY_6			(1<<5)
#define MENU_KEY_7			(1<<6)
#define MENU_KEY_8			(1<<7)
#define MENU_KEY_9			(1<<8)
#define MENU_KEY_0			(1<<9)
