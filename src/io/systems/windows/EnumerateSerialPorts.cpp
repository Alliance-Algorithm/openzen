#define NOMINMAX
#include "windows.h"

#include "EnumerateSerialPorts.h"

// This is derived from CEnumerateSerial found here: http://www.naughter.com/enumser.html
// referenced here https://stackoverflow.com/a/1394301/8680401.
//
// Copyright header on the original source:
/*
Copyright(c) 1998 - 2019 by PJ Naughter(Web: www.naughter.com, Email : pjna@naughter.com)

All rights reserved.

Copyright / Usage Details :

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 
*/
// I don't think this qualifies as a version of the original code in the sense described above, as it
// cannot be used in the original's stead, this code isn't a drop-in replacement even for
// CEnumerateSerial::UsingRegistry().
// It has at least the following behavioral differences:
//  1) no inconsistent trailing NUL-characters in output
//  2) output is sorted
//  3) restricted to one-byte characters
//  4) doesn't call SetLastError()
// Also there are a number of implementation differences:
//  1) no _in_, _out_ etc annotations, no warning suppressions
//  2) no ATL used
//  3) automatic registry handle cleanup via gsl::finally
//  4) registry type check already during length check
//  5) main loop structure simplified
//  6) indentation, function names, "::" prefixes, etc. in line with OpenZen habits

#include <algorithm>
#include <string>
#include <vector>

#include "gsl/gsl"

namespace {
    // Gets the string valued registry key sans trailing NULs.
    bool GetRegistryString(HKEY key, LPCSTR lpValueName, std::string& sValue)
    {
        // Query for the size of the registry value
        ULONG nBytes = 0;
        DWORD dwType = 0;
        LSTATUS nStatus = ::RegQueryValueExA(key, lpValueName, nullptr, &dwType, nullptr, &nBytes);
        if (nStatus != ERROR_SUCCESS)
            return false;
        if ((dwType != REG_SZ) && (dwType != REG_EXPAND_SZ))
            return false;

        // Allocate enough bytes for the return value
        sValue.resize(static_cast<size_t>(nBytes));

        // Call again, now actually loading the string.
        nStatus = ::RegQueryValueExA(key, lpValueName, nullptr, &dwType,
            reinterpret_cast<LPBYTE>(sValue.data()), &nBytes);
        if (nStatus != ERROR_SUCCESS)
            return false;

        // COM1 at least has a NUL in the end, clean up.
        sValue.erase(std::find(sValue.begin(), sValue.end(), '\0'), sValue.end());
        return true;
    }
}

bool zen::EnumerateSerialPorts(std::vector<PortAndSerial>& vAvailablePortAndSerial)
{
    // Find all known SiLabs devices in VCP mode.
    std::vector<PortAndSerial> vPortAndSerialAll;
    {
        HKEY hKey = 0;
        auto closeReg = gsl::finally([&hKey]() { if (hKey) RegCloseKey(hKey); });
        // This key contains all devices with this known vendor and product id. pid = EA60 is VCP mode, EA61 is USBXpress
        LSTATUS nStatus = ::RegOpenKeyExA(HKEY_LOCAL_MACHINE, R"(SYSTEM\CurrentControlSet\Enum\USB\VID_10C4&PID_EA60)", 0,
            KEY_QUERY_VALUE | KEY_READ, &hKey);
        if (nStatus != ERROR_SUCCESS)
            return false;

        // Get the max value name and max value lengths
        DWORD cSubKeys = 0;
        nStatus = ::RegQueryInfoKeyA(hKey, nullptr, nullptr, nullptr, &cSubKeys, nullptr,
            nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
        if (nStatus != ERROR_SUCCESS)
            return false;

        for (DWORD i = 0; i < cSubKeys; i++)
        {
            std::vector<char> vSerialNumber(16383);
            DWORD len = (DWORD)vSerialNumber.size();
            auto retCode = RegEnumKeyExA(hKey, i,
                vSerialNumber.data(),
                &len,
                nullptr,
                nullptr,
                nullptr,
                nullptr);

            if (retCode != ERROR_SUCCESS)
                continue;
            auto serialNumber = std::string(vSerialNumber.data(), len);

            HKEY hSubKey = 0;
            auto closeSubKey = gsl::finally([&hSubKey] { if (hSubKey) RegCloseKey(hSubKey); });
            std::string sKey = std::string(serialNumber) + "\\Device Parameters";
            nStatus = ::RegOpenKeyExA(hKey, sKey.c_str(), 0, KEY_QUERY_VALUE | KEY_READ, &hSubKey);
            if (nStatus != ERROR_SUCCESS)
                continue;

            std::string sPortName;
            if (::GetRegistryString(hSubKey, "PortName", sPortName)) {
                vPortAndSerialAll.push_back(PortAndSerial{ sPortName, serialNumber });
            }
        }
    }

    // Filter for the ones associated to a live COM port.
    vAvailablePortAndSerial = {};
    {
        HKEY hKey = 0;
        auto closeReg = gsl::finally([&hKey]() { if (hKey) RegCloseKey(hKey); });
        LSTATUS nStatus = ::RegOpenKeyExA(HKEY_LOCAL_MACHINE, R"(HARDWARE\DEVICEMAP\SERIALCOMM)", 0,
            KEY_QUERY_VALUE, &hKey);
        if (nStatus != ERROR_SUCCESS)
            return false;

        // Get the max value name and max value lengths
        DWORD dwMaxValueNameLen = 0;
        nStatus = ::RegQueryInfoKeyA(hKey, nullptr, nullptr, nullptr, nullptr, nullptr,
            nullptr, nullptr, &dwMaxValueNameLen, nullptr, nullptr, nullptr);
        if (nStatus != ERROR_SUCCESS)
            return false;

        const DWORD dwMaxValueNameSizeInChars = dwMaxValueNameLen + 1; //Include space for the null terminator

        // Enumerate all the values underneath HKEY_LOCAL_MACHINE\HARDWARE\DEVICEMAP\SERIALCOMM
        DWORD dwIndex = 0;
        std::vector<char> valueName(dwMaxValueNameSizeInChars, 0);
        while (true) {
            DWORD dwValueNameSize = dwMaxValueNameSizeInChars;
            if (::RegEnumValueA(hKey, dwIndex++, valueName.data(), &dwValueNameSize,
                nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS)
                break;

            std::string sPortName;
            if (::GetRegistryString(hKey, valueName.data(), sPortName)) {
                // Insert this port into list if it is an SiLabs device, i.e. if it is included
                // in the previous list.
                for (auto& pas : vPortAndSerialAll) {
                    if (pas.port == sPortName)
                        vAvailablePortAndSerial.emplace_back(pas);
                }
            }
        }
    }

    // Sort the output.
    std::sort(vAvailablePortAndSerial.begin(), vAvailablePortAndSerial.end(),
        [](const PortAndSerial& left, const PortAndSerial& right) -> bool {
            // Strings are either COMx, COMxx or COMxxx with no leading zeros.
            // COMx comes before COMxx, COMxxx etc.
            if (left.port.size() != right.port.size())
                return left.port.size() < right.port.size();
            // Compare the digit strings which are guaranteed to be of the same length.
            return left.port.substr(3) < right.port.substr(3);
        });

    return true;
}
