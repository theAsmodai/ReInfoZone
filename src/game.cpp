#include "precompiled.h"

CGame g_game;

radiomenu_t::radiomenu_t(size_t menuid, lang_t lng, const char* txt, int keys): id(menuid), lang(lng), keybits(keys)
{
	text = dupString(txt);
	len = strlen(txt);
}

CGame::CGame() : m_maxclients(0), m_edictList(nullptr), m_gameActive(false), m_mapActive(false), m_lastLogEdict(nullptr)
{
	memset(&m_bomb, 0, sizeof m_bomb);
}

size_t CGame::indexOfEdict(const edict_t* ed) const
{
	return ed - m_edictList;
}

edict_t* CGame::edictByIndex(size_t index) const
{
	return m_edictList + index;
}

edict_t* CGame::getLastLogEdict()
{
	edict_t* ed = m_lastLogEdict;
	m_lastLogEdict = nullptr;
	return ed;
}

void CGame::setLastLogEdict(edict_t* ed)
{
	if (!m_lastLogEdict)
		m_lastLogEdict = ed;	
}

bool CGame::isGameActive() const
{
	return m_gameActive;
}

bool CGame::isMapActive() const
{
	return m_mapActive;
}

bombstate_e CGame::getBombState() const
{
	return m_bomb.state;
}

bombdata_t& CGame::getBombData()
{
	if (iz_smooth_positioning->string[0] != '1')
		updateBombPosition();
	return m_bomb;
}

void CGame::mapStart(edict_t* edicts, size_t maxclients)
{
	m_maxclients = maxclients;
	m_edictList = edicts;
	m_mapActive = true;

	loadConfigs();

	if (g_engfuncs.pfnCVarGetFloat("developer") != 0.0)
		LCPrintf("Memory used: %.1fKb\n", memoryUsed() / 1024.0);

	g_players.init(edicts, maxclients);
	startHudTimer();
}

void CGame::mapEnd()
{
	m_maxclients = 0;
	m_edictList = nullptr;
	m_gameActive = false;
	m_mapActive = false;
	m_lastLogEdict = nullptr;
	memset(&m_bomb, 0, sizeof m_bomb);

	m_radio.clear();
	m_radioMenuCache.clear();

	stopHudTimer();
	g_zoneManager.clearZones();
	g_players.resetPositionsData();
	g_lang.clear();
	freeMemory();
}

void CGame::roundRestart()
{
	m_gameActive = false;
	g_players.resetPositionsData();
}

void CGame::roundStart()
{
	m_gameActive = true;
}

void CGame::roundEnd()
{
	m_gameActive = false;
	engine_hookCvarSetFloat();
}

void CGame::bombCollected()
{
	m_bomb.state = bs_carried;
	m_bomb.owner = nullptr;
	m_bomb.lastPos = Vector(0.0, 0.0, 0.0);
	m_bomb.lastZone = UNKNOWN_ZONE;
}

void CGame::bombDropped(CPlayer* player)
{
	m_bomb.state = bs_dropped;
	m_bomb.owner = nullptr;
	if (player) {
		m_bomb.lastPos = player->getOrigin();
		m_bomb.lastZone = player->getZone();
	}
}

void CGame::bombPicked(CPlayer* player)
{
	bombCollected();
}

void CGame::bombPlanted(CPlayer* player)
{
	auto bomb = getPlantedBomb();

	m_bomb.state = bs_planted;
	m_bomb.owner = nullptr;
	if (bomb) m_bomb.lastPos = bomb->v.origin;
	if (player) m_bomb.lastZone = player->getZone();
}

bool CGame::atCommand(CPlayer* player, const char* command)
{
	const radiocommand_t* rc = getRadio(command);

	if (rc) {
		player->radio(rc);
		return !g_config.botsFix;
	}

	if (player->isAlive() && !player->isSpectator() && !strncmp(command, "radio", 5)) {
		player->showMenu(getRadioMenu(strtoul(command + 5, nullptr, 10), player->getLang()));
		return true;
	}

	const char* arg = g_engfuncs.pfnCmd_Argv(1);

	if (!strcmp(arg, "/iz") && !strcmp(command, "say") || !strcmp(command, "say_team")) {
		player->showOptions();
		return true;
	}

	if (!strncmp(command, "menuselect", 10)) {
		return player->onMenuselect(strtol(arg, nullptr, 10));
	}

	if (!strcmp(command, "ignorerad")) {
		player->toggleIgnoreRadio();
		return true;
	}

	return false;
}

edict_t* CGame::getPlantedBomb() const
{
	auto isC4 = [](edict_t* ed) -> bool {
		enum { m_bIsC4 = 385 };
		return getPData(ed->pvPrivateData, m_bIsC4, EXTRAOFFSET_ITEM, false);
	};

	edict_t* ent = nullptr;
	while ((ent = g_engfuncs.pfnFindEntityByString(ent, "classname", "grenade")) != nullptr) {
		if (isC4(ent))
			break;
	}
	return ent;
}

void CGame::updateBombPosition()
{
	if (m_bomb.state != bs_dropped)
		return;

	if (!m_bomb.owner) {
		auto bomb = g_engfuncs.pfnFindEntityByString(g_game.edictByIndex(g_players.getMaxClients() + 1), "classname", "weapon_c4");
		if (!bomb || !bomb->v.owner)
			return;

		m_bomb.owner = &bomb->v.owner->v;
		if (!m_bomb.owner)
		 	return;
	}

	if (vectorCompare(m_bomb.lastPos, m_bomb.owner->origin))
		return;

	m_bomb.lastPos = m_bomb.owner->origin;

	if (!g_zoneManager.getZonesCount())
		return;

	if (m_bomb.lastZone != UNKNOWN_ZONE && g_zoneManager.isInZone(m_bomb.owner->origin, m_bomb.lastZone))
		return;

	m_bomb.lastZone = g_zoneManager.getNearestZoneByOrigin(m_bomb.owner->origin, iz_item_max_radius->value);
}

void CGame::addRadio(size_t menuid, const char* command, translation_t translations[5][16], size_t langs_count, bool mult)
{
	size_t phrases_count = mult ? 5 : 3;

	radiocommand_t rc;
	rc.command = dupString(command);
	rc.phrases = (phrase_t *)allocMemory(phrases_count * sizeof(phrase_t));
	rc.menuid = (uint8)menuid;
	rc.mult = mult;

	for (size_t i = 0; i < phrases_count; i++) {
		rc.phrases[i].translations_count = langs_count;
		rc.phrases[i].translations = (translation_t *)allocMemory(langs_count * sizeof(translation_t));

		for (size_t j = 0; j < langs_count; j++) {
			rc.phrases[i].translations[j].lang = translations[i][j].lang;
			rc.phrases[i].translations[j].text = dupString(translations[i][j].text);
		}
	}

	m_radio.emplace_back(rc);
}

const radiocommand_t* CGame::getRadio(const char* command) const
{
	for (size_t i = 0; i < m_radio.size(); i++) {
		if (!strcmp(command, m_radio[i].command))
			return &m_radio[i];
	}
	return nullptr;
}

const radiocommand_t* CGame::getRadio(size_t menuid, const int key) const
{
	int curKey = 1;

	for (size_t i = 0; i < m_radio.size(); i++) {
		if (menuid == m_radio[i].menuid) {
			if (key == curKey) {
				return &m_radio[i];
			}
			curKey++;
		}
	}
	return nullptr;
}

radiomenu_t* CGame::getRadioMenu(size_t menuid, lang_t lang)
{
	for (size_t i = 0; i < m_radioMenuCache.size(); i++) {
		auto menu = &m_radioMenuCache[i];
		if (menuid == menu->id && lang == menu->lang)
			return menu;
	}
	
	CMsgBuf buf(va("%s\n\n", g_lang.localize(va("radio_menu_%i", menuid), lang, "\\yRadio\\w")));
	auto menu_option = g_lang.localize("radio_menu_option", lang, "[number]. [option]");
	int keys = MENU_KEY_0;
	size_t n = 0;

	for (auto& radio : m_radio) {
		if (radio.menuid != menuid)
			continue;

		keys |= 1 << n++;

		CMsgBuf option(va("%s\n", menu_option));
		option.replace("[number]", va("%i", n));
		option.replace("[option]", g_lang.localize(radio.phrases[rp_menu], lang));
		buf.add(option);

		if (n > 8)
			break;
	}

	if (n) {
		buf.add(va("\n%s", g_lang.localize("radio_menu_exit", lang, "0. Exit")));
		m_radioMenuCache.emplace_back(menuid, lang, buf.data(), keys);
		return &m_radioMenuCache[m_radioMenuCache.size() - 1];
	}

	return nullptr;
}
