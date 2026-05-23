#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sticker.h"
#include "storage.h"

void album_interactive(Sticker stickers[], int count, char team_code[], char message[]) {

    char input[32];

    while (1) {

        // clear screen
        // system("clear");
        printf("\033[2J\033[H");

        album_page(stickers, team_code, message);

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

            sticker_remove(stickers, full_code, message);
        }
        else {
            char number[8];
            char full_code[16];

            sscanf(input, "%s", number);

            sprintf(full_code, "%s%02d", team_code, atoi(number));

            sticker_add(stickers, full_code, message);
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
    if (argc < 2 || strcmp(argv[1], "help") == 0) {
        printf("\nStracker WC2026 - Sticker Album CLI\n");
        printf("Usage: stracker <command> [args]\n\n");
        printf("Commands:\n");
        printf("  list <missing|have|duplicate>  [TEAM...] [--oneline]\n");
        printf("  add <CODE>\n");
        printf("  remove <CODE>\n");
        printf("  album <TEAM>\n");
        printf("  compare <your_db> <their_db>\n");
        printf("  help\n\n");
        printf("Examples:\n");
        printf("  ./stracker list missing\n");
        printf("  ./stracker list d mex arg --oneline\n");
        printf("  ./stracker add MEX01\n");
        printf("  ./stracker album MEX\n");
        printf("  ./stracker compare my.dat friend.dat\n");
        return 0;
    }
    else {
        if (strcmp(argv[1], "list") == 0) {
            if (argc < 3) {
                fprintf(stderr, "error: missing status argument\n");
                fprintf(stderr, "usage: stracker list <missing|have|duplicate> [TEAM...] [--oneline]\n");
                printf("Example: ./stracker list h\n");
                return 1;
            }
            sticker_list(stickers, argc, argv, message);
        }
        else if (strcmp(argv[1], "add") == 0) {
            if (argc < 3) {
                fprintf(stderr, "error: missing sticker code\n");
                fprintf(stderr, "usage: stracker add <CODE>\n");
                return 1;
            }
            sticker_add(stickers, argv[2], message);
            save_db("storage.dat", stickers, count);
        }
        else if (strcmp(argv[1], "remove") == 0) {
            if (argc < 3) {
                fprintf(stderr, "error: missing sticker code\n");
                fprintf(stderr, "usage: stracker remove <CODE>\n");
                return 1;
            }
            sticker_remove(stickers, argv[2], message);
            save_db("storage.dat", stickers, count);
        }
        else if (strcmp(argv[1], "album") == 0) {
            if (argc < 3) {
                fprintf(stderr, "error: missing team code\n");
                fprintf(stderr, "usage: stracker album <TEAM>\n");
                fprintf(stderr, "Example: ./stracker album MEX\n");
                return 1;
            }
            album_interactive(stickers, count, argv[2], message);
        }
        else if (strcmp(argv[1], "compare") == 0) {
            if (argc < 4) {
                fprintf(stderr, "error: missing file arguments\n");
                fprintf(stderr, "usage: stracker compare <your_storage> <their_storage>\n");
                return 1;
            }
            int count_a = MAX_STICKERS, count_b = MAX_STICKERS;
            Sticker *a = load_db(argv[2], &count_a);
            Sticker *b = load_db(argv[3], &count_b);
            if (!a || !b) {
                fprintf(stderr, "error: could not load one or both storage files\n");
                return 1;
            }
            printf("Stickers they have that you need (they can give you):\n");
            int found = 0;
            for (int i = 0; i < MAX_STICKERS; i++) {
                if (stickers[i].status == MISSING && b[i].status != MISSING) {
                    printf("  %s - %s\n", stickers[i].code, stickers[i].name);
                    found++;
                }
            }
            if (!found) printf("  (none)\n");

            printf("\nStickers you have extra that they need (you can give them):\n");
            found = 0;
            for (int i = 0; i < MAX_STICKERS; i++) {
                if (stickers[i].status == DUPLICATE && b[i].status == MISSING) {
                    printf("  %s - %s\n", stickers[i].code, stickers[i].name);
                    found++;
                }
            }
            if (!found) printf("  (none)\n");

            free(a);
            free(b);
        }
        else {
            fprintf(stderr, "error: unknown command '%s'\n", argv[1]);
            fprintf(stderr, "run 'stracker help' for usage\n");
            return 1;
        }
    }

    return 0;
}
