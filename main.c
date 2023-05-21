#include <stdio.h>
#include <comdef.h>
#include <Wbemidl.h>

int main() {
    HRESULT hres;

    // Step 1: Initialize COM
    hres = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hres)) {
        printf("Failed to initialize COM library. Error code: 0x%lx\n", hres);
        return 1;
    }

    // Step 2: Initialize security
    hres = CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE,
        NULL
    );
    if (FAILED(hres)) {
        CoUninitialize();
        printf("Failed to initialize security. Error code: 0x%lx\n", hres);
        return 1;
    }

    // Step 3: Obtain the initial locator to WMI
    IWbemLocator* pLoc = NULL;
    hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        (LPVOID*)&pLoc
    );
    if (FAILED(hres)) {
        CoUninitialize();
        printf("Failed to create IWbemLocator object. Error code: 0x%lx\n", hres);
        return 1;
    }

    // Step 4: Connect to WMI through the IWbemLocator::ConnectServer method
    IWbemServices* pSvc = NULL;
    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\WMI"),
        NULL,
        NULL,
        _bstr_t(L""),
        0,
        NULL,
        NULL,
        &pSvc
    );
    if (FAILED(hres)) {
        pLoc->Release();
        CoUninitialize();
        printf("Could not connect to WMI namespace. Error code: 0x%lx\n", hres);
        return 1;
    }

    // Step 5: Execute a query to retrieve CPU temperature
    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        _bstr_t(L"WQL"),
        _bstr_t(L"SELECT * FROM MSAcpi_ThermalZoneTemperature"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator
    );
    if (FAILED(hres)) {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        printf("Query failed. Error code: 0x%lx\n", hres);
        return 1;
    }

    // Step 6: Retrieve the CPU temperature data from the query result
    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;

    while (pEnumerator) {
        hres = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

        if (uReturn == 0) {
            break;
        }

        VARIANT vtProp;
        hres = pclsObj->Get(L"CurrentTemperature", 0, &vtProp, 0, 0);
        if (SUCCEEDED(hres)) {
            // Convert the temperature value to Celsius
            double temperatureCelsius = (double)vtProp.intVal / 100.0;
            wprintf(L"CPU Temperature: %.1f\u00B0C\n", temperatureCelsius);

            VariantClear(&vtProp);
        }

        pclsObj->Release();
    }
        
    // Step 7: Cleanup
    pEnumerator->Release();
    pSvc->Release();
    pLoc->Release();
    CoUninitialize();

    return 0;
}






