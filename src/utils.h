#pragma once

class CMsgBuf
{
public:
	CMsgBuf(const char* string);
	CMsgBuf(CMsgBuf& buf);
	void replace(const char* what, const char* with);
	void add(const char* string);
	void add(CMsgBuf& buf);
	const char* data() const;
	size_t len() const;

private:
	char	m_buf[512];
	size_t	m_len;
};

struct va
{
	char buf[256];
	va(const char* fmt, ...);
	operator char*() { return buf; };
};

extern cvar_t* pcv_mp_logecho;
extern cvar_t* pcv_sv_accelerate;
extern int gmsgSayText;
extern int gmsgTextMsg;
extern int gmsgTeamInfo;
extern int gmsgSendAudio;
extern int gmsgShowMenu;
extern int g_sModelIndexRadio;
extern int g_sModelIndexDot;
extern int g_sModelIndexLedglow;

template<typename T>
T getPData(void* base, ptrdiff_t offset, ptrdiff_t extra, T def)
{
	return ptrdiff_t(base) ? *(reinterpret_cast<T *>(ptrdiff_t(base) + offset + extra)) : def;
}

template<typename T>
void setPData(void* base, ptrdiff_t offset, ptrdiff_t extra, T value)
{
	if (ptrdiff_t(base))
		*(reinterpret_cast<T *>(ptrdiff_t(base) + offset + extra)) = value;
}

void LCPrintf(const char *fmt, ...);
void debug(const char* fmt, ...);

void UTIL_SysError(const char* fmt, ...);
void UTIL_PrintChatColor(size_t player, const char* msg);
void UTIL_ClientPrint(edict_t* pEntity, int msg_dest, char* msg);
void UTIL_ShowMenu(edict_t* pEdict, int keybits, int time, char *menu, int mlen);
void UTIL_SendRadioSound(edict_t* receiver, int sender, const char* sound, bool showIcon = true);
void UTIL_DrawLine(edict_t* ed, Vector& v1, Vector& v2, color24 color, size_t width = 4);
void UTIL_DrawBox(edict_t* ed, Vector& v1, Vector& v2, color24 color);
//void UTIL_DrawAngle(edict_t* ed, Vector& target, float deg);
void UTIL_BombLed(edict_t* ed, Vector& v);

char* trimbuf(char *str);
size_t parse(char* line, char** argv, size_t max_args, const char* tokens = " \t\n\r", bool allow_quotes = true);
size_t replace(char* src, size_t maxlen, const char* strold, const char* strnew);
bool nullLast(char* string, char sym);
size_t readFlags(const char* c);
void* allocMemory(size_t size);
void* dupMemory(const void* ptr, size_t size);
const char* dupString(const char* string);
void freeMemory();
size_t memoryUsed();

double timeFrom(double from);
void startHudTimer();
void stopHudTimer();
void checkHudTimer(edict_t* ed);
uint32 getHash(const char* message);
short fixedSigned16(float value, float scale);
unsigned short fixedUnsigned16(float value, float scale);
