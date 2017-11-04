#pragma once

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <time.h>

#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

#include <vector>
#include <array>
#include <algorithm>

#include <extdll.h>
#include <meta_api.h>
#include <rehlds_api.h>
#include <sys_shared.h>
#include <usercmd.h>
#include <crc32c.h>

#include "amxxmodule.h"
#include "iz_rehlds_api.h"
#include "allocators.h"
#include "game_const.h"
#include "utils.h"
#include "vecmath.h"
#include "lang.h"
#include "config.h"
#include "zones.h"
#include "player.h"
#include "game.h"

template <typename T, size_t N>
char(&ArraySizeHelper(T(&array)[N]))[N];
#define arraysize(array) (sizeof(ArraySizeHelper(array)))
