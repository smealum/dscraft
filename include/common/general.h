#ifndef __GENERAL9__
#define __GENERAL9__

#define GAMEVERSION "1.0"

#define v1 NORMAL_PACK( 0, 0,0);
#define v2 NORMAL_PACK( 0,64,0);
#define v3 NORMAL_PACK(64,64,0);
#define v4 NORMAL_PACK(64, 0,0);

#define t1 NORMAL_PACK(-32,-32,0)
#define t2 NORMAL_PACK(-32,32,0)
#define t3 NORMAL_PACK(32,32,0)
#define t4 NORMAL_PACK(32,-32,0)

#include <nds.h>
#include <fat.h>
#include <filesystem.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include <unistd.h>

#include "creeper_bin.h"

#include "fat/fatfile.h"

#include "common/math.h"

#include "game/ex_game.h"
#include "menu/ex_menu.h"

#include "lodepng.h"
#include "debug/xmem.h"
#include "debug/stats.h"
#include "engine/state.h"
#include "engine/debug.h"
#include "engine/error.h"
#include "engine/files.h"
#include "engine/memory.h"

#endif
