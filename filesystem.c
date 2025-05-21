#include "filesystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_SIZE 1024

FILE* fs_open(const char* filename) {
    FILE* fs = fopen(filename, "a+");
    if (!fs) {
        perror("Failed to open or create filesystem");
        exit(EXIT_FAILURE);
    }
    return fs;
}

int fs_create_file(FILE* fs, const char* filename, const char* content) {
    // Проверка существования файла
    fseek(fs, 0, SEEK_SET);
    char buffer[MAX_SIZE];
    while (fgets(buffer, MAX_SIZE, fs)) {
        buffer[strcspn(buffer, "\n")] = 0;
        if (strcmp(buffer, filename) == 0) {
            fprintf(stderr, "Error: File '%s' already exists\n", filename);
            return 0;
        }
    }
    
    // Создание нового файла
    fseek(fs, 0, SEEK_END);
    if (fprintf(fs, "%s\n%s\n/\n", filename, content) < 0) {
        perror("Failed to write to filesystem");
        return 0;
    }
    return 1;
}

char* fs_view_file(FILE* fs, const char* filename) {
    fseek(fs, 0, SEEK_SET);
    char buffer[MAX_SIZE];
    char* content = NULL;
    size_t content_size = 0;
    int found = 0;

    while (fgets(buffer, MAX_SIZE, fs)) {
        buffer[strcspn(buffer, "\n")] = 0;
        
        if (!found) {
            if (strcmp(buffer, filename) == 0) found = 1;
        } else {
            if (buffer[0] == '/') break;
            
            size_t line_len = strlen(buffer);
            char* temp = realloc(content, content_size + line_len + 2);
            if (!temp) {
                free(content);
                return NULL;
            }
            content = temp;
            
            if (content_size > 0) {
                strcat(content, "\n");
                content_size++;
            }
            strcat(content, buffer);
            content_size += line_len;
        }
    }
    
    return content;
}

int fs_delete_file(FILE** fs, const char* filename, const char* fs_filename) {
    fseek(*fs, 0, SEEK_SET);
    
    // Создаем временный файл
    char temp_filename[] = "temp_fs.XXXXXX";
    int fd = mkstemp(temp_filename);
    if (fd == -1) {
        perror("Failed to create temporary file");
        return 0;
    }
    
    FILE* temp_fs = fdopen(fd, "w+");
    if (!temp_fs) {
        perror("Failed to open temporary filesystem");
        close(fd);
        return 0;
    }
    
    // Копируем все кроме удаляемого файла
    int skip_mode = 0;
    int deleted = 0;
    char buffer[MAX_SIZE];
    
    while (fgets(buffer, MAX_SIZE, *fs)) {
        buffer[strcspn(buffer, "\n")] = 0;
        
        if (!skip_mode) {
            if (strcmp(buffer, filename) == 0) {
                skip_mode = 1;
                deleted = 1;
                continue;
            }
            fprintf(temp_fs, "%s\n", buffer);
        } else {
            if (buffer[0] == '/') {
                skip_mode = 0;
                fprintf(temp_fs, "%s\n", buffer);
            }
        }
    }
    
    // Заменяем оригинальный файл
    fclose(*fs);
    fclose(temp_fs);
    
    if (remove(fs_filename) != 0) {
        perror("Failed to remove original filesystem");
        return 0;
    }
    
    if (rename(temp_filename, fs_filename) != 0) {
        perror("Failed to rename temporary filesystem");
        return 0;
    }
    
    *fs = fopen(fs_filename, "r+");
    return deleted;
}

int fs_modify_file(FILE** fs, const char* filename, const char* new_content, const char* fs_filename) {
    fseek(*fs, 0, SEEK_SET);
    
    // Создаем временный файл
    char temp_filename[] = "temp_fs.XXXXXX";
    int fd = mkstemp(temp_filename);
    if (fd == -1) {
        perror("Failed to create temporary file");
        return 0;
    }
    
    FILE* temp_fs = fdopen(fd, "w+");
    if (!temp_fs) {
        perror("Failed to open temporary filesystem");
        close(fd);
        return 0;
    }
    
    // Копируем с заменой содержимого
    int modify_mode = 0;
    int modified = 0;
    char buffer[MAX_SIZE];
    
    while (fgets(buffer, MAX_SIZE, *fs)) {
        buffer[strcspn(buffer, "\n")] = 0;
        
        if (!modify_mode) {
            if (strcmp(buffer, filename) == 0) {
                modify_mode = 1;
                modified = 1;
                fprintf(temp_fs, "%s\n%s\n", buffer, new_content);
                continue;
            }
            fprintf(temp_fs, "%s\n", buffer);
        } else {
            if (buffer[0] == '/') {
                modify_mode = 0;
                fprintf(temp_fs, "/\n");
            }
        }
    }
    
    // Заменяем оригинальный файл
    fclose(*fs);
    fclose(temp_fs);
    
    if (remove(fs_filename) != 0) {
        perror("Failed to remove original filesystem");
        return 0;
    }
    
    if (rename(temp_filename, fs_filename) != 0) {
        perror("Failed to rename temporary filesystem");
        return 0;
    }
    
    *fs = fopen(fs_filename, "r+");
    return modified;
}
