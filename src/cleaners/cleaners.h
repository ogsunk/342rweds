#pragma once

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>

void DisablePrefetchForCurrentProcess();
void CleanPrefetchEntries();
void CleanBAMEntries();
void CleanTempDlls();
void CleanEventLogs();
void CleanUsnJournal();
void CleanRecentDocs();
void CleanShimCache();
void CleanUserAssist();
void CleanAmcache();
void RunCleanerAndExit();
