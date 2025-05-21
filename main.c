#include "filesystem.h"
#include <stdio.h>
#include <string.h>

void print_help() {
    printf("Доступные команды:\n");
    printf("создать <имя_файла> <содержимое>\n");
    printf("удалить <имя_файла>\n");
    printf("изменить <имя_файла> <новое_содержимое>\n");
    printf("просмотреть <имя_файла>\n");
    printf("выход\n");
}

int main() {
    FILE* fs = fs_open("disk.fs");
    char command[50], filename[50], content[1024];
    
    printf("AKMO Filesystem v1.0\n");
    print_help();
    
    while(1) {
        printf("\n> ");
        scanf("%s", command);
        
        if (strcmp(command, "выход") == 0) break;
        
        if (strcmp(command, "создать") == 0) {
            scanf("%s %[^\n]", filename, content);
            if (fs_create_file(fs, filename, content))
                printf("Файл '%s' создан\n", filename);
        }
        else if (strcmp(command, "просмотреть") == 0) {
            scanf("%s", filename);
            char* file_content = fs_view_file(fs, filename);
            if (file_content) {
                printf("%s\n", file_content);
                free(file_content);
            } else printf("Файл не найден\n");
        }
        /* ... другие команды ... */
    }
    
    fclose(fs);
    return 0;
}
