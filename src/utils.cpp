#include "precompiled.h"

edict_t* g_pEdictList;
static_allocator<4096> g_stAlloc;
edict_t* g_hudTimer;
cvar_t* pcv_mp_logecho;
cvar_t* pcv_sv_accelerate;
int gmsgSayText;
int gmsgTextMsg;
int gmsgTeamInfo;
int gmsgSendAudio;
int gmsgShowMenu;
int g_sModelIndexRadio;
int g_sModelIndexDot;
int g_sModelIndexLedglow;

const char* g_teamNames[] =
{
	"",
	"TERRORIST",
	"CT",
	"SPECTATOR"
};

CMsgBuf::CMsgBuf(const char* string)
{
	strncpy(m_buf, string, sizeof m_buf - 1);
	m_buf[sizeof m_buf - 1] = '\0';
	m_len = strlen(m_buf);
}

CMsgBuf::CMsgBuf(CMsgBuf& buf)
{
	strcpy(m_buf, buf.m_buf);
	m_len = buf.m_len;
}

void CMsgBuf::replace(const char* what, const char* with)
{
	m_len = ::replace(m_buf, sizeof m_buf - 1, what, with);
}

void CMsgBuf::add(const char* string)
{
	strncpy(m_buf + m_len, string, sizeof m_buf - 1 - m_len);
	m_buf[sizeof m_buf - 1] = '\0';
	m_len = strlen(m_buf);
}

void CMsgBuf::add(CMsgBuf& buf)
{
	strncpy(m_buf + m_len, buf.m_buf, sizeof m_buf - 1 - m_len);
	m_buf[sizeof m_buf - 1] = '\0';
	m_len = min(m_len + buf.m_len, sizeof buf - 1);
}

const char* CMsgBuf::data() const
{
	return m_buf;
}

size_t CMsgBuf::len() const
{
	return m_len;
}

va::va(const char* fmt, ...)
{
	va_list argptr;
	va_start(argptr, fmt);
	vsnprintf(buf, sizeof buf, fmt, argptr);
	va_end(argptr);
}

char logstring[2048] = "[ReIZ]: ";

NOINLINE void LCPrintf(const char *fmt, ...)
{
	va_list		argptr;
	const int	prefixlen = sizeof("[ReIZ]: ") - 1;

	va_start(argptr, fmt);
	vsnprintf(logstring + prefixlen, sizeof(logstring) - prefixlen, fmt, argptr);
	va_end(argptr);

	SERVER_PRINT(logstring);
}

NOINLINE void UTIL_LogPrintf(char *fmt, ...)
{
	va_list argptr;
	char string[1024];

	va_start(argptr, fmt);
	vsnprintf(string, sizeof string, fmt, argptr);
	va_end(argptr);

	// Print to log
	ALERT(at_logged, "%s", string);
}

NOINLINE void UTIL_HudMessage(edict_t * pEntity, const hudtextparms_t& textparms, const char* pMessage)
{
	MESSAGE_BEGIN(MSG_ONE, SVC_TEMPENTITY, nullptr, pEntity);
		WRITE_BYTE(TE_TEXTMESSAGE);
		WRITE_BYTE(textparms.channel);

		WRITE_SHORT(textparms.x);
		WRITE_SHORT(textparms.y);
		WRITE_BYTE(textparms.effect);

		WRITE_BYTE(textparms.r1);
		WRITE_BYTE(textparms.g1);
		WRITE_BYTE(textparms.b1);
		WRITE_BYTE(textparms.a1);

		WRITE_BYTE(textparms.r2);
		WRITE_BYTE(textparms.g2);
		WRITE_BYTE(textparms.b2);
		WRITE_BYTE(textparms.a2);

		WRITE_SHORT(textparms.fadeinTime);
		WRITE_SHORT(textparms.fadeoutTime);
		WRITE_SHORT(textparms.holdTime);

		if (textparms.effect == 2)
			WRITE_SHORT(textparms.fxTime);

		if (strlen(pMessage) < 512) {
			WRITE_STRING(pMessage);
		}
		else {
			char tmp[512];
			strncpy(tmp, pMessage, sizeof tmp - 1);
			tmp[sizeof tmp - 1] = '\0';
			WRITE_STRING(tmp);
		}
	MESSAGE_END();
}

NOINLINE void UTIL_ClientSayText(edict_t *pEntity, char* msg, int sender, CsTeams team)
{
	if (sender) {
		MESSAGE_BEGIN(MSG_ONE, gmsgSayText, NULL, pEntity);
		WRITE_BYTE(sender);
		WRITE_STRING(msg);
		MESSAGE_END();
	}
	else {
		size_t player_index = g_game.indexOfEdict(pEntity);
		CsTeams player_team = g_players.getPlayer(player_index)->getTeam();

		MESSAGE_BEGIN(MSG_ONE, gmsgTeamInfo, NULL, pEntity);
		WRITE_BYTE(player_index);
		WRITE_STRING(g_teamNames[team]);
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_ONE, gmsgSayText, NULL, pEntity);
		WRITE_BYTE(sender);
		WRITE_STRING(msg);
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_ONE, gmsgTeamInfo, NULL, pEntity);
		WRITE_BYTE(player_index);
		WRITE_STRING(g_teamNames[player_team]);
		MESSAGE_END();
	}
}

CsTeams UTIL_FormatColors(char* dest, size_t maxlen, const char* msg)
{
	CsTeams team = CS_TEAM_UNASSIGNED;

	if (!maxlen) {
		dest[0] = '\0';
		return team;
	}

	dest[0] = 0x1;
	size_t i = 1;

	while (*msg)
	{
		if (i == maxlen)
			break;

		if (msg[0] == '!') {
			switch (msg[1]) {
			case 'n':
				msg++;
				dest[i++] = 0x01;
				break;
			case 't':
				msg++;
				dest[i++] = 0x03;
				break;
			case 'r':
				msg++;
				dest[i++] = 0x03;
				team = CS_TEAM_T;
				break;
			case 'b':
				msg++;
				dest[i++] = 0x03;
				team = CS_TEAM_CT;
				break;
			case 'w':
				msg++;
				dest[i++] = 0x03;
				team = CS_TEAM_SPECTATOR;
				break;
			case 'g':
				msg++;
				dest[i++] = 0x04;
				break;
			default:
				dest[i++] = '!';
			}
		}
		else
			dest[i++] = msg[0];

		msg++;
	}

	dest[i] = '\0';
	return team;
}

void UTIL_PrintChatColor(size_t player, const char* msg)
{
	char buf[191];
	CsTeams team = UTIL_FormatColors(buf, sizeof buf - 1, msg);
	size_t sender = (team != CS_TEAM_UNASSIGNED) ? g_players.getAnyPlayer(team) : player;

	if (player) {
		UTIL_ClientSayText(g_game.edictByIndex(player), buf, sender, team);
	}
	else {
		for (size_t i = 0; i < g_players.getMaxClients(); i++) {
			if (!g_players.getPlayer(i)->isInGame())
				continue;

			UTIL_ClientSayText(g_game.edictByIndex(i), buf, (team != CS_TEAM_UNASSIGNED) ? sender : i, team);
		}
	}
}

void UTIL_ClientPrint(edict_t* pEntity, int msg_dest, char* msg)
{
	gmsgTextMsg = GET_USER_MSG_ID(PLID, "TextMsg", NULL);

	char c = msg[190];
	msg[190] = '\0';

	if (pEntity)
		MESSAGE_BEGIN(MSG_ONE, gmsgTextMsg, NULL, pEntity);
	else
		MESSAGE_BEGIN(MSG_BROADCAST, gmsgTextMsg);

	WRITE_BYTE(msg_dest);
	WRITE_STRING(msg);
	MESSAGE_END();
	msg[190] = c;
}

void UTIL_ShowMenu(edict_t* pEdict, int keybits, int time, char *menu, int mlen)
{
	char *n = menu;
	char c = 0;
	int a;

	do {
		a = mlen;
		if (a > 175) a = 175;
		mlen -= a;
		c = *(n+=a);
		*n = 0;

		MESSAGE_BEGIN(MSG_ONE, gmsgShowMenu, NULL, pEdict);
			WRITE_SHORT(keybits);
			WRITE_CHAR(time);
			WRITE_BYTE(c ? TRUE : FALSE);
			WRITE_STRING(menu);
		MESSAGE_END();
		*n = c;
		menu = n;
	}
	while (*n);
}

void UTIL_SendRadioSound(edict_t* receiver, int sender, const char* sound, bool showIcon)
{
	MESSAGE_BEGIN(MSG_ONE, gmsgSendAudio, NULL, receiver);
		WRITE_BYTE(sender);
		WRITE_STRING(sound);
		WRITE_SHORT(100);
	MESSAGE_END();

	if (showIcon) {
		MESSAGE_BEGIN(MSG_ONE, SVC_TEMPENTITY, NULL, receiver);
			WRITE_BYTE(TE_PLAYERATTACHMENT);
			WRITE_BYTE(sender);
			WRITE_COORD(35);
			WRITE_SHORT(g_sModelIndexRadio);
			WRITE_SHORT(15);
		MESSAGE_END();
	}
}

NOINLINE void UTIL_DrawLine(edict_t* ed, Vector& v1, Vector& v2, color24 color, size_t width)
{
#ifndef NDEBUG
	MESSAGE_BEGIN(MSG_ONE_UNRELIABLE, SVC_TEMPENTITY, NULL, ed);
		WRITE_BYTE(TE_BEAMPOINTS);
		WRITE_COORD(v1.x);
		WRITE_COORD(v1.y);
		WRITE_COORD(v1.z);
		WRITE_COORD(v2.x);
		WRITE_COORD(v2.y);
		WRITE_COORD(v2.z);
		WRITE_SHORT(g_sModelIndexDot);		// sprite index
		WRITE_BYTE(1);			// starting frame
		WRITE_BYTE(1);			// frame rate in 0.1's
		WRITE_BYTE(5);			// life in 0.1's
		WRITE_BYTE(width);			// line width in 0.1's
		WRITE_BYTE(0); 			// noise amplitude in 0.01's
		WRITE_BYTE(color.r);   	// r, g, b
		WRITE_BYTE(color.g);
		WRITE_BYTE(color.b);
		WRITE_BYTE(250); 		// brightness
		WRITE_BYTE(0);   		// scroll speed in 0.1's
	MESSAGE_END();
#endif
}

NOINLINE void UTIL_DrawBox(edict_t* ed, Vector& v1, Vector& v2, color24 color)
{
#ifndef NDEBUG
	static double lastDraw;
	if (lastDraw > gpGlobals->time)
		lastDraw = 0.0;
	if (timeFrom(lastDraw) < 0.48)
		return;
	lastDraw = gpGlobals->time;

	UTIL_DrawLine(ed, v1, v2, color);

	/*MESSAGE_BEGIN(MSG_ONE_UNRELIABLE, SVC_TEMPENTITY, NULL, ed);
		WRITE_BYTE(TE_BOX);
		WRITE_COORD(v1.x);
		WRITE_COORD(v1.y);
		WRITE_COORD(v1.z);
		WRITE_COORD(v2.x);
		WRITE_COORD(v2.y);
		WRITE_COORD(v2.z);
		WRITE_SHORT(5);			// life in 0.1's
		WRITE_BYTE(color.r);   	// r, g, b
		WRITE_BYTE(color.g);
		WRITE_BYTE(color.b);
	MESSAGE_END();*/

	UTIL_DrawLine(ed, v1, Vector(v1.x, v2.y, v1.z), color);
	UTIL_DrawLine(ed, v1, Vector(v2.x, v1.y, v1.z), color);
	UTIL_DrawLine(ed, Vector(v2.x, v2.y, v1.z), Vector(v2.x, v1.y, v1.z), color);
	UTIL_DrawLine(ed, Vector(v2.x, v2.y, v1.z), Vector(v1.x, v2.y, v1.z), color);

	UTIL_DrawLine(ed, v1, Vector(v1.x, v1.y, v2.z), color);
	UTIL_DrawLine(ed, v2, Vector(v2.x, v2.y, v1.z), color);
	UTIL_DrawLine(ed, Vector(v1.x, v2.y, v1.z), Vector(v1.x, v2.y, v2.z), color);
	UTIL_DrawLine(ed, Vector(v2.x, v1.y, v1.z), Vector(v2.x, v1.y, v2.z), color);

	UTIL_DrawLine(ed, v2, Vector(v1.x, v2.y, v2.z), color);
	UTIL_DrawLine(ed, v2, Vector(v2.x, v1.y, v2.z), color);
	UTIL_DrawLine(ed, Vector(v1.x, v1.y, v2.z), Vector(v2.x, v1.y, v2.z), color);
	UTIL_DrawLine(ed, Vector(v1.x, v1.y, v2.z), Vector(v1.x, v2.y, v2.z), color);
#endif
}

void UTIL_BombLed(edict_t* ed, Vector& v)
{
#ifndef NDEBUG
	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_GLOWSPRITE);
		WRITE_COORD(v.x);
		WRITE_COORD(v.y);
		WRITE_COORD(v.z);
		WRITE_SHORT(g_sModelIndexLedglow);
		WRITE_BYTE(1);
		WRITE_BYTE(3);
		WRITE_BYTE(255);
	MESSAGE_END();
#endif
}

void debug_log(const char* fmt, ...)
{
	va_list	argptr;
	char buf[4096];
	va_start(argptr, fmt);
	vsnprintf(buf, sizeof buf, fmt, argptr);
	buf[sizeof buf - 1] = 0;
	va_end(argptr);

	FILE* fp = fopen("iz_debug.log", "a");

	if (fp) {
		time_t curtime;
		struct tm* timeinfo;
		char timebuf[64];

		time(&curtime);
		timeinfo = localtime(&curtime);

		strftime(timebuf, sizeof timebuf, "%d/%m/%Y - %H:%M:%S", timeinfo);
		fprintf(fp, "%s: %s\n", timebuf, buf);
		fclose(fp);
	}
}

void debug(const char* fmt, ...)
{
#ifndef NDEBUG
	va_list	argptr;
	char buf[4096];
	va_start(argptr, fmt);
	vsnprintf(buf, sizeof buf, fmt, argptr);
	buf[sizeof buf - 1] = 0;
	va_end(argptr);

	printf("[DEBUG]: %s\n", buf);
	debug_log(buf);
#endif
}

NOINLINE void UTIL_SysError(const char* fmt, ...)
{
	va_list	argptr;
	char buf[4096];
	va_start(argptr, fmt);
	vsnprintf(buf, ARRAYSIZE(buf) - 1, fmt, argptr);
	buf[ARRAYSIZE(buf) - 1] = 0;
	va_end(argptr);

	printf("ERROR: %s", buf);
	*(int *)0 = 0;
}

NOINLINE char* trimbuf(char *str)
{
	char *ibuf;

	if (str == NULL) return NULL;
	for (ibuf = str; *ibuf && (byte)(*ibuf) < (byte)0x80 && isspace(*ibuf); ++ibuf)
		;

	int i = strlen(ibuf);
	if (str != ibuf)
		memmove(str, ibuf, i);

	while (--i >= 0) {
		if (!isspace(str[i]))
			break;
	}
	str[i + 1] = '\0';
	return str;
}

NOINLINE size_t parse(char* line, char** argv, size_t max_args, const char* tokens, bool allow_quotes)
{
	size_t count = 0;

	auto istoken = [tokens](int c) { return strchr(tokens, c); };

	while (*line) {
		// null whitespaces
		while (istoken(*line))
			*line++ = '\0';

		if (count == max_args)
			break;

		if (*line) {
			bool quotes = allow_quotes && *line == '"';

			if (quotes && (*line++ = '\0', *line == '\0'))
				break;

			argv[count++] = line; // save arg address

			// skip arg
			while (*line != '\0' && (quotes ? *line != '"' : !istoken(*line)))
				line++;

			if (*line && quotes)
				*line++ = '\0';
		}
	}

	return count;
}

NOINLINE size_t replace(char* src, size_t maxlen, const char* strold, const char* strnew)
{
	char *p = src;
	size_t srcLen = strlen(src);
	size_t oldLen = strlen(strold);
	size_t newLen = strlen(strnew);

	while ((p = strstr(p, strold)) != nullptr) {
		size_t newSrcLen = srcLen - oldLen + newLen;

		if (newSrcLen > maxlen) {
			newLen -= newSrcLen - maxlen;
			newSrcLen = maxlen;
		}

		if (oldLen != newLen)
			memmove(p + newLen, p + oldLen, srcLen - (p - src) - oldLen + 1);

		memcpy(p, strnew, newLen);
		srcLen = newSrcLen;
		p += newLen;
	}

	return srcLen;
}

NOINLINE bool nullLast(char* string, char sym)
{
	char* pos = strrchr(string, sym);
	if (!pos)
		return false;
	*pos = '\0';
	return true;
}

size_t readFlags(const char* c)
{
	size_t flags = 0;
	while (*c)
		flags |= (1 << (*c++ - 'a'));
	return flags;
}

NOINLINE void* allocMemory(size_t size)
{
	return g_stAlloc.allocate(size);
}

NOINLINE void* dupMemory(const void* ptr, size_t size)
{
	return memcpy(g_stAlloc.allocate(size), ptr, size);
}

NOINLINE const char* dupString(const char* string)
{
	return reinterpret_cast<char *>(dupMemory(string, strlen(string) + 1));
}

NOINLINE void freeMemory()
{
	g_stAlloc.deallocate_all();
}

size_t memoryUsed()
{
	return g_stAlloc.memoryUsed();
}

double timeFrom(double from)
{
	return gpGlobals->time - from;
}

void startHudTimer()
{
	g_hudTimer = CREATE_NAMED_ENTITY(MAKE_STRING("info_target"));
	g_hudTimer->v.classname = MAKE_STRING("hudTimer");
	g_hudTimer->v.nextthink = gpGlobals->time + 1.0;
}

void stopHudTimer()
{
	g_engfuncs.pfnRemoveEntity(g_hudTimer);
	g_hudTimer = nullptr;
}

void checkHudTimer(edict_t* ed)
{
	if (ed == g_hudTimer) {
		g_game.updateBombPosition();

		if (iz_smooth_positioning->string[0] == '0') {
			ed->v.nextthink = gpGlobals->time + HUD_CHECK_INTERVAL * 10;
			return;
		}

		ed->v.nextthink = gpGlobals->time + HUD_CHECK_INTERVAL;
		g_players.updateHud();
	}
}

NOINLINE uint32 getHash(const char* message)
{
	return crc32c((uint8 *)message, strlen(message));
}

short fixedSigned16(float value, float scale)
{
	return (short)clamp(int(value * scale), SHRT_MIN, SHRT_MAX);
}

unsigned short fixedUnsigned16(float value, float scale)
{
	return (unsigned short)clamp(int(value * scale), 0, USHRT_MAX);
}
