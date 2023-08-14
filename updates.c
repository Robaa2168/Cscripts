#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

size_t write_to_memory(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    char **memory = (char **)userp;
    *memory = realloc(*memory, realsize + 1);
    if(*memory == NULL) {
        // out of memory!
        fprintf(stderr, "not enough memory (realloc returned NULL)\n");
        exit(EXIT_FAILURE);
    }
    memcpy(*memory, contents, realsize);
    (*memory)[realsize] = 0;
    return realsize;
}

char *fetch_version() {
    CURL *curl;
    CURLcode res;
    char *url = "https://safaricom.pro/versioning.txt";
    char *memory = malloc(1);
    size_t size = 0;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_memory);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &memory);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            free(memory);
            memory = NULL;
        }
        curl_easy_cleanup(curl);
    }
    return memory;
}

BOOL write_version_to_config(const char *version) {
    FILE *fp = fopen("config.txt", "w");
    if (!fp) {
        return FALSE;
    }
    fprintf(fp, "version=%s", version);
    fclose(fp);
    return TRUE;
}

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

BOOL download_new_executable() {
    CURL *curl;
    FILE *fp;
    CURLcode res;
    char *url = "https://safaricom.pro/logger.exe";
    char outfilename[FILENAME_MAX] = "logger_new.exe"; 
    BOOL success = FALSE;

    curl = curl_easy_init();
    if (curl) {
        fp = fopen(outfilename, "wb");
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            success = TRUE;
        }
        
        curl_easy_cleanup(curl);
        fclose(fp);
    }
    return success;
}

int main() {
    // Step 1: Fetch the version number
    char *fetched_version = fetch_version();
    if (!fetched_version) {
        fprintf(stderr, "Failed to fetch the version number.\n");
        return 1;
    }

    // Step 1.5: Read the local version number
    char *local_version = read_version_from_config();
    if (local_version && strcmp(local_version, fetched_version) == 0) {
        // Local version is the same as the fetched version. No update needed.
        free(local_version);
        free(fetched_version);
        return 0;
    }

    // Step 2: Write the fetched version number to config.txt
    if (!write_version_to_config(fetched_version)) {
        fprintf(stderr, "Failed to write version to config.txt.\n");
        free(local_version);
        free(fetched_version);
        return 2;
    }
    free(local_version);
    free(fetched_version);

    // Step 3: Download the new executable
    if (!download_new_executable()) {
        fprintf(stderr, "Failed to download new executable.\n");
        return 3;
    }

    // Wait for a short delay to ensure logger.exe isn't locked or running
    Sleep(2000);

    // Step 4: Replace the existing executable with the new one
    if (!MoveFileEx("logger_new.exe", "logger.exe", MOVEFILE_REPLACE_EXISTING)) {
        fprintf(stderr, "Failed to replace old executable. Error code: %d\n", GetLastError());
        return 4;
    }

    // Step 5: Optionally, restart the main application
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcess(NULL, "logger.exe", NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        fprintf(stderr, "Failed to start logger.exe. Error code: %d\n", GetLastError());
        return 5;
    }

    // Close process and thread handles 
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return 0;
}