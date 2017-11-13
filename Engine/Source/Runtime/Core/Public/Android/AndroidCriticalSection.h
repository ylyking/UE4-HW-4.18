// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "PThreadCriticalSection.h"
#include "GenericPlatform/GenericPlatformCriticalSection.h"
#include "PThreadRWLock.h"

typedef FPThreadsCriticalSection FCriticalSection;
typedef FSystemWideCriticalSectionNotImplemented FSystemWideCriticalSection;
typedef FPThreadsRWLock FRWLock;
