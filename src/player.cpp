#include "precompiled.h"

enum
{
	m_iTeam = 456,
	m_bIgnoreRadio = 772,
	m_bHasC4 = 773,
	m_iMenu = 820
};

const char* fith_formats_loc[] =
{
	"fith_he_loc",
	"fith_flahs_loc",
	"fith_smoke_loc"
};
const char* fith_formats_def[] =
{
	"fith_he_def",
	"fith_flahs_def",
	"fith_smoke_def"
};

hudparms_t g_hudparms[8];
size_t g_hudparms_count;
CIzPlayers g_players;

CPlayer::CPlayer() : m_pev(nullptr), m_edict(nullptr), m_ingame(false), m_lang(UNKNOWN_LANG), m_lastZoneId(UNKNOWN_ZONE), m_lastZoneTime(0.0), m_lastReport(0.0), m_lastRadio(0), m_amxx(nullptr)
{
	m_options.integer = 0;
}

void CPlayer::init(edict_t* ed)
{
	m_pev = &ed->v;
	m_edict = ed;
}

void CPlayer::putInServer()
{
	m_ingame = true;
	m_lastZoneId = UNKNOWN_ZONE;
	m_lastZoneTime = gpGlobals->time;
	m_lastReport = gpGlobals->time;
	m_lastRadio = gpGlobals->time;
	loadOptions(g_engfuncs.pfnGetInfoKeyBuffer(m_edict));
	blockGameDllRadio();
	m_amxx = (amxxplayer_t *)g_amxxapi.PlayerPropAddr(getIndex(), Player_Vgui);

	if (&m_amxx->page != (int *)g_amxxapi.PlayerPropAddr(getIndex(), Player_NewmenuPage))
		UTIL_SysError("Invalid player prop struct addresses\n");
}

void CPlayer::disconnect()
{
	m_ingame = false;
}

void CPlayer::updateZone()
{
	if (!g_zoneManager.getZonesCount())
		return;

	if (g_zoneManager.isInZone(m_pev->origin, m_lastZoneId)) {
		m_lastZoneTime = gpGlobals->time;
		return;
	}

	const size_t zoneId = g_zoneManager.getZoneByOrigin(getOrigin());

	if (zoneId == UNKNOWN_ZONE && iz_smooth_positioning->value != 0.0) {
		if (iz_zone_leave_time->value < 0)
			return;
		if (timeFrom(m_lastZoneTime) <= iz_zone_leave_time->value)
			return;
	}
	else {
		m_lastZoneTime = gpGlobals->time;
	}

	m_lastZoneId = zoneId;
}

lang_t CPlayer::getLang() const
{
	return m_lang;
}

const char* CPlayer::getPosition(lang_t lang)
{
	return g_zoneManager.getZoneName(getZone(), lang);
}

void CPlayer::loadOptions(char* info)
{
	uint8 options = g_engfuncs.pfnInfoKeyValue(info, "iz")[0];

	if (options > '\\')
		options--;
	options -= ' ';

	m_options.integer = (options ^ g_config.defaultOptions) & 63;
	m_lang = g_engfuncs.pfnInfoKeyValue(info, "lang");
}

void CPlayer::saveOptions() const
{
	uint8 store = (m_options.integer ^ g_config.defaultOptions) & 63;

	if (store) {
		store += ' ';

		if (store >= '\\')
			store++;

		g_engfuncs.pfnClientCommand(m_pev->pContainingEntity, "setinfo iz \"%c\"\n", store);
	}
	else
		g_engfuncs.pfnClientCommand(m_pev->pContainingEntity, "setinfo iz \"\"\n", store);
}

int CPlayer::nextHUDChannel() const
{
	int ilow = 1;

	for (int i = ilow + 1; i <= 4; i++) {
		if (m_amxx->channels[i] < m_amxx->channels[ilow])
			ilow = i;
	}

	return ilow;
}

void CPlayer::showHud(hudparms_t& parms, float holdTime, const char* message) const
{
	if (!parms.coord.x && !parms.coord.y)
		return;

	MESSAGE_BEGIN(MSG_ONE, SVC_TEMPENTITY, NULL, m_pev->pContainingEntity);
		WRITE_BYTE(TE_TEXTMESSAGE);
		WRITE_BYTE(nextHUDChannel());

		WRITE_SHORT(fixedSigned16(parms.coord.x, 1 << 13));
		WRITE_SHORT(fixedSigned16(parms.coord.y, 1 << 13));
		WRITE_BYTE(0);

		WRITE_BYTE(parms.color.r);
		WRITE_BYTE(parms.color.g);
		WRITE_BYTE(parms.color.b);
		WRITE_BYTE(0);

		WRITE_BYTE(255);
		WRITE_BYTE(255);
		WRITE_BYTE(255);
		WRITE_BYTE(0);

		WRITE_SHORT(fixedUnsigned16(0.05, 1 << 8));
		WRITE_SHORT(fixedUnsigned16(2.0, 1 << 8));
		WRITE_SHORT(fixedUnsigned16(holdTime, 1 << 8));

		//if (effect == 2)
			//WRITE_SHORT(fixedUnsigned16(textparms.fxTime, 1 << 8));

		if (strlen(message) < 512) {
			WRITE_STRING(message);
		}
		else {
			char tmp[512];
			memcpy(tmp, message, 511);
			tmp[sizeof tmp - 1] = 0;
			WRITE_STRING(tmp);
		}
	MESSAGE_END();
}

void CPlayer::showMenu(radiomenu_t* menu) const
{
	if(menu) showMenu(menu->keybits, menu->text, menu->len, _Menu(Menu_Radio1 - 1 + menu->id));
}

void CPlayer::showMenu(int keys, const char* text, size_t len, _Menu menu) const
{
	m_amxx->vgui = true;
	m_amxx->menu = 0;
	m_amxx->keys = keys;
	m_amxx->newmenu = -1;
	setPData(m_iMenu, menu);

	char buf[512];
	strncpy(buf, text, sizeof buf - 1);
	buf[sizeof buf - 1] = '\0';

	UTIL_ShowMenu(m_edict, keys, -1, buf, min(len, sizeof buf - 1));
}

void CPlayer::showOptions() const
{
	auto lang = getLang();
	auto enabled = g_lang.localize("enabled", lang, "\\yenabled\\w");
	auto disabled = g_lang.localize("disabled", lang, "\\rdisabled\\w");

	CMsgBuf buf(va("%s\n\n", g_lang.localize("options_menu", lang, "\\yOptions\\w")));
	buf.add(va("%s %s\n", g_lang.localize("options_hud", lang, "1. HUD position:"), g_lang.localize(g_hudparms[m_options.hudpos].phrase, lang)));
	buf.add(va("%s %s\n", g_lang.localize("options_radio", lang, "2. Radio commands:"), m_options.block_radio ? disabled : enabled));
	buf.add(va("%s %s\n", g_lang.localize("options_fith", lang, "3. Fire in the hole:"), m_options.block_fith ? disabled : enabled));
	buf.add(va("%s\n", g_lang.localize("options_reset", lang, "4. Restore defaults")));
	buf.add(va("\n%s", g_lang.localize("options_exit", lang, "0. Exit")));

	showMenu(MENU_KEY_1 | MENU_KEY_2 | MENU_KEY_3 | MENU_KEY_4 | MENU_KEY_0, buf.data(), buf.len(), Menu_InfoZone);
}

bool CPlayer::onMenuselect(const int option)
{
	if (m_amxx->vgui != true || m_amxx->menu != 0 || m_amxx->newmenu != -1)
		return false;

	const auto menu = getPData(m_iMenu, Menu_OFF);

	if (menu == Menu_InfoZone) {
		if (option >= 10) {
			setPData(m_iMenu, Menu_OFF);
			saveOptions();
		}
		else {
			if ((1 << (option - 1)) & m_amxx->keys) {
				changeOption(option);
			}
			showOptions();
		}
		return true;
	}

	if (menu >= Menu_Radio1 && menu <= Menu_Radio3) {
		if (option < 10 && (1 << (option - 1)) & m_amxx->keys) {
			auto rc = g_game.getRadio(menu - Menu_Radio1 + 1, option);
			if (rc) radio(rc);
		}
		setPData(m_iMenu, Menu_OFF);
	}

	return false;
}

void CPlayer::changeOption(const int optId)
{
	size_t newHudPos;

	switch (optId) {
	case 1:
		newHudPos = m_options.hudpos + 1;
		if (newHudPos >= g_hudparms_count)
			newHudPos = 0;
		m_options.hudpos = newHudPos;
		showPosition();
		break;

	case 2:
		m_options.block_radio = !m_options.block_radio;
		break;

	case 3:
		m_options.block_fith = !m_options.block_fith;
		break;

	case 4:
		m_options.integer = g_config.defaultOptions & 63;
		break;
	}
}

bool CPlayer::needSendRadio(CsTeams sender_team) const
{
	if (!m_ingame)
		return false;

	if (!isSpectator()) {
		if (!isAlive() || sender_team != getTeam())
			return false;
	}
	else {
		size_t specMode = getSpecMode();
		if (specMode != OBS_CHASE_LOCKED && specMode != OBS_CHASE_FREE && specMode != OBS_IN_EYE)
			return false;

		size_t specTarget = getSpecTarget();
		if (!specTarget || g_players.getPlayer(specTarget)->getTeam() != sender_team)
			return false;
	}

	return true;
}

void CPlayer::toggleIgnoreRadio()
{
	m_options.block_radio = m_options.block_radio ^ 1;
	saveOptions();
}

void CPlayer::radio(const radiocommand_t* rc)
{
	if (timeFrom(m_lastRadio) < 1.5f)
		return;
	m_lastRadio = gpGlobals->time;

	if (!rc->mult || g_engfuncs.pfnRandomLong(0, 1))
		radioCommand(rc->phrases[rp_sound], rc->phrases[rp_text]);
	else
		radioCommand(rc->phrases[rp_sound2], rc->phrases[rp_text2]);
}

void CPlayer::showPosition()
{
	CPlayer* player;

	if (!isAlive()) {
		if (getSpecMode() != OBS_IN_EYE || m_pev->iuser2 == 0)
			return;

		player = g_players.getPlayer(getSpecTarget());

		if (!player->isAlive())
			return;
	}
	else
		player = this;

	auto position = player->getPosition(getLang());
	if (position[0]) {
		showHud(g_hudparms[clamp(m_options.hudpos, 0u, g_hudparms_count - 1u)], HUD_CHECK_INTERVAL, position);
	}
}

bool CPlayer::isInGame() const
{
	return m_ingame;
}

bool CPlayer::isHasC4() const
{
	return getPData(m_bHasC4, false);
}

bool CPlayer::isAlive() const
{
	return m_ingame && m_pev->deadflag == DEAD_NO && m_pev->health > 0;
}

bool CPlayer::isSpectator() const
{
	return (m_pev->flags & FL_SPECTATOR) != 0;
}

size_t CPlayer::getSpecMode() const
{
	return size_t(m_pev->iuser1);
}

size_t CPlayer::getSpecTarget() const
{
	return size_t(m_pev->iuser2);
}

entvars_t* CPlayer::getPev() const
{
	return m_pev;
}

edict_t* CPlayer::getEdict() const
{
	return m_edict;
}

size_t CPlayer::getIndex() const
{
	return g_players.getIndex(this);
}

Vector& CPlayer::getOrigin() const
{
	return m_pev->origin;
}

CsTeams CPlayer::getTeam() const
{
	return getPData(m_iTeam, CS_TEAM_UNASSIGNED);
}

size_t CPlayer::getZone()
{
	if (iz_smooth_positioning->string[0] != '1') {
		updateZone();
	}
	return m_lastZoneId;
}

bool CPlayer::usePressed()
{
	if (!g_game.isGameActive() || !isAlive() /*|| getTeam() != CS_TEAM_CT*/)
		return false;

	if (g_game.getBombState() == bs_dropped) {
		auto& bomb = g_game.getBombData();

		if (bomb.owner) {
			auto& bombOrigin = bomb.owner->origin;

			if (getViewAngleToOrigin(m_pev, bombOrigin) <= iz_max_aim_angle->value) {
				if (timeFrom(m_lastReport) >= iz_use_interval->value) {
					m_lastReport = gpGlobals->time;

					if (iz_smooth_positioning->string[0] != '1')
						g_game.updateBombPosition();

					radioAimReport(bomb.lastZone);
				}

				return (bombOrigin - m_pev->origin).Length() > 100.0;
			}
		}
	}

	return false;
}

void CPlayer::resetPositionData()
{
	m_lastZoneId = UNKNOWN_ZONE;
}

void CPlayer::radioCommand(phrase_t& sound, phrase_t& text)
{
	if (!isAlive() || isSpectator())
		return;

	CsTeams team = getTeam();
	size_t index = getIndex();

	for (size_t i = 0; i < g_players.getMaxClients(); i++) {
		auto& player = g_players[i];

		if (!player.needSendRadio(team) || m_options.block_radio)
			continue;

		auto lang = player.getLang();
		auto position = getPosition(lang);
		auto audio = g_lang.localize(sound, lang);
		UTIL_SendRadioSound(player.m_edict, index, audio);

		CMsgBuf msg(g_lang.localize(position[0] ? "radio_format_loc" : "radio_format_def", lang, "[name] (RADIO): [command]"));
		msg.replace("[command]", g_lang.localize(text, lang));
		msg.replace("[zone]", position);
		msg.replace("[name]", STRING(m_pev->netname));
		UTIL_PrintChatColor(i + 1, msg.data());
	}
}

void CPlayer::radioThrowGrenade(size_t grenade_type)
{
	if (!isAlive() || isSpectator())
		return;

	CsTeams team = getTeam();
	size_t index = getIndex();

	for (size_t i = 0; i < g_players.getMaxClients(); i++) {
		auto& player = g_players[i];

		if (!player.needSendRadio(team) || m_options.block_fith)
			continue;

		auto lang = player.getLang();
		auto position = getPosition(lang);
		auto audio = g_lang.localize("fith_sound", lang, "%!MRAD_FIREINHOLE");
		UTIL_SendRadioSound(player.m_edict, index, audio);

		CMsgBuf msg(g_lang.localize(position[0] ? fith_formats_loc[grenade_type] : fith_formats_def[grenade_type], lang, "[name] (RADIO): [command]"));
		msg.replace("[zone]", position);
		msg.replace("[name]", STRING(m_pev->netname));
		UTIL_PrintChatColor(i + 1, msg.data());
	}
}

void CPlayer::radioAimReport(size_t aimzone)
{
	if (!isAlive() || isSpectator())
		return;

	CsTeams team = getTeam();
	size_t index = getIndex();

	for (size_t i = 0; i < g_players.getMaxClients(); i++) {
		auto& player = g_players[i];

		if (!player.needSendRadio(team))
			continue;

		auto lang = player.getLang();
		auto position = getPosition(lang);
		auto aimpos = g_zoneManager.getZoneName(aimzone, lang);

		UTIL_SendRadioSound(player.m_edict, index, g_lang.localize("bomb_here_sound", lang, "%!MRAD_BACKUP"));

		CMsgBuf msg(g_lang.localize(position[0] ? "aim_report_loc" : "aim_report_def", lang, "[name] (RADIO): "));
		msg.add(g_lang.localize(aimpos[0] ? "bomb_here_loc" : "bomb_here_def", lang, "I'm seeing the bomb!"));
		msg.replace("[zone]", position);
		msg.replace("[aimzone]", aimpos);
		msg.replace("[name]", STRING(m_pev->netname));
		UTIL_PrintChatColor(i + 1, msg.data());
	}
}

void CPlayer::blockGameDllRadio() const
{
	setPData(m_bIgnoreRadio, true);
}

CPlayer* CIzPlayers::getPlayer(const edict_t* ed)
{
	size_t index = g_game.indexOfEdict(ed);
	if (!index || index > m_maxplayers)
		return nullptr;
	return getPlayer(index);
}

CPlayer* CIzPlayers::getPlayer(size_t index)
{
	return m_players + index - 1;
}

size_t CIzPlayers::getAnyPlayer(CsTeams team) const
{
	for (size_t i = 0; i < m_maxplayers; i++) {
		if (m_players[i].getTeam() == team)
			return i + 1;
	}
	return 0;
}

size_t CIzPlayers::getIndex(const CPlayer* player)
{
	return player - m_players + 1;
}

CPlayer* CIzPlayers::getBomber()
{
	for (size_t i = 0; i < m_maxplayers; i++) {
		if (m_players[i].isHasC4())
			return m_players + i;
	}
	return nullptr;
}

CPlayer& CIzPlayers::operator[](size_t index)
{
	return m_players[index];
}

void CIzPlayers::init(edict_t* ed, size_t maxplayers)
{
	m_maxplayers = maxplayers;

	for (size_t i = 0; i < maxplayers; i++)
		m_players[i].init(g_engfuncs.pfnPEntityOfEntIndex(i + 1));
}

void CIzPlayers::updateHud()
{
	if (!g_game.isGameActive() || !g_hudparms_count || !g_zoneManager.getZonesCount())
		return;

	for (size_t i = 0; i < m_maxplayers; i++) {
		if (!m_players[i].isInGame())
			continue;

		m_players[i].showPosition();
	}
}

void CIzPlayers::resetPositionsData()
{
	for (size_t i = 0; i < m_maxplayers; i++)
		m_players[i].resetPositionData();
}

size_t CIzPlayers::getMaxClients() const
{
	return m_maxplayers;
}

void resetHudparms()
{
	g_hudparms_count = 0;
}

void addHudparms(translation_t* translations, size_t translations_count, float x, float y, byte red, byte green, byte blue)
{
	if (g_hudparms_count == arraysize(g_hudparms))
		return;

	auto hudparms = &g_hudparms[g_hudparms_count++];
	hudparms->phrase = phrase_t(translations, translations_count);
	hudparms->coord = Vector2D(x, y);
	hudparms->color.r = red;
	hudparms->color.g = green;
	hudparms->color.b = blue;
}
