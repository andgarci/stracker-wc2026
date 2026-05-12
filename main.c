#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sticker.h"
#include "storage.h"

void album_interactive(Sticker stickers[], int count, char team_code[], char message[]) {

    char input[32];

    while (1) {

        // clear screen
        system("clear");

        album_page(stickers, team_code);

        printf("\n%s\n", message);
        strcpy(message, "");
        printf("\nCommands:\n");
        printf("CODE   -> add sticker\n");
        printf("-CODE  -> remove sticker\n");
        printf("q      -> quit\n\n");

        printf("> ");

        fgets(input, sizeof(input), stdin);

        // remove newline
        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "q") == 0) {
            break;
        }
        else if (input[0] == '-') {

            char number[8];
            char full_code[16];

            sscanf(input + 1, "%s", number);

            sprintf(full_code, "%s%02d", team_code, atoi(number));

            sticker_remove(stickers, &count, full_code, message);
        }
        else {
            char number[8];
            char full_code[16];

            sscanf(input, "%s", number);

            sprintf(full_code, "%s%02d", team_code, atoi(number));

            sticker_add(stickers, &count, full_code, message);
        }

        save_db("storage.dat", stickers, count);
    }
}

int main(int argc, char *argv[]) {
    int count = MAX_STICKERS;
    Sticker *stickers = load_db("storage.dat", &count);
    char message[256];
    if (!stickers) {
        save_db("storage.dat", CATALOG, count);
        stickers = load_db("storage.dat", &count);
    }
    if (argc < 2) {
        return 0;
    }
    else {
        if (strcmp(argv[1], "list") == 0) {
            sticker_list(stickers, &count, argc, argv, message);
        }
        else if (strcmp(argv[1], "add") == 0) {
            if (argc < 3) return 0;
            sticker_add(stickers, &count, argv[2], message);
            save_db("storage.dat", stickers, count);
        }
        else if (strcmp(argv[1], "remove") == 0) {
            if (argc < 3) return 0;
            sticker_remove(stickers, &count, argv[2], message);
            save_db("storage.dat", stickers, count);
        }
        else if (strcmp(argv[1], "album") == 0) {
            if (argc < 3) {
                return 0;
            }
            album_interactive(stickers, count, argv[2], message);
        }
    }

    return 0;
}
