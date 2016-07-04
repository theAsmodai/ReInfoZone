#pragma once

enum grenadetype_e
{
	gt_explosive,
	gt_flash,
	gt_smoke
};

enum bombstate_e
{
	bs_unknown,
	bs_carried,
	bs_dropped,
	bs_planted
};

struct bombdata_t
{
	bombstate_e	state;
	entvars_t*	owner;
	size_t		lastZone;
	Vector		lastPos;
};

enum { UNKNOWN_RADIO = -1 };

enum
{
	rp_menu,
	rp_text,
	rp_sound,
	rp_text2,
	rp_sound2,
};

struct radiocommand_t
{
	const char* command;
	phrase_t*	phrases;
	uint8		menuid;
	bool		mult;
};

struct radiomenu_t
{
	radiomenu_t(size_t menuid, lang_t lng, const char* txt, int keys);
	size_t id;
	lang_t lang;
	int keybits;
	const char* text;
	size_t len;
};

class CGame
{
public:
	CGame();

	size_t indexOfEdict(const edict_t* ed) const;
	edict_t* edictByIndex(size_t index) const;
	edict_t* getLastLogEdict();
	void setLastLogEdict(edict_t* ed);
	bool isGameActive() const;
	bool isMapActive() const;
	bombstate_e getBombState() const;
	bombdata_t& getBombData();
	
	void mapStart(edict_t* edicts, size_t maxclients);
	void mapEnd();

	void roundRestart();
	void roundStart();
	void roundEnd();

	void bombCollected();
	void bombDropped(CPlayer* player);
	void bombPicked(CPlayer* player);
	void bombPlanted(CPlayer* player);

	void updateBombPosition();
	bool atCommand(CPlayer* player, const char* command);

	void addRadio(size_t menuid, const char* command, translation_t translations[5][16], size_t langs_count, bool mult);
	const radiocommand_t* getRadio(const char* command) const;
	const radiocommand_t* getRadio(size_t menuid, const int key) const;
	radiomenu_t* getRadioMenu(size_t menuid, lang_t lang);

private:
	edict_t* getPlantedBomb() const;

	size_t		m_maxclients;
	edict_t*	m_edictList;
	bool		m_gameActive;
	bool		m_mapActive;
	edict_t*	m_lastLogEdict;
	bombdata_t	m_bomb;

	std::vector<radiocommand_t> m_radio;
	std::vector<radiomenu_t> m_radioMenuCache;
};

extern CGame g_game;

void engine_hookCvarSetFloat();
