#include "stdafx.h"
#include "EventSink.h"
#include "Client.h"

ULONG STDMETHODCALLTYPE EventSink::AddRef()
{
    return InterlockedIncrement(&m_IRef);
}

ULONG STDMETHODCALLTYPE EventSink::Release()
{
    LONG IRef = InterlockedDecrement(&m_IRef);
    if (IRef == 0)
        delete this;
    return IRef;
}

HRESULT STDMETHODCALLTYPE EventSink::QueryInterface(REFIID riid, void** ppv)
{
    if (riid == IID_IUnknown || riid == IID_IWbemObjectSink) {
        *ppv = (IWbemObjectSink*)this;
        AddRef();
        return WBEM_S_NO_ERROR;
    }
    else return E_NOINTERFACE;
}

HRESULT STDMETHODCALLTYPE EventSink::Indicate(
    LONG lObjectCount,
    IWbemClassObject __RPC_FAR* __RPC_FAR* apObjArray
)
{
    HRESULT hr = S_OK;
    _variant_t vtProp;

    for (int i = 0; i < lObjectCount; i++)
    {
        DWORD pid = 0;
        std::wstring processName, commandLine;

        hr = apObjArray[i]->Get(_bstr_t(L"TargetInstance"), 0, &vtProp, 0, 0);
        if (!FAILED(hr))
        {
            IUnknown* str = vtProp;
            hr = str->QueryInterface(IID_IWbemClassObject, reinterpret_cast<void**>(&apObjArray[i]));
            if (SUCCEEDED(hr))
            {
                _variant_t cn;
                hr = apObjArray[i]->Get(L"Caption", 0, &cn, NULL, NULL);
                if (SUCCEEDED(hr))
                {
                    if ((cn.vt == VT_NULL) || (cn.vt == VT_EMPTY))
                        wcout << "Caption : " << ((cn.vt == VT_NULL) ? "NULL" : "EMPTY") << endl;
                    else if ((cn.vt & VT_ARRAY))
                        wcout << "Caption : " << "Array types not supported (yet)" << endl;
                    else {
                        wcout << "Caption : " << cn.bstrVal << endl;
                        processName = cn.bstrVal;
                    }
                }
                VariantClear(&cn);

                hr = apObjArray[i]->Get(L"CommandLine", 0, &cn, NULL, NULL);
                if (SUCCEEDED(hr))
                {
                    if ((cn.vt == VT_NULL) || (cn.vt == VT_EMPTY))
                        wcout << "CommandLine : " << ((cn.vt == VT_NULL) ? "NULL" : "EMPTY") << endl;
                    else if ((cn.vt & VT_ARRAY))
                        wcout << "CommandLine : " << "Array types not supported (yet)" << endl;
                    else {
                        wcout << "CommandLine : " << cn.bstrVal << endl;
                        commandLine = cn.bstrVal;
                    }
                }
                VariantClear(&cn);

                hr = apObjArray[i]->Get(L"Handle", 0, &cn, NULL, NULL);
                if (SUCCEEDED(hr))
                {
                    if ((cn.vt == VT_NULL) || (cn.vt == VT_EMPTY))
                        wcout << "Handle : " << ((cn.vt == VT_NULL) ? "NULL" : "EMPTY") << endl;
                    else if ((cn.vt & VT_ARRAY))
                        wcout << "Handle : " << "Array types not supported (yet)" << endl;
                    else {
                        wcout << "Handle : " << cn.bstrVal << endl;
                        pid = (DWORD)std::stoi(cn.bstrVal);
                    }
                }
                VariantClear(&cn);
            }

            if (pid != 0) {
                writeProcessEvent(pid, processName, L"process_created");
            }
        }
        VariantClear(&vtProp);
    }

    if (m_callback != nullptr)
        m_callback();

    /* Unregister event sink */
    //pSvc->CancelAsyncCall(pStubSink);

    return WBEM_S_NO_ERROR;
}

HRESULT STDMETHODCALLTYPE EventSink::SetStatus(LONG IFlags, HRESULT hResult, BSTR strParam, IWbemClassObject __RPC_FAR* pObjParam)
{
    return WBEM_S_NO_ERROR;
}
