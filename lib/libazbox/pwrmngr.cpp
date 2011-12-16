#include <stdio.h>

#include "pwrmngr.h"

static const char * FILENAME = "pwrmngr.cpp";

void cCpuFreqManager::Up(void) { printf("%s:%s\n", FILENAME, __FUNCTION__); }
void cCpuFreqManager::Down(void) { printf("%s:%s\n", FILENAME, __FUNCTION__); }
void cCpuFreqManager::Reset(void) { printf("%s:%s\n", FILENAME, __FUNCTION__); }
//
bool cCpuFreqManager::SetCpuFreq(unsigned long CpuFreq) { printf("%s:%s\n", FILENAME, __FUNCTION__); return 0; }
bool cCpuFreqManager::SetDelta(unsigned long Delta) { printf("%s:%s\n", FILENAME, __FUNCTION__); return 0; }
unsigned long cCpuFreqManager::GetCpuFreq(void) { printf("%s:%s\n", FILENAME, __FUNCTION__); return 0; }
unsigned long cCpuFreqManager::GetDelta(void) { printf("%s:%s\n", FILENAME, __FUNCTION__); return 0; }
//
cCpuFreqManager::cCpuFreqManager(void) { printf("%s:%s\n", FILENAME, __FUNCTION__); }



bool cPowerManager::SetState(PWR_STATE PowerState) { printf("%s:%s\n", FILENAME, __FUNCTION__); return 0; }

bool cPowerManager::Open(void) { printf("%s:%s\n", FILENAME, __FUNCTION__); return 0; }
void cPowerManager::Close(void) { printf("%s:%s\n", FILENAME, __FUNCTION__); }
//
bool cPowerManager::SetStandby(bool Active, bool Passive) { printf("%s:%s\n", FILENAME, __FUNCTION__); return 0; }
//
cPowerManager::cPowerManager(void) { printf("%s:%s\n", FILENAME, __FUNCTION__); }
cPowerManager::~cPowerManager() { printf("%s:%s\n", FILENAME, __FUNCTION__); }

