/// \file  menus/options-server-advanced.c
/// \brief Advanced Server Options

#include "../k_menu.h"

menuitem_t OPTIONS_ServerAdvanced[] =
{

	{IT_STRING | IT_CVAR | IT_CV_STRING, "Server Browser Address", "Default is \'https://ms.kartkrew.org/ms/api\'",
		NULL, {.cvar = &cv_masterserver}, 0, 0},

	{IT_STRING | IT_CVAR, "Resynch. Attempts", "How many times to attempt sending data to desynchronized players.",
		NULL, {.cvar = &cv_resynchattempts}, 0, 0},

	{IT_STRING | IT_CVAR, "Ping Limit (ms)", "Players above the ping limit will get kicked from the server.",
		NULL, {.cvar = &cv_maxping}, 0, 0},

	{IT_STRING | IT_CVAR, "Ping Timeout (s)", "Players must be above the ping limit for this long before being kicked.",
		NULL, {.cvar = &cv_pingtimeout}, 0, 0},

	{IT_STRING | IT_CVAR, "Connection Timeout (tics)", "Players not giving any netowrk activity for this long are kicked.",
		NULL, {.cvar = &cv_nettimeout}, 0, 0},

	{IT_STRING | IT_CVAR, "Join Timeout (tics)", "Players taking too long to join are kicked.",
		NULL, {.cvar = &cv_jointimeout}, 0, 0},

	{IT_SPACE | IT_NOTHING, NULL,  NULL,
		NULL, {NULL}, 0, 0},

	{IT_STRING | IT_CVAR, "Max File Transfer", "Maximum size of the files that can be downloaded from joining clients. (KB)",
		NULL, {.cvar = &cv_maxsend}, 0, 0},

	{IT_STRING | IT_CVAR, "File Transfer Speed", "File transfer packet rate. Larger values send more data.",
		NULL, {.cvar = &cv_downloadspeed}, 0, 0},

	{IT_SPACE | IT_NOTHING, NULL,  NULL,
		NULL, {NULL}, 0, 0},

	{IT_STRING | IT_CVAR, "Log Joiner IPs", "Shows the IP of connecting players.",
		NULL, {.cvar = &cv_showjoinaddress}, 0, 0},

	{IT_STRING | IT_CVAR, "Log Resynch", "Shows which players need resynchronization.",
		NULL, {.cvar = &cv_blamecfail}, 0, 0},

	{IT_STRING | IT_CVAR, "Log Transfers", "Shows when clients are downloading files from you.",
		NULL, {.cvar = &cv_noticedownload}, 0, 0},
};

menu_t OPTIONS_ServerAdvancedDef = {
	sizeof (OPTIONS_ServerAdvanced) / sizeof (menuitem_t),
	&OPTIONS_ServerDef,
	0,
	OPTIONS_ServerAdvanced,
	48, 70,	// This menu here is slightly higher because there's a lot of options...
	SKINCOLOR_VIOLET, 0,
	2, 5,
	M_DrawGenericOptions,
	M_OptionsTick,
	NULL,
	NULL,
	NULL,
};