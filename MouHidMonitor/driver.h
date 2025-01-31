/*++

Copyright (c) 2019 changeofpace. All rights reserved.

Use of this source code is governed by the MIT license. See the 'LICENSE' file
for more information.

--*/

#pragma once

#include <Windows.h>
#include "../Common/ioctl.h"

//=============================================================================
// Meta Interface
//=============================================================================
_Check_return_
BOOL
DrvInitialization();

VOID
DrvTermination();

//=============================================================================
// Public Interface
//=============================================================================
_Check_return_
BOOL
DrvQueryMouHidInputMonitor(
    _Out_ PBOOL pfEnabled
);

_Check_return_
BOOL
DrvEnableMouHidInputMonitor();

_Check_return_
BOOL
DrvDisableMouHidInputMonitor();

_Success_(return)
BOOL
DrvGetInputPacket(
    _Out_ PGET_INPUT_PACKET_REPLY pInputPacket
);
