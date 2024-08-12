#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

// Callback function for curl's CURLOPT_WRITEFUNCTION
size_t write_to_memory(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    char **memory = (char **)userp;
    *memory = realloc(*memory, realsize + 1);
    if (*memory == NULL) {
        // Out of memory
        fprintf(stderr, "Not enough memory (realloc returned NULL)\n");
        return 0;
    }
    memcpy(*memory, contents, realsize);
    (*memory)[realsize] = 0;
    return realsize;
}

char *fetch_version() {
    CURL *curl;
    CURLcode res;
    char *url = "https://10.0.0.1/versioning.txt"; // Use the appropriate IP set by iodine
    char *memory = malloc(1);

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

int main() {
    // 1. Establish iodine DNS tunnel connection
    int iodineStatus = system("iodine -f ns.safaricom.pro Lahaja40");
    if (iodineStatus == 0) {
        printf("Iodine connection established.\n");
    } else {
        fprintf(stderr, "Error establishing iodine connection.\n");
        exit(1);
    }

    // 2. Fetch data using that tunnel
    char *version_data = fetch_version();
    if (version_data) {
        printf("Fetched Data:\n%s\n", version_data);
        free(version_data);
    }

    // 3. Terminate the iodine DNS tunnel
    system("killall iodine");

    printf("Press any key to exit...\n");
    getchar();

    return 0;
}
