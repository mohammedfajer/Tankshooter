#include "win32_fileIO.h"
#include <windows.h>
#include <stdio.h>

#define BUFSIZE 1024

char* readFile(const char *fileName) {
    HANDLE hFile;
    DWORD dwRead;
    char buffer[BUFSIZE] = {0};
    char *fileContent = NULL;
    
    // Open the file for reading
    hFile = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        printf("Error opening file.\n");
        return NULL;
    }

    // Read from the file
    if (!ReadFile(hFile, buffer, BUFSIZE - 1, &dwRead, NULL)) {
        printf("Error reading file.\n");
        CloseHandle(hFile);
        return NULL;
    }

    // Null-terminate the buffer
    buffer[dwRead] = '\0';

    // Close the file
    CloseHandle(hFile);

    // Allocate memory for the file content
    fileContent = (char*)malloc(dwRead + 1);
    if (fileContent == NULL) {
        printf("Memory allocation failed.\n");
        return NULL;
    }

    // Copy the buffer content to the allocated memory
    strcpy(fileContent, buffer);

    return fileContent;
}