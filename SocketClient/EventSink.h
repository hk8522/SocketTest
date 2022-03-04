#pragma once

#define _WIN32_DCOM
#include <iostream>
using namespace std;
#include <comdef.h>
#include <Wbemidl.h>
#include <atlcomcli.h>

#pragma comment(lib, "wbemuuid.lib")
#include "CreationEvent.h"

class EventSink : public IWbemObjectSink {
    friend void CreationEvent::registerCreationCallback(TNotificationFunc callback);

    CComPtr<IWbemServices> pSvc;
    CComPtr<IWbemObjectSink> pStubSink;
    LONG m_IRef;
    CreationEvent::TNotificationFunc m_callback;

public:
    EventSink(CreationEvent::TNotificationFunc callback) :m_IRef(0), m_callback(callback) {}
    ~EventSink() {
    }

    virtual ULONG STDMETHODCALLTYPE AddRef();

    virtual ULONG STDMETHODCALLTYPE Release();

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv);

    virtual HRESULT STDMETHODCALLTYPE Indicate(
        LONG lObjectCount,
        IWbemClassObject __RPC_FAR* __RPC_FAR* apObjArray
    );
    virtual HRESULT STDMETHODCALLTYPE SetStatus(LONG IFlags, HRESULT hResult, BSTR strParam, IWbemClassObject __RPC_FAR* pObjParam);
};
