#include "stdafx.h"
#include "CreationEvent.h"
#include "EventSink.h"

void CreationEvent::registerCreationCallback(TNotificationFunc callback) {
    CComPtr<IWbemLocator> pLoc;
    CoInitializeEx(0, COINIT_MULTITHREADED);
    HRESULT hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLoc);

    if (FAILED(hres)) {
        cout << "Failed to create IWbemLocator object."
            << " Err code = 0x"
            << hex << hres << endl;
        throw std::exception("CreationEvent initialization failed");
    }
    CComPtr<EventSink> pSink(new EventSink(callback));

    hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSink->pSvc);
    if (FAILED(hres)) {
        cout << "Could not connect. Error code = 0x" << hex << hres << endl;
        throw std::exception("CreationEvent initialization failed");
    }
    hres = CoSetProxyBlanket(pSink->pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
    if (FAILED(hres)) {
        cout << "Coult not set proxy blanket, Error code =0x" << hex << hres << endl;
        throw std::exception("CreationEvent initialization failed");
    }

    CComPtr<IUnsecuredApartment> pUnsecApp;
    hres = CoCreateInstance(CLSID_UnsecuredApartment, NULL, CLSCTX_LOCAL_SERVER, IID_IUnsecuredApartment, (void**)&pUnsecApp);
    CComPtr<IUnknown> pStubUnk;
    pUnsecApp->CreateObjectStub(pSink, &pStubUnk);
    pStubUnk->QueryInterface(IID_IWbemObjectSink, (void**)&pSink->pStubSink);


    char buffer[512];
    //sprintf_s(buffer, "SELECT * FROM __InstanceCreationEvent WITHIN 1 WHERE TargetInstance ISA 'Win32_Process' AND TargetInstance.Name = 'iexplore.exe'");
    sprintf_s(buffer, "SELECT * FROM __InstanceCreationEvent WITHIN 1 WHERE TargetInstance ISA 'Win32_Process'");
    //sprintf_s(buffer, "SELECT * FROM Win32_ProcessTrace");
    //sprintf_s(buffer, "SELECT * FROM __InstanceCreationEvent WITHIN 1");

    hres = pSink->pSvc->ExecNotificationQueryAsync(_bstr_t("WQL"), _bstr_t(buffer), WBEM_FLAG_SEND_STATUS, NULL, pSink->pStubSink);

    if (FAILED(hres)) {
        cout << "ExecNotificationQueryAsync failed with = 0x" << hex << hres << endl;
        throw std::exception("CreationEvent initialization failed");
    }
}
