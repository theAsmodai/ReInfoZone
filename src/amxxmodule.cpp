// vim: set ts=4 sw=4 tw=99 noet:
//
// AMX Mod X, based on AMX Mod by Aleksander Naszko ("OLO").
// Copyright (C) The AMX Mod X Development Team.
// Parts Copyright (C) 2001-2003 Will Day <willday@hpgx.net>
//
// This software is licensed under the GNU General Public License, version 3 or higher.
// Additional exceptions apply. For full license details, see LICENSE.txt or visit:
//     https://alliedmods.net/amxmodx-license

//
// Module SDK
//

#include "precompiled.h"

int OnAmxx_Attach();

/************* AMXX Stuff *************/

// *** Globals ***
// Module info
static amxx_module_info_s g_ModuleInfo =
{
	Plugin_info.name,
	Plugin_info.author,
	Plugin_info.version,
	FALSE,
	Plugin_info.logtag,
	"reinfozone",
	"reinfozone"
};

// Storage for the requested functions
amxxapi_t g_amxxapi;

#define DECLARE_REQ(x)	{#x, offsetof(amxxapi_t, x)}

static struct funcreq_t
{
	const char*	name;
	size_t		offset;
} g_funcrequests[] =
{
	//DECLARE_REQ(AddNatives),
	//DECLARE_REQ(AddNewNatives),
	//DECLARE_REQ(BuildPathname),
	DECLARE_REQ(BuildPathnameR),
	//DECLARE_REQ(GetAmxAddr),
	//DECLARE_REQ(GetAmxVectorNull),
	//DECLARE_REQ(PrintSrvConsole),
	//DECLARE_REQ(GetModname),
	//DECLARE_REQ(GetAmxScriptName),
	//DECLARE_REQ(GetAmxScript),
	//DECLARE_REQ(FindAmxScriptByAmx),
	//DECLARE_REQ(FindAmxScriptByName),
	//DECLARE_REQ(SetAmxString),
	//DECLARE_REQ(SetAmxStringUTF8Char),
	//DECLARE_REQ(SetAmxStringUTF8Cell),
	//DECLARE_REQ(GetAmxString),
	//DECLARE_REQ(GetAmxStringNull),
	//DECLARE_REQ(GetAmxStringLen),
	//DECLARE_REQ(FormatAmxString),
	//DECLARE_REQ(CopyAmxMemory),
	//DECLARE_REQ(Log),
	//DECLARE_REQ(LogError),
	//DECLARE_REQ(RaiseAmxError),
	//DECLARE_REQ(RegisterForward),
	//DECLARE_REQ(ExecuteForward),
	//DECLARE_REQ(PrepareCellArray),
	//DECLARE_REQ(PrepareCharArray),
	//DECLARE_REQ(PrepareCellArrayA),
	//DECLARE_REQ(PrepareCharArrayA),
	//DECLARE_REQ(IsPlayerValid),
	//DECLARE_REQ(GetPlayerName),
	//DECLARE_REQ(GetPlayerIP),
	//DECLARE_REQ(IsPlayerIngame),
	//DECLARE_REQ(IsPlayerBot),
	//DECLARE_REQ(IsPlayerAuthorized),
	//DECLARE_REQ(GetPlayerTime),
	//DECLARE_REQ(GetPlayerPlayTime),
	//DECLARE_REQ(GetPlayerCurweapon),
	//DECLARE_REQ(GetPlayerTeam),
	//DECLARE_REQ(GetPlayerTeamID),
	//DECLARE_REQ(GetPlayerDeaths),
	//DECLARE_REQ(GetPlayerMenu),
	//DECLARE_REQ(GetPlayerKeys),
	//DECLARE_REQ(IsPlayerAlive),
	//DECLARE_REQ(GetPlayerFrags),
	//DECLARE_REQ(IsPlayerConnecting),
	//DECLARE_REQ(IsPlayerHLTV),
	//DECLARE_REQ(GetPlayerArmor),
	//DECLARE_REQ(GetPlayerHealth),
#ifdef MEMORY_TEST
	//DECLARE_REQ(Allocator),
	//DECLARE_REQ(Reallocator),
	//DECLARE_REQ(Deallocator),
#endif
	//DECLARE_REQ(amx_Exec),
	//DECLARE_REQ(amx_Execv),
	//DECLARE_REQ(amx_Allot),
	//DECLARE_REQ(amx_FindPublic),
	//DECLARE_REQ(LoadAmxScript),
	//DECLARE_REQ(UnloadAmxScript),
	//DECLARE_REQ(RealToCell),
	//DECLARE_REQ(CellToReal),
	//DECLARE_REQ(RegisterSPForward),
	//DECLARE_REQ(RegisterSPForwardByName),
	//DECLARE_REQ(UnregisterSPForward),
	//DECLARE_REQ(MergeDefinition_File),
	//DECLARE_REQ(amx_FindNative),
	//DECLARE_REQ(GetPlayerFlags),
	//DECLARE_REQ(GetPlayerEdict),
	//DECLARE_REQ(Format),
	//DECLARE_REQ(RegisterFunction),
	//DECLARE_REQ(RequestFunction),
	//DECLARE_REQ(amx_Push),
	//DECLARE_REQ(SetTeamInfo),
	DECLARE_REQ(PlayerPropAddr),
	//DECLARE_REQ(RegAuthFunc),
	//DECLARE_REQ(UnregAuthFunc),
	//DECLARE_REQ(FindLibrary),
	//DECLARE_REQ(AddLibraries),
	//DECLARE_REQ(RemoveLibraries),
	//DECLARE_REQ(OverrideNatives),
	DECLARE_REQ(GetLocalInfo),
	//DECLARE_REQ(AmxReRegister),
	//DECLARE_REQ(RegisterFunctionEx),
	//DECLARE_REQ(MessageBlock),
};

// *** Exports ***
C_DLLEXPORT int AMXX_Query(int *interfaceVersion, amxx_module_info_s *moduleInfo)
{
	// check parameters
	if (!interfaceVersion || !moduleInfo)
		return AMXX_PARAM;

	// check interface version
	if (*interfaceVersion != AMXX_INTERFACE_VERSION)
	{
		// Tell amxx core our interface version
		*interfaceVersion = AMXX_INTERFACE_VERSION;
		return AMXX_IFVERS;
	}

	// copy module info
	memcpy(moduleInfo, &g_ModuleInfo, sizeof(amxx_module_info_s));
	return AMXX_OK;
}

C_DLLEXPORT int AMXX_CheckGame(const char *game)
{
	return AMXX_GAME_OK;
}

C_DLLEXPORT int AMXX_Attach(PFN_REQ_FNPTR reqFnptrFunc)
{
	// Check pointer
	if (!reqFnptrFunc)
		return AMXX_PARAM;

	for (size_t i = 0; i < ARRAYSIZE(g_funcrequests); i++)
	{
		void* fptr = reqFnptrFunc(g_funcrequests[i].name);
		if (fptr == NULL)
		{
			return AMXX_FUNC_NOT_PRESENT;
		}
		*(void **)((ptrdiff_t)&g_amxxapi + g_funcrequests[i].offset) = fptr;
	}

	//RegisterNatives(); // TODO
	return OnAmxx_Attach();
}

C_DLLEXPORT int AMXX_Detach()
{
	return AMXX_OK;
}

C_DLLEXPORT int AMXX_PluginsLoaded()
{
	return AMXX_OK;
}

C_DLLEXPORT void AMXX_PluginsUnloaded()
{

}

C_DLLEXPORT void AMXX_PluginsUnloading()
{

}

// Advanced MF functions
NOINLINE void MF_Log(const char *fmt, ...)
{
	char msg[2048];
	va_list arglst;
	va_start(arglst, fmt);
	vsnprintf(msg, sizeof msg, fmt, arglst);
	va_end(arglst);

	g_amxxapi.Log("[%s] %s", g_ModuleInfo.logtag, msg);
}

NOINLINE void MF_LogError(AMX *amx, int err, const char *fmt, ...)
{
	char msg[2048];
	va_list arglst;
	va_start(arglst, fmt);
	vsnprintf(msg, sizeof msg, fmt, arglst);
	va_end(arglst);

	g_amxxapi.LogError(amx, err, "[%s] %s", g_ModuleInfo.logtag, msg);
}

#ifdef MEMORY_TEST

/************* MEMORY *************/
// undef all defined macros
#undef new
#undef delete
#undef malloc
#undef calloc
#undef realloc
#undef free

const		unsigned int	m_alloc_unknown = 0;
const		unsigned int	m_alloc_new = 1;
const		unsigned int	m_alloc_new_array = 2;
const		unsigned int	m_alloc_malloc = 3;
const		unsigned int	m_alloc_calloc = 4;
const		unsigned int	m_alloc_realloc = 5;
const		unsigned int	m_alloc_delete = 6;
const		unsigned int	m_alloc_delete_array = 7;
const		unsigned int	m_alloc_free = 8;

const char *g_Mem_CurrentFilename = "??";
int g_Mem_CurrentLine = 0;
const char *g_Mem_CurrentFunc = "??";

const char *Mem_MakeSourceFile(const char *sourceFile)
{
	static char buffer[512];
	static size_t pos = 0;
	if (!pos)
	{
		// init
		buffer[0] = '[';
		strcpy(buffer + 1, MODULE_NAME);
		pos = strlen(MODULE_NAME) + 1;
		buffer[pos++] = ']';
	}

	// convert from absolute path to [modulename]filename
	const char *ptr = strrchr(sourceFile, '\\');
	if (ptr)
		ptr++;
	else
	{
		ptr = strrchr(sourceFile, '/');
		if (ptr)
			ptr++;
		else
			ptr = sourceFile;
	}
	strcpy(buffer + pos, ptr);
	return buffer;
}

void Mem_SetOwner(const char *filename, int line, const char *function)
{
	g_Mem_CurrentFilename = filename;
	g_Mem_CurrentLine = line;
	g_Mem_CurrentFunc = function;
}

void Mem_ResetGlobals()
{
	Mem_SetOwner("??", 0, "??");
}

// raw (re/de)allocators
void *	Mem_Allocator(const char *sourceFile, const unsigned int sourceLine, const char *sourceFunc,
					  const unsigned int allocationType, const size_t reportedSize)
{
	if (g_fn_Allocator)
		return g_fn_Allocator(Mem_MakeSourceFile(sourceFile), sourceLine, sourceFunc, allocationType, reportedSize);
	else
		return malloc(reportedSize);
}

void *	Mem_Reallocator(const char *sourceFile, const unsigned int sourceLine, const char *sourceFunc,
						const unsigned int reallocationType, const size_t reportedSize, void *reportedAddress)
{
	if (g_fn_Reallocator)
		return g_fn_Reallocator(Mem_MakeSourceFile(sourceFile), sourceLine, sourceFunc, reallocationType, reportedSize, reportedAddress);
	else
		return realloc(reportedAddress, reportedSize);
}

void	Mem_Deallocator(const char *sourceFile, const unsigned int sourceLine, const char *sourceFunc,
						const unsigned int deallocationType, void *reportedAddress)
{
	// If you you get user breakpoint here, something failed :)
	//  - invalid pointer
	//  - alloc type mismatch	( for example
	//							char *a = new char[5]; delete char;
	//							)
	//  - The allocation unit is damaged (for example
	//							char *a = new char[5]; a[6] = 8;
	//							)
	//  - break on dealloc flag set (somehow)

	if (g_fn_Deallocator)
		g_fn_Deallocator(Mem_MakeSourceFile(sourceFile), sourceLine, sourceFunc, deallocationType, reportedAddress);
	else
		free(reportedAddress);
}

// new and delete operators
void	*operator new( size_t reportedSize )
{
	if (reportedSize == 0)
	reportedSize = 1;
	void *ptr = Mem_Allocator(g_Mem_CurrentFilename, g_Mem_CurrentLine, g_Mem_CurrentFunc, m_alloc_new, reportedSize);
	// :TODO: Handler support ?
	if (ptr)
		return ptr;

	// allocation failed
	return NULL;
}

void	*operator new[](size_t reportedSize)
{
	if (reportedSize == 0)
		reportedSize = 1;
	void *ptr = Mem_Allocator(g_Mem_CurrentFilename, g_Mem_CurrentLine, g_Mem_CurrentFunc, m_alloc_new_array, reportedSize);
	// :TODO: Handler support ?
	if (ptr)
		return ptr;

	// allocation failed
	return NULL;
}

// Microsoft memory tracking operators
void	*operator new( size_t reportedSize, const char *sourceFile, int sourceLine )
{
	if (reportedSize == 0)
	reportedSize = 1;
	void *ptr = Mem_Allocator(g_Mem_CurrentFilename, g_Mem_CurrentLine, g_Mem_CurrentFunc, m_alloc_new, reportedSize);
	// :TODO: Handler support ?
	if (ptr)
		return ptr;

	// allocation failed
	return NULL;
}
void	*operator new[](size_t reportedSize, const char *sourceFile, int sourceLine)
{
	if (reportedSize == 0)
		reportedSize = 1;
	void *ptr = Mem_Allocator(g_Mem_CurrentFilename, g_Mem_CurrentLine, g_Mem_CurrentFunc, m_alloc_new_array, reportedSize);
	// :TODO: Handler support ?
	if (ptr)
		return ptr;

	// allocation failed
	return NULL;
}

void	operator delete( void *reportedAddress )
{
	if (!reportedAddress)
		return;

	Mem_Deallocator(g_Mem_CurrentFilename, g_Mem_CurrentLine, g_Mem_CurrentFunc, m_alloc_delete, reportedAddress);
}

void	operator delete[](void *reportedAddress)
{
	if (!reportedAddress)
		return;

	Mem_Deallocator(g_Mem_CurrentFilename, g_Mem_CurrentLine, g_Mem_CurrentFunc, m_alloc_delete_array, reportedAddress);
}

#else

#if !defined NO_ALLOC_OVERRIDES && !defined MEMORY_TEST && !defined WIN32
void * operator new( size_t size ){
	return( calloc(1, size) );
}

void * operator new[](size_t size)
{
	return( calloc(1, size) );
}

void operator delete( void * ptr )
{
	if (ptr)
		free(ptr);
}

void operator delete[](void * ptr)
{
	if (ptr)
		free(ptr);
}
#endif

#endif //MEMORY_TEST