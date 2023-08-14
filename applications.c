#include <stdio.h>
#include <windows.h>

void ListInstalledApps(const char* registryKey, REGSAM access) {
    HKEY hUninstallKey;
    HKEY hAppKey;
    char szAppName[512];
    char szKeyName[512];
    DWORD dwSize, dwValue, dwType;
    int i = 0;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, registryKey, 0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS | access, &hUninstallKey) != ERROR_SUCCESS) {
        fprintf(stderr, "Error opening key %s: %lu\n", registryKey, GetLastError());
        return;
    }

    while (RegEnumKeyEx(hUninstallKey, i, szKeyName, &dwSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
        if (RegOpenKeyEx(hUninstallKey, szKeyName, 0, KEY_QUERY_VALUE, &hAppKey) == ERROR_SUCCESS) {
            dwValue = sizeof(szAppName);
            if (RegQueryValueEx(hAppKey, "DisplayName", NULL, &dwType, (unsigned char *)szAppName, &dwValue) == ERROR_SUCCESS) {
                printf("%s\n", szAppName);
            } else {
                fprintf(stderr, "Error querying DisplayName for key %s: %lu\n", szKeyName, GetLastError());
            }
            RegCloseKey(hAppKey);
        } else {
            fprintf(stderr, "Error opening subkey %s: %lu\n", szKeyName, GetLastError());
        }
        i++;
        dwSize = sizeof(szKeyName);  // Reset the size for the next iteration.
    }

    RegCloseKey(hUninstallKey);
}

int main() {
    printf("32-bit Applications:\n");
    ListInstalledApps("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall", KEY_WOW64_32KEY);
    
    printf("\n64-bit Applications:\n");
    ListInstalledApps("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall", KEY_WOW64_64KEY);
    
    return 0;
}
