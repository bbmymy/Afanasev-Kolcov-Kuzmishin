#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SIZE 1024

// 1.1 Функция открытия/создания файловой системы
FILE* open_or_create_fs(const char* filename) {
    FILE* fs = fopen(filename, "a+"); // Открываем для чтения и записи (создаем если не существует)
    if (fs == NULL) {
        perror("Failed to open or create filesystem");
        exit(EXIT_FAILURE);
    }
    return fs;
}

// 1.2 Функция просмотра содержимого файла
char* view_file_in_fs(FILE* fs, const char* filename) {
    fseek(fs, 0, SEEK_SET); // Перемещаемся в начало файла
    
    char buffer[MAX_SIZE];
    char* content = NULL;
    int found = 0;
    size_t content_size = 0;
    
    while (fgets(buffer, MAX_SIZE, fs) != NULL) {
        // Удаляем символ новой строки
        buffer[strcspn(buffer, "\n")] = 0;
        
        if (!found) {
            // Проверяем, является ли строка именем файла
            if (strcmp(buffer, filename) == 0) {
                found = 1;
            }
        } else {
            // Проверяем конец содержимого файла
            if (buffer[0] == '/') {
                break;
            }
            
            // Добавляем строку к содержимому
            size_t line_len = strlen(buffer);
            char* temp = realloc(content, content_size + line_len + 2); // +2 для \n и \0
            if (temp == NULL) {
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

// 1.3 Функция удаления файла
int delete_file_in_fs(FILE** fs, const char* filename, const char* fs_filename) {
    fseek(*fs, 0, SEEK_SET);
    
    char buffer[MAX_SIZE];
    char temp_filename[] = "temp_fs.XXXXXX";
    int fd = mkstemp(temp_filename);
    FILE* temp_fs = fdopen(fd, "w+");
    
    if (temp_fs == NULL) {
        perror("Failed to create temporary filesystem");
        return 0;
    }
    
    int skip_mode = 0;
    int deleted = 0;
    
    while (fgets(buffer, MAX_SIZE, *fs) != NULL) {
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
    
    fclose(*fs);
    fclose(temp_fs);
    
    remove(fs_filename);
    rename(temp_filename, fs_filename);
    
    *fs = fopen(fs_filename, "r+");
    return deleted;
}
