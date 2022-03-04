#pragma once

#define _WIN32_DCOM
#include <iostream>
using namespace std;
#include <comdef.h>
#include <Wbemidl.h>
#include <atlcomcli.h>

namespace CreationEvent {
    typedef void(* TNotificationFunc)();
    void registerCreationCallback(TNotificationFunc callback);
}
