#pragma once

enum radio_type
{
	rt_command,
	rt_fith,
	rt_info
};

struct radiomenu_t;
struct radiocommand_t;

struct hudopt_t
{
	phrase_t		phrase;
	hudtextparms_t	parms;
};

struct AString
{
	const char* chars;
	size_t length;
};

struct amxxplayer_t
{
	bool vgui;
	bool teamIdsInitialized;

	float time;
	float playtime;
	float menuexpire;

	struct
	{
		int ammo;
		int clip;
	} weapons[MAX_WEAPONS];

	int current;
	int teamId;
	int deaths;
	int aiming;
	int menu;
	int keys;
	int index;
	int flags[32];

	int death_headshot;
	int death_killer;
	int death_victim;
	bool death_tk;
	AString death_weapon;
	int newmenu;
	int page;

	float channels[5];
};

class CPlayer
{
public:
	CPlayer();

	void init(edict_t* ed);
	void putInServer();
	void disconnect();
	void updateZone();
	void showPosition();
	bool usePressed();
	void resetPositionData();

	bool isInGame() const;
	bool isHasC4() const;
	bool isAlive() const;
	bool isSpectator() const;

	entvars_t*	getPev() const;
	edict_t*	getEdict() const;
	size_t		getIndex() const;
	Vector&		getOrigin() const;
	CsTeams		getTeam() const;
	size_t		getZone();
	lang_t		getLang() const;

	void loadOptions(char* info);
	void saveOptions() const;
	void toggleIgnoreRadio();
	void radio(const radiocommand_t* rc);
	void radioThrowGrenade(size_t grenade_type);

	void showHud(hudtextparms_t& parms, float fadeOut, const char* message) const;
	void showMenu(radiomenu_t* menu) const;
	void showMenu(int keys, const char* text, size_t len, _Menu menu) const;
	void showOptions() const;
	bool onMenuselect(const int option);
	void changeOption(const int optId);

	union options_u
	{
		size_t integer;
		struct
		{
			size_t hudpos : 4;
			size_t block_radio : 1;
			size_t block_fith : 1;
		};
	};

private:
	const char* getPosition(lang_t lang);
	size_t getSpecMode() const;
	size_t getSpecTarget() const;
	int nextHUDChannel() const;
	bool needSendRadio(CsTeams sender_team, radio_type type) const;
	void blockGameDllRadio() const;
	void radioCommand(phrase_t& sound, phrase_t& text);
	void radioAimReport(size_t aimzone);

	template<typename T>
	T getPData(ptrdiff_t offset, T def) const { return ::getPData<T>(m_edict->pvPrivateData, offset, EXTRAOFFSET_PLAYER, def); }
	template<typename T>
	void setPData(ptrdiff_t offset, T value) const { ::setPData<T>(m_edict->pvPrivateData, offset, EXTRAOFFSET_PLAYER, value); }

	entvars_t* m_pev;
	edict_t* m_edict;
	bool m_ingame;
	lang_t m_lang;
	options_u m_options; // 6 bit max
	size_t m_lastZoneId;
	size_t m_leavedZoneId;
	float m_lastZoneTime;
	float m_lastReport;
	float m_lastRadio;
	amxxplayer_t* m_amxx;
};

class CIzPlayers
{
public:
	CIzPlayers();;

	void init(edict_t* ed, size_t maxplayers);
	void updateHud();
	void resetPositionsData();

	size_t   getMaxClients() const;

	CPlayer* getPlayer(const edict_t* ed);
	CPlayer* getPlayer(size_t index);
	size_t   getAnyPlayer(CsTeams team) const;
	size_t   getIndex(const CPlayer* player);
	CPlayer* getBomber();

	CPlayer& operator[](size_t index);

private:
	size_t m_maxplayers;
	CPlayer m_players[32];
};

extern CIzPlayers g_players;

void resetHudopts();
void addHudopt(translation_t* translations, size_t translations_count, float x, float y, byte red, byte green, byte blue);
