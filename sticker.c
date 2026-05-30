#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "sticker.h"
#include <unistd.h>

#ifdef _WIN32
    #include <windows.h>
    #define SLEEP_MS(ms) Sleep(ms)
#else
    #include <unistd.h>
    #define SLEEP_MS(ms) usleep((ms) * 1000)
#endif

const char *status_names[] = {"MISSING", "HAVE", "DUPLICATE"};
static char current_team[4] = "NAN";


static const char *team_flag(const char *code) {
    static const struct { const char *code; const char *flag; } flags[] = {
        {"ALG", "🇩🇿"}, {"ARG", "🇦🇷"}, {"AUS", "🇦🇺"}, {"AUT", "🇦🇹"},
        {"BEL", "🇧🇪"}, {"BIH", "🇧🇦"}, {"BRA", "🇧🇷"}, {"CAN", "🇨🇦"},
        {"CIV", "🇨🇮"}, {"COD", "🇨🇩"}, {"COL", "🇨🇴"}, {"CPV", "🇨🇻"},
        {"CRO", "🇭🇷"}, {"CUW", "🇨🇼"}, {"CZE", "🇨🇿"}, {"ECU", "🇪🇨"},
        {"EGY", "🇪🇬"}, {"ENG", "🏴󠁧󠁢󠁥󠁮󠁧󠁿"}, {"ESP", "🇪🇸"}, {"FRA", "🇫🇷"},
        {"FWC", "🌍"}, {"GER", "🇩🇪"}, {"GHA", "🇬🇭"}, {"HAI", "🇭🇹"},
        {"IRN", "🇮🇷"}, {"IRQ", "🇮🇶"}, {"JOR", "🇯🇴"}, {"JPN", "🇯🇵"},
        {"KOR", "🇰🇷"}, {"KSA", "🇸🇦"}, {"MAR", "🇲🇦"}, {"MEX", "🇲🇽"},
        {"NED", "🇳🇱"}, {"NOR", "🇳🇴"}, {"NZL", "🇳🇿"}, {"PAN", "🇵🇦"},
        {"PAR", "🇵🇾"}, {"PNI", "🏆"}, {"POR", "🇵🇹"}, {"QAT", "🇶🇦"},
        {"RSA", "🇿🇦"}, {"SCO", "🏴󠁧󠁢󠁳󠁣󠁯󠁿"}, {"SEN", "🇸🇳"}, {"SUI", "🇨🇭"},
        {"SWE", "🇸🇪"}, {"TUN", "🇹🇳"}, {"TUR", "🇹🇷"}, {"URU", "🇺🇾"},
        {"USA", "🇺🇸"}, {"UZB", "🇺🇿"},
    };
    for (size_t i = 0; i < sizeof(flags) / sizeof(flags[0]); i++)
        if (strcmp(flags[i].code, code) == 0) return flags[i].flag;
    return "";
}

void sticker_print(Sticker *sticker, char message[], int oneline) {
    if (oneline) {
        if (strcmp(current_team, sticker->team_code) != 0) {
            printf("\n%s %s: ", sticker->team_code, team_flag(sticker->team_code));
            sprintf(current_team, "%s", sticker->team_code);
            strncpy(current_team, sticker->team_code, 3);
        }
        if (sticker->status == DUPLICATE && sticker->quantity > 2) {
            printf("%s (x%d), ", sticker->code + 3, sticker->quantity - 1);
            sprintf(message + strlen(message), "%s (x%d), ", sticker->code + 3, sticker->quantity - 1);
        } else {
            printf("%s, ", sticker->code + 3);
            sprintf(message + strlen(message), "%s, ", sticker->code + 3);
        }
    }
    else {
        printf("Sticker %s: \n\t Name: %s\n\t Team: %s\n\t Status: %s\n\t Quantity: %d\n\n", 
                sticker->code, sticker->name, sticker->team_code, status_names[sticker->status], sticker->quantity);
        sprintf(message + strlen(message), "Sticker %s: \n\t Name: %s\n\t Team: %s\n\t Status: %s\n\t Quantity: %d\n\n", 
                sticker->code, sticker->name, sticker->team_code, status_names[sticker->status], sticker->quantity);
    }
}

Sticker *sticker_find(Sticker stickers[], char code[], char message[]) {
    char normalized[8];
    int i = 0;
    while (code[i] && isalpha((unsigned char)code[i])) i++;
    if (strlen(code + i) == 1) {
        snprintf(normalized, sizeof(normalized), "%.*s0%s", i, code, code + i);
        code = normalized;
    }
    for (int i = 0; i < MAX_STICKERS ; i++) {
        //printf("sticker code: %s\n code: %s\n", stickers[i].code, code);
        if (strcmp(stickers[i].code, code) == 0) {
            return &stickers[i];
        }
    }

    printf("error: sticker with code %s does not exist\n", code);
    sprintf(message, "error: sticker with code %s does not exist\n", code);
    return NULL;
}

void update_sticker_status(Sticker *sticker) {
    switch(sticker->quantity) {
        case 0:
            sticker->status = MISSING;
            break;
        case 1:
            sticker->status = HAVE;
            break;
        default:
            sticker->status = DUPLICATE;
    }
}


void sticker_add(Sticker stickers[], char code[], char message[]) {
    Sticker *sticker = sticker_find(stickers, code, message);
    if (sticker != NULL) {
        sticker->quantity++;
        update_sticker_status(sticker);
        printf("Successfully added sticker!\n\n");
        sprintf(message, "Successfully added sticker!\n\n");
        sticker_print(sticker, message, 0);
    }
}

int sticker_remove(Sticker stickers[], char code[], char message[]) {
    Sticker *sticker = sticker_find(stickers, code, message);
    if (sticker != NULL) {
        if (sticker->quantity > 0) {
            sticker->quantity--;
            update_sticker_status(sticker);
            printf("Successfully removed sticker!\n\n");
            sprintf(message, "Successfully removed sticker!\n\n");
            sticker_print(sticker, message, 0);
        }
        else {
            return 1;
        }
        return 0;
    }
    return 1;
}

void sticker_list(Sticker stickers[], int argc, char *argv[], char message[]) {
    strcpy(current_team, "NAN");
    if (argc < 3) return;

    else {
        int status = -1;
        if (strcmp(argv[2], "missing") == 0 || strcmp(argv[2], "m") == 0) {
            status = MISSING;
        }
        else if (strcmp(argv[2], "have") == 0 || strcmp(argv[2], "h") == 0) {
            status = HAVE;
        }
        else if (strcmp(argv[2], "duplicate") == 0 || strcmp(argv[2], "d") == 0) {
            status = DUPLICATE;
        }
        if (status == -1) {
            printf("error: Unknown status %s, please try missing/m, have/h or duplicate/d\n", argv[2]);
        }

        int oneline = 0;
        char teams[48][4];
        int team_count = 0;

        for (int i = 3; i < argc; i++) {
            if (strcmp(argv[i], "--oneline") == 0) {
                oneline = 1;
            } 
            else {
                int j = 0;
                while (argv[i][j] && j < 4) {
                    teams[team_count][j] = toupper((unsigned char)argv[i][j]);
                    j++;
                }
                teams[team_count][j] = '\0';
                team_count++;
            }
        }    

        int found = 0;
        int found_d = 0;

        for (int i = 0; i < MAX_STICKERS; i++) {
            if (team_count > 0) {
                int match = 0;
                for (int t = 0; t < team_count; t++) {
                    if (strcmp(stickers[i].team_code, teams[t]) == 0) {
                        match = 1;
                        break;
                    }
                }
                if (!match) continue;
            }

            if (stickers[i].status == (Status)status) {
                sticker_print(&stickers[i], message, oneline);
                strcpy(message, "");
                found++;
            } else if (stickers[i].status == DUPLICATE) {
                found_d++;
            }
        }

        if (status != DUPLICATE) {
            int total_have = (status == MISSING)
                ? MAX_STICKERS - found         // missing shown = not owned
                : found + found_d;             // have + duplicates
            int scope = (team_count > 0) ? found : MAX_STICKERS;
            printf("\n\nYou have %d / %d stickers\n", total_have, scope);
            printf("Album %.1f%% complete\n\n", ((float)total_have / MAX_STICKERS) * 100.0f);

            if (status == MISSING && team_count == 0) {
                char worst_team[4] = "";
                char cur[4] = "";
                int cur_missing = 0, max_missing = 0;
                for (int i = 0; i < MAX_STICKERS; i++) {
                    if (strcmp(stickers[i].team_code, cur) != 0) {
                        if (cur_missing > max_missing) {
                            max_missing = cur_missing;
                            strncpy(worst_team, cur, 4);
                        }
                        strncpy(cur, stickers[i].team_code, 4);
                        cur_missing = 0;
                    }
                    if (stickers[i].status == MISSING) cur_missing++;
                }
                if (cur_missing > max_missing) strncpy(worst_team, cur, 4);
                printf("Most missing team: %s\n", worst_team);
            }
        }
        else {
            int total_extra = 0;
            for (int i = 0; i < MAX_STICKERS; i++)
                if (stickers[i].status == DUPLICATE)
                    total_extra += stickers[i].quantity - 1;
            printf("\n\nYou have %d duplicate stickers\n", found);
            printf("Total tradeable: %d\n", total_extra);
            Sticker *most = NULL;
            for (int i = 0; i < MAX_STICKERS; i++) {
                if (stickers[i].status == DUPLICATE)
                    if (!most || stickers[i].quantity > most->quantity)
                        most = &stickers[i];
            }
            if (most)
                printf("Most duplicated: %s - %s (x%d)\n", most->code, most->name, most->quantity - 1);
        }
    }

}

int album_page_complete(Sticker stickers[], char team_code[],
                        int *layout, int rows, int cols) {
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            int number = layout[r * cols + c];
            if (number == -1) continue;
            char full_code[16];
            sprintf(full_code, "%s%02d", team_code, number);
            // find without printing errors
            for (int i = 0; i < MAX_STICKERS; i++) {
                if (strcmp(stickers[i].code, full_code) == 0) {
                    if (stickers[i].quantity == 0) return 0;
                    break;
                }
            }
        }
    }
    return 1;
}

void animate_complete(char team_code[]) {
    // flash the completion banner 3 times
    for (int i = 0; i < 3; i++) {
        // bold yellow
        printf("\033[1;33m");
        printf("  ★ ★ ★  %s PAGE COMPLETE!  ★ ★ ★\n", team_code);
        printf("\033[0m");
        fflush(stdout);
        SLEEP_MS(300);

        // erase the line
        printf("\033[1A\033[2K");
        fflush(stdout);
        SLEEP_MS(200);
    }

    // final persistent message
    printf("\033[1;32m  ★ ★ ★  %s PAGE COMPLETE!  ★ ★ ★\033[0m\n\n", team_code);
    fflush(stdout);
}

void album_page(Sticker stickers[], char team_code[], char message[]) {

    printf("\n===================== %s =====================\n\n", team_code);
    int fwc_layout[4][12] = {
        { 1, -1, -1,  7, -1, -1, -1, -1, -1, -1, -1, -1},
        { 2, -1,  5, -1, -1, -1, 11, -1, -1, -1, 16, 17},
        { 3, -1, -1,  8, -1,  9, 12, -1, -1, 14, -1, 18},
        { 4, -1,  6, -1, -1, 10, -1, 13, -1, 15, -1, 19}
    };

    int other_layout[3][9] = {
        {-1, -1,  1,  2, 11, 12, 13, -1, -1},
        { 3,  4,  5,  6, 14, 15, 16, 17, -1},
        { 7,  8,  9, 10, -1, 18, 19, 20, -1}
    };

    int *layout;
    int rows, cols;
    if (strcmp(team_code, "FWC") == 0) {
        layout = &fwc_layout[0][0];
        rows = 4; cols = 12;
    }
    else {
        layout = &other_layout[0][0];
        rows = 3; cols = 9;
    }


    for (int row = 0; row < rows; row++) {

        for (int col = 0; col < cols; col++) {

            int number = layout[row * cols + col];

            // empty slot
            if (number == -1) {
                printf("      ");
                continue;
            }

            char full_code[16];

            sprintf(full_code, "%s%02d", team_code, number);

            Sticker *sticker = sticker_find(stickers, full_code, message);

            if (sticker != NULL && sticker->quantity > 0) {
                printf("\033[32m[✓%02d]\033[0m ", number);
            }
            else {
                printf("[ %02d] ", number);
            }
        }

        printf("\n");
    }

    printf("\n");

    if (album_page_complete(stickers, team_code, layout, rows, cols)) {
            animate_complete(team_code);
    }

}


Sticker CATALOG[MAX_STICKERS] = {
    { MISSING, 0, "PNI01", "PNI", "Panini Logo" },
    { MISSING, 0, "FWC01", "FWC", "Official Emblem1" },
    { MISSING, 0, "FWC02", "FWC", "Official Emblem2" },
    { MISSING, 0, "FWC03", "FWC", "Official Mascots" },
    { MISSING, 0, "FWC04", "FWC", "Official Slogan" },
    { MISSING, 0, "FWC05", "FWC", "Official Ball" },
    { MISSING, 0, "FWC06", "FWC", "Canada" },
    { MISSING, 0, "FWC07", "FWC", "Mexico" },
    { MISSING, 0, "FWC08", "FWC", "USA" },
    { MISSING, 0, "MEX01", "MEX", "Emblem" },
    { MISSING, 0, "MEX02", "MEX", "Luis Malagón" },
    { MISSING, 0, "MEX03", "MEX", "Johan Vasquez" },
    { MISSING, 0, "MEX04", "MEX", "Jorge Sánchez" },
    { MISSING, 0, "MEX05", "MEX", "Cesar Montes" },
    { MISSING, 0, "MEX06", "MEX", "Jesus Gallardo" },
    { MISSING, 0, "MEX07", "MEX", "Israel Reyes" },
    { MISSING, 0, "MEX08", "MEX", "Diego Lainez" },
    { MISSING, 0, "MEX09", "MEX", "Carlos Rodriguez" },
    { MISSING, 0, "MEX10", "MEX", "Edson Alvarez" },
    { MISSING, 0, "MEX11", "MEX", "Orbelin Pineda" },
    { MISSING, 0, "MEX12", "MEX", "Marcel Ruiz" },
    { MISSING, 0, "MEX13", "MEX", "Team Photo" },
    { MISSING, 0, "MEX14", "MEX", "Érick Sánchez" },
    { MISSING, 0, "MEX15", "MEX", "Hirving Lozano" },
    { MISSING, 0, "MEX16", "MEX", "Santiago Giménez" },
    { MISSING, 0, "MEX17", "MEX", "Raúl Jiménez" },
    { MISSING, 0, "MEX18", "MEX", "Alexis Vega" },
    { MISSING, 0, "MEX19", "MEX", "Roberto Alvarado" },
    { MISSING, 0, "MEX20", "MEX", "Cesar Huerta" },
    { MISSING, 0, "RSA01", "RSA", "Emblem" },
    { MISSING, 0, "RSA02", "RSA", "Ronwen Williams" },
    { MISSING, 0, "RSA03", "RSA", "Sipho Chaine" },
    { MISSING, 0, "RSA04", "RSA", "Aubrey Modiba" },
    { MISSING, 0, "RSA05", "RSA", "Samukele Kabini" },
    { MISSING, 0, "RSA06", "RSA", "Mbekezeli Mbokazi" },
    { MISSING, 0, "RSA07", "RSA", "Khulumani Ndamane" },
    { MISSING, 0, "RSA08", "RSA", "Siyabonga Ngezana" },
    { MISSING, 0, "RSA09", "RSA", "Khuliso Mudau" },
    { MISSING, 0, "RSA10", "RSA", "Nkosinathi Sibisi" },
    { MISSING, 0, "RSA11", "RSA", "Teboho Mokoena" },
    { MISSING, 0, "RSA12", "RSA", "Thalente Mbatha" },
    { MISSING, 0, "RSA13", "RSA", "Team Photo" },
    { MISSING, 0, "RSA14", "RSA", "Bathasi Aubaas" },
    { MISSING, 0, "RSA15", "RSA", "Yaya Sithole" },
    { MISSING, 0, "RSA16", "RSA", "Sipho Mbule" },
    { MISSING, 0, "RSA17", "RSA", "Lyle Foster" },
    { MISSING, 0, "RSA18", "RSA", "Iqraam Rayners" },
    { MISSING, 0, "RSA19", "RSA", "Mohau Nkota" },
    { MISSING, 0, "RSA20", "RSA", "Oswin Appollis" },
    { MISSING, 0, "KOR01", "KOR", "Emblem" },
    { MISSING, 0, "KOR02", "KOR", "Hyeon-woo Jo" },
    { MISSING, 0, "KOR03", "KOR", "Seung-Gyu Kim" },
    { MISSING, 0, "KOR04", "KOR", "Min-jae Kim" },
    { MISSING, 0, "KOR05", "KOR", "Yu-min Cho" },
    { MISSING, 0, "KOR06", "KOR", "Young-woo Seol" },
    { MISSING, 0, "KOR07", "KOR", "Han-beom Lee" },
    { MISSING, 0, "KOR08", "KOR", "Tae-seok Lee" },
    { MISSING, 0, "KOR09", "KOR", "Myung-jae Lee" },
    { MISSING, 0, "KOR10", "KOR", "Jae-sung Lee" },
    { MISSING, 0, "KOR11", "KOR", "In-beom Hwang" },
    { MISSING, 0, "KOR12", "KOR", "Kang-in Lee" },
    { MISSING, 0, "KOR13", "KOR", "Team Photo" },
    { MISSING, 0, "KOR14", "KOR", "Seung-ho Paik" },
    { MISSING, 0, "KOR15", "KOR", "Jens Castrop" },
    { MISSING, 0, "KOR16", "KOR", "Dongg-yeong Lee" },
    { MISSING, 0, "KOR17", "KOR", "Gue-sung Cho" },
    { MISSING, 0, "KOR18", "KOR", "Heung-min Son" },
    { MISSING, 0, "KOR19", "KOR", "Hee-chan Hwang" },
    { MISSING, 0, "KOR20", "KOR", "Hyeon-Gyu Oh" },
    { MISSING, 0, "CZE01", "CZE", "Emblem" },
    { MISSING, 0, "CZE02", "CZE", "Matej Kovar" },
    { MISSING, 0, "CZE03", "CZE", "Jindrich Stanek" },
    { MISSING, 0, "CZE04", "CZE", "Ladislav Krejci" },
    { MISSING, 0, "CZE05", "CZE", "Vladimir Coufal" },
    { MISSING, 0, "CZE06", "CZE", "Jaroslav Zeleny" },
    { MISSING, 0, "CZE07", "CZE", "Tomas Holes" },
    { MISSING, 0, "CZE08", "CZE", "David Zima" },
    { MISSING, 0, "CZE09", "CZE", "Michal Sadilek" },
    { MISSING, 0, "CZE10", "CZE", "Lukas Provod" },
    { MISSING, 0, "CZE11", "CZE", "Lukas Cerv" },
    { MISSING, 0, "CZE12", "CZE", "Tomas Soucek" },
    { MISSING, 0, "CZE13", "CZE", "Team Photo" },
    { MISSING, 0, "CZE14", "CZE", "Pavel Sulc" },
    { MISSING, 0, "CZE15", "CZE", "Matej Vydra" },
    { MISSING, 0, "CZE16", "CZE", "Vasil Kusej" },
    { MISSING, 0, "CZE17", "CZE", "Tomas Chory" },
    { MISSING, 0, "CZE18", "CZE", "Vaclav Cerny" },
    { MISSING, 0, "CZE19", "CZE", "Adam Hlozek" },
    { MISSING, 0, "CZE20", "CZE", "Patrik Schick" },
    { MISSING, 0, "CAN01", "CAN", "Emblem" },
    { MISSING, 0, "CAN02", "CAN", "Dayne St.Clair" },
    { MISSING, 0, "CAN03", "CAN", "Alphonso Davies" },
    { MISSING, 0, "CAN04", "CAN", "Alistair Johnston" },
    { MISSING, 0, "CAN05", "CAN", "Samuel Adekugbe" },
    { MISSING, 0, "CAN06", "CAN", "Riche Larvea" },
    { MISSING, 0, "CAN07", "CAN", "Derek Cornelius" },
    { MISSING, 0, "CAN08", "CAN", "Moïse Bombito" },
    { MISSING, 0, "CAN09", "CAN", "Kamal Miller" },
    { MISSING, 0, "CAN10", "CAN", "Stephen Eustáquio" },
    { MISSING, 0, "CAN11", "CAN", "Ismaël Koné" },
    { MISSING, 0, "CAN12", "CAN", "Jonathan Osorio" },
    { MISSING, 0, "CAN13", "CAN", "Team Photo" },
    { MISSING, 0, "CAN14", "CAN", "Jacob Shaffelburg" },
    { MISSING, 0, "CAN15", "CAN", "Mathieu Choinière" },
    { MISSING, 0, "CAN16", "CAN", "Niko Sigur" },
    { MISSING, 0, "CAN17", "CAN", "Tajon Buchanan" },
    { MISSING, 0, "CAN18", "CAN", "Liam Millar" },
    { MISSING, 0, "CAN19", "CAN", "Cyle Larin" },
    { MISSING, 0, "CAN20", "CAN", "Jonathan David" },
    { MISSING, 0, "BIH01", "BIH", "Emblem" },
    { MISSING, 0, "BIH02", "BIH", "Nikola Vasilj" },
    { MISSING, 0, "BIH03", "BIH", "Amer Dedic" },
    { MISSING, 0, "BIH04", "BIH", "Sead Kolasinac" },
    { MISSING, 0, "BIH05", "BIH", "Tarik Muharemovic" },
    { MISSING, 0, "BIH06", "BIH", "Nihad Mujakic" },
    { MISSING, 0, "BIH07", "BIH", "Nikola Katic" },
    { MISSING, 0, "BIH08", "BIH", "Amir Hadziahmetovic" },
    { MISSING, 0, "BIH09", "BIH", "Benjamin Tahirovic" },
    { MISSING, 0, "BIH10", "BIH", "Armin Gigovic" },
    { MISSING, 0, "BIH11", "BIH", "Ivan Sunjic" },
    { MISSING, 0, "BIH12", "BIH", "Ivan Basic" },
    { MISSING, 0, "BIH13", "BIH", "Team Photo" },
    { MISSING, 0, "BIH14", "BIH", "Dzenis Burnic" },
    { MISSING, 0, "BIH15", "BIH", "Esmir Bajraktarevic" },
    { MISSING, 0, "BIH16", "BIH", "Amar Memic" },
    { MISSING, 0, "BIH17", "BIH", "Ermedin Demirovic" },
    { MISSING, 0, "BIH18", "BIH", "Edin Dzeko" },
    { MISSING, 0, "BIH19", "BIH", "Samed Bazdar" },
    { MISSING, 0, "BIH20", "BIH", "Haris Tabakovic" },
    { MISSING, 0, "QAT01", "QAT", "Emblem" },
    { MISSING, 0, "QAT02", "QAT", "Meshaal Barsham" },
    { MISSING, 0, "QAT03", "QAT", "Sultan Albrake" },
    { MISSING, 0, "QAT04", "QAT", "Lucas Mendes" },
    { MISSING, 0, "QAT05", "QAT", "Homam Ahmed" },
    { MISSING, 0, "QAT06", "QAT", "Boualem Khoukhi" },
    { MISSING, 0, "QAT07", "QAT", "Pedro Miguel" },
    { MISSING, 0, "QAT08", "QAT", "Tarek Salman" },
    { MISSING, 0, "QAT09", "QAT", "Mohamed Al-Mannai" },
    { MISSING, 0, "QAT10", "QAT", "Karim Boudiaf" },
    { MISSING, 0, "QAT11", "QAT", "Assim Madibo" },
    { MISSING, 0, "QAT12", "QAT", "Ahmed Fatehi" },
    { MISSING, 0, "QAT13", "QAT", "Team Photo" },
    { MISSING, 0, "QAT14", "QAT", "Mohammed Waad" },
    { MISSING, 0, "QAT15", "QAT", "Abdulaziz Hatem" },
    { MISSING, 0, "QAT16", "QAT", "Hassan Al-Haydos" },
    { MISSING, 0, "QAT17", "QAT", "Edmilson Junior" },
    { MISSING, 0, "QAT18", "QAT", "Akram Hassan Afif" },
    { MISSING, 0, "QAT19", "QAT", "Ahmed Al Ganehi" },
    { MISSING, 0, "QAT20", "QAT", "Almoez Ali" },
    { MISSING, 0, "SUI01", "SUI", "Emblem" },
    { MISSING, 0, "SUI02", "SUI", "Gregor Kobel" },
    { MISSING, 0, "SUI03", "SUI", "Yvon Mvogo" },
    { MISSING, 0, "SUI04", "SUI", "Manuel Akanji" },
    { MISSING, 0, "SUI05", "SUI", "Ricardo Rodriguez" },
    { MISSING, 0, "SUI06", "SUI", "Nico Elvedi" },
    { MISSING, 0, "SUI07", "SUI", "Aurèle Amenda" },
    { MISSING, 0, "SUI08", "SUI", "Silvan Widmer" },
    { MISSING, 0, "SUI09", "SUI", "Granit Xhaka" },
    { MISSING, 0, "SUI10", "SUI", "Denis Zakaria" },
    { MISSING, 0, "SUI11", "SUI", "Remo Freuler" },
    { MISSING, 0, "SUI12", "SUI", "Fabian Rieder" },
    { MISSING, 0, "SUI13", "SUI", "Team Photo" },
    { MISSING, 0, "SUI14", "SUI", "Ardon Jashari" },
    { MISSING, 0, "SUI15", "SUI", "Johan Manzambi" },
    { MISSING, 0, "SUI16", "SUI", "Michel Aebischer" },
    { MISSING, 0, "SUI17", "SUI", "Breel Embolo" },
    { MISSING, 0, "SUI18", "SUI", "Ruben Vargas" },
    { MISSING, 0, "SUI19", "SUI", "Dan Ndoye" },
    { MISSING, 0, "SUI20", "SUI", "Zeki Amdouni" },
    { MISSING, 0, "BRA01", "BRA", "Emblem" },
    { MISSING, 0, "BRA02", "BRA", "Alisson" },
    { MISSING, 0, "BRA03", "BRA", "Bento" },
    { MISSING, 0, "BRA04", "BRA", "Marquinhos" },
    { MISSING, 0, "BRA05", "BRA", "Éder Militão" },
    { MISSING, 0, "BRA06", "BRA", "Gabriel Magalhães" },
    { MISSING, 0, "BRA07", "BRA", "Danilo" },
    { MISSING, 0, "BRA08", "BRA", "Wesley" },
    { MISSING, 0, "BRA09", "BRA", "Lucas Paquetá" },
    { MISSING, 0, "BRA10", "BRA", "Casemiro" },
    { MISSING, 0, "BRA11", "BRA", "Bruno Guimarães" },
    { MISSING, 0, "BRA12", "BRA", "Luiz Henrique" },
    { MISSING, 0, "BRA13", "BRA", "Team Photo" },
    { MISSING, 0, "BRA14", "BRA", "Vinicius Júnior" },
    { MISSING, 0, "BRA15", "BRA", "Rodrygo" },
    { MISSING, 0, "BRA16", "BRA", "João Pedro" },
    { MISSING, 0, "BRA17", "BRA", "Matheus Cunha" },
    { MISSING, 0, "BRA18", "BRA", "Gabriel Martinelli" },
    { MISSING, 0, "BRA19", "BRA", "Raphinha" },
    { MISSING, 0, "BRA20", "BRA", "Estévão" },
    { MISSING, 0, "MAR01", "MAR", "Emblem" },
    { MISSING, 0, "MAR02", "MAR", "Yassine Bounou" },
    { MISSING, 0, "MAR03", "MAR", "Munir El Kajoui" },
    { MISSING, 0, "MAR04", "MAR", "Achraf Hakimi" },
    { MISSING, 0, "MAR05", "MAR", "Noussair Mazraoui" },
    { MISSING, 0, "MAR06", "MAR", "Nayef Aguerd" },
    { MISSING, 0, "MAR07", "MAR", "Roman Saiss" },
    { MISSING, 0, "MAR08", "MAR", "Jawad El Yamio" },
    { MISSING, 0, "MAR09", "MAR", "Adam Masina" },
    { MISSING, 0, "MAR10", "MAR", "Sofyan Amrabat" },
    { MISSING, 0, "MAR11", "MAR", "Azzedine Ounahi" },
    { MISSING, 0, "MAR12", "MAR", "Eliesse Ben Seghir" },
    { MISSING, 0, "MAR13", "MAR", "Team Photo" },
    { MISSING, 0, "MAR14", "MAR", "Bilal El Khannouss" },
    { MISSING, 0, "MAR15", "MAR", "Ismael Saibari" },
    { MISSING, 0, "MAR16", "MAR", "Youssef En-Nesyri" },
    { MISSING, 0, "MAR17", "MAR", "Abde Ezzalzouli" },
    { MISSING, 0, "MAR18", "MAR", "Soufiane Rahimi" },
    { MISSING, 0, "MAR19", "MAR", "Brahim Diaz" },
    { MISSING, 0, "MAR20", "MAR", "Ayoub El Kaabi" },
    { MISSING, 0, "HAI01", "HAI", "Emblem" },
    { MISSING, 0, "HAI02", "HAI", "Johny Placide" },
    { MISSING, 0, "HAI03", "HAI", "Carlens Arcus" },
    { MISSING, 0, "HAI04", "HAI", "Martin Expérience" },
    { MISSING, 0, "HAI05", "HAI", "Jean-Kevin Duverne" },
    { MISSING, 0, "HAI06", "HAI", "Ricardo Adé" },
    { MISSING, 0, "HAI07", "HAI", "Duke Lacroix" },
    { MISSING, 0, "HAI08", "HAI", "Garven Metusala" },
    { MISSING, 0, "HAI09", "HAI", "Hannes Delcroix" },
    { MISSING, 0, "HAI10", "HAI", "Leverton Pierre" },
    { MISSING, 0, "HAI11", "HAI", "Danley Jean Jacques" },
    { MISSING, 0, "HAI12", "HAI", "Jean-Ricner Bellegarde" },
    { MISSING, 0, "HAI13", "HAI", "Team Photo" },
    { MISSING, 0, "HAI14", "HAI", "Christopher Attys" },
    { MISSING, 0, "HAI15", "HAI", "Derrick Etienne Jr" },
    { MISSING, 0, "HAI16", "HAI", "Josue Casimir" },
    { MISSING, 0, "HAI17", "HAI", "Ruben Providence" },
    { MISSING, 0, "HAI18", "HAI", "Duckens Nazon" },
    { MISSING, 0, "HAI19", "HAI", "Louicius Deedson" },
    { MISSING, 0, "HAI20", "HAI", "Frantzdy Pierrot" },
    { MISSING, 0, "SCO01", "SCO", "Emblem" },
    { MISSING, 0, "SCO02", "SCO", "Angus Gunn" },
    { MISSING, 0, "SCO03", "SCO", "Jack Hendry" },
    { MISSING, 0, "SCO04", "SCO", "Kieran Tierney" },
    { MISSING, 0, "SCO05", "SCO", "Aaron Hickey" },
    { MISSING, 0, "SCO06", "SCO", "Andrew Robertson" },
    { MISSING, 0, "SCO07", "SCO", "Scott McKenna" },
    { MISSING, 0, "SCO08", "SCO", "John Souttar" },
    { MISSING, 0, "SCO09", "SCO", "Anthony Ralston" },
    { MISSING, 0, "SCO10", "SCO", "Grant Hanley" },
    { MISSING, 0, "SCO11", "SCO", "Scott McTominay" },
    { MISSING, 0, "SCO12", "SCO", "Billy Gilmour" },
    { MISSING, 0, "SCO13", "SCO", "Team Photo" },
    { MISSING, 0, "SCO14", "SCO", "Lewis Ferguson" },
    { MISSING, 0, "SCO15", "SCO", "Ryan Christie" },
    { MISSING, 0, "SCO16", "SCO", "Kenny McLean" },
    { MISSING, 0, "SCO17", "SCO", "John McGinn" },
    { MISSING, 0, "SCO18", "SCO", "Lyndon Dykes" },
    { MISSING, 0, "SCO19", "SCO", "Che Adams" },
    { MISSING, 0, "SCO20", "SCO", "Ben Gannon-Doak" },
    { MISSING, 0, "USA01", "USA", "Emblem" },
    { MISSING, 0, "USA02", "USA", "Math Freese" },
    { MISSING, 0, "USA03", "USA", "Chris Richards" },
    { MISSING, 0, "USA04", "USA", "Tim Ream" },
    { MISSING, 0, "USA05", "USA", "Mark McKenzie" },
    { MISSING, 0, "USA06", "USA", "Alex Freeman" },
    { MISSING, 0, "USA07", "USA", "Antonee Robinson" },
    { MISSING, 0, "USA08", "USA", "Tyler Adams" },
    { MISSING, 0, "USA09", "USA", "Tanner Tessmann" },
    { MISSING, 0, "USA10", "USA", "Weston McKenny" },
    { MISSING, 0, "USA11", "USA", "Christian Roldan" },
    { MISSING, 0, "USA12", "USA", "Timothy Weah" },
    { MISSING, 0, "USA13", "USA", "Team Photo" },
    { MISSING, 0, "USA14", "USA", "Diego Luna" },
    { MISSING, 0, "USA15", "USA", "Malik Tillman" },
    { MISSING, 0, "USA16", "USA", "Christian Pulisic" },
    { MISSING, 0, "USA17", "USA", "Brenden Aaronson" },
    { MISSING, 0, "USA18", "USA", "Ricardo Pepi" },
    { MISSING, 0, "USA19", "USA", "Haji Wright" },
    { MISSING, 0, "USA20", "USA", "Folarin Balogun" },
    { MISSING, 0, "PAR01", "PAR", "Emblem" },
    { MISSING, 0, "PAR02", "PAR", "Roberto Fernandez" },
    { MISSING, 0, "PAR03", "PAR", "Orlando Gill" },
    { MISSING, 0, "PAR04", "PAR", "Gustavo Gomez" },
    { MISSING, 0, "PAR05", "PAR", "Fabián Balbuena" },
    { MISSING, 0, "PAR06", "PAR", "Juan José Cáceres" },
    { MISSING, 0, "PAR07", "PAR", "Omar Alderete" },
    { MISSING, 0, "PAR08", "PAR", "Junior Alonso" },
    { MISSING, 0, "PAR09", "PAR", "Mathías Villasanti" },
    { MISSING, 0, "PAR10", "PAR", "Diego Gomez" },
    { MISSING, 0, "PAR11", "PAR", "Damián Bobadilla" },
    { MISSING, 0, "PAR12", "PAR", "Andres Cubas" },
    { MISSING, 0, "PAR13", "PAR", "Team Photo" },
    { MISSING, 0, "PAR14", "PAR", "Matias Galarza Fonda" },
    { MISSING, 0, "PAR15", "PAR", "Julio Enciso" },
    { MISSING, 0, "PAR16", "PAR", "Alejandro Romero Gamarra" },
    { MISSING, 0, "PAR17", "PAR", "Miguel Almirón" },
    { MISSING, 0, "PAR18", "PAR", "Ramon Sosa" },
    { MISSING, 0, "PAR19", "PAR", "Angel Romero" },
    { MISSING, 0, "PAR20", "PAR", "Antonio Sanabria" },
    { MISSING, 0, "AUS01", "AUS", "Emblem" },
    { MISSING, 0, "AUS02", "AUS", "Mathew Ryan" },
    { MISSING, 0, "AUS03", "AUS", "Joe Gauci" },
    { MISSING, 0, "AUS04", "AUS", "Harry Souttar" },
    { MISSING, 0, "AUS05", "AUS", "Alessandro Circati" },
    { MISSING, 0, "AUS06", "AUS", "Jordan Bos" },
    { MISSING, 0, "AUS07", "AUS", "Aziz Behich" },
    { MISSING, 0, "AUS08", "AUS", "Cameron Burgess" },
    { MISSING, 0, "AUS09", "AUS", "Lewis Miller" },
    { MISSING, 0, "AUS10", "AUS", "Milos Degenek" },
    { MISSING, 0, "AUS11", "AUS", "Jackson Irvine" },
    { MISSING, 0, "AUS12", "AUS", "Riley McGree" },
    { MISSING, 0, "AUS13", "AUS", "Team Photo" },
    { MISSING, 0, "AUS14", "AUS", "Aiden O'Neill" },
    { MISSING, 0, "AUS15", "AUS", "Connor Metcalfe" },
    { MISSING, 0, "AUS16", "AUS", "Patrick Yazbek" },
    { MISSING, 0, "AUS17", "AUS", "Craig Goodwin" },
    { MISSING, 0, "AUS18", "AUS", "Kusini Vengi" },
    { MISSING, 0, "AUS19", "AUS", "Nestory Irankunda" },
    { MISSING, 0, "AUS20", "AUS", "Mohamed Touré" },
    { MISSING, 0, "TUR01", "TUR", "Emblem" },
    { MISSING, 0, "TUR02", "TUR", "Ugurcan Cakir" },
    { MISSING, 0, "TUR03", "TUR", "Mert Muldur" },
    { MISSING, 0, "TUR04", "TUR", "Zeki Celik" },
    { MISSING, 0, "TUR05", "TUR", "Abdulkerim Bardakci" },
    { MISSING, 0, "TUR06", "TUR", "Caglar Soyuncu" },
    { MISSING, 0, "TUR07", "TUR", "Merih Demiral" },
    { MISSING, 0, "TUR08", "TUR", "Ferdi Kadioglu" },
    { MISSING, 0, "TUR09", "TUR", "Kaan Ayhan" },
    { MISSING, 0, "TUR10", "TUR", "Ismail Yuksek" },
    { MISSING, 0, "TUR11", "TUR", "Hakan Calhanoglu" },
    { MISSING, 0, "TUR12", "TUR", "Orkun Kokcu" },
    { MISSING, 0, "TUR13", "TUR", "Team Photo" },
    { MISSING, 0, "TUR14", "TUR", "Arda Guler" },
    { MISSING, 0, "TUR15", "TUR", "Irfan Can Kahveci" },
    { MISSING, 0, "TUR16", "TUR", "Yunus Akgun" },
    { MISSING, 0, "TUR17", "TUR", "Can Uzun" },
    { MISSING, 0, "TUR18", "TUR", "Baris Alper Yilmaz" },
    { MISSING, 0, "TUR19", "TUR", "Kerem Akturkoglu" },
    { MISSING, 0, "TUR20", "TUR", "Kenan Yildiz" },
    { MISSING, 0, "GER01", "GER", "Emblem" },
    { MISSING, 0, "GER02", "GER", "Marc-André ter Stegen" },
    { MISSING, 0, "GER03", "GER", "Jonathan Tah" },
    { MISSING, 0, "GER04", "GER", "David Raum" },
    { MISSING, 0, "GER05", "GER", "Nico Schlotterbeck" },
    { MISSING, 0, "GER06", "GER", "Antonio Rüdiger" },
    { MISSING, 0, "GER07", "GER", "Waldemar Anton" },
    { MISSING, 0, "GER08", "GER", "Ridle Baku" },
    { MISSING, 0, "GER09", "GER", "Maximilian Mittelstadt" },
    { MISSING, 0, "GER10", "GER", "Joshua Kimmich" },
    { MISSING, 0, "GER11", "GER", "Florian Wirtz" },
    { MISSING, 0, "GER12", "GER", "Felix Nmecha" },
    { MISSING, 0, "GER13", "GER", "Team Photo" },
    { MISSING, 0, "GER14", "GER", "Leon Goretzka" },
    { MISSING, 0, "GER15", "GER", "Jamal Musiala" },
    { MISSING, 0, "GER16", "GER", "Serge Gnabry" },
    { MISSING, 0, "GER17", "GER", "Kai Havertz" },
    { MISSING, 0, "GER18", "GER", "Leroy Sane" },
    { MISSING, 0, "GER19", "GER", "Karim Adeyemi" },
    { MISSING, 0, "GER20", "GER", "Nick Woltemade" },
    { MISSING, 0, "CUW01", "CUW", "Emblem" },
    { MISSING, 0, "CUW02", "CUW", "Eloy Room" },
    { MISSING, 0, "CUW03", "CUW", "Armando Obispo" },
    { MISSING, 0, "CUW04", "CUW", "Sherel Floranus" },
    { MISSING, 0, "CUW05", "CUW", "Jurien Gaari" },
    { MISSING, 0, "CUW06", "CUW", "Joshua Brenet" },
    { MISSING, 0, "CUW07", "CUW", "Roshon Van Eijma" },
    { MISSING, 0, "CUW08", "CUW", "Shurandy Sambo" },
    { MISSING, 0, "CUW09", "CUW", "Livano Comenencia" },
    { MISSING, 0, "CUW10", "CUW", "Godfried Roemeratoe" },
    { MISSING, 0, "CUW11", "CUW", "Juninho Bacuna" },
    { MISSING, 0, "CUW12", "CUW", "Leandro Bacuna" },
    { MISSING, 0, "CUW13", "CUW", "Team Photo" },
    { MISSING, 0, "CUW14", "CUW", "Tahith Chong" },
    { MISSING, 0, "CUW15", "CUW", "Kenji Gorre" },
    { MISSING, 0, "CUW16", "CUW", "Jearl Margaritha" },
    { MISSING, 0, "CUW17", "CUW", "Jurgen Locadia" },
    { MISSING, 0, "CUW18", "CUW", "Jeremy Antonisse" },
    { MISSING, 0, "CUW19", "CUW", "Gervane Kastaneer" },
    { MISSING, 0, "CUW20", "CUW", "Sontje Hansen" },
    { MISSING, 0, "CIV01", "CIV", "Emblem" },
    { MISSING, 0, "CIV02", "CIV", "Yahia Fofana" },
    { MISSING, 0, "CIV03", "CIV", "Ghislain Konan" },
    { MISSING, 0, "CIV04", "CIV", "Wilfried Singo" },
    { MISSING, 0, "CIV05", "CIV", "Odilon Kossounou" },
    { MISSING, 0, "CIV06", "CIV", "Evan Ndicka" },
    { MISSING, 0, "CIV07", "CIV", "Willy Boly" },
    { MISSING, 0, "CIV08", "CIV", "Emmanuel Agbadou" },
    { MISSING, 0, "CIV09", "CIV", "Ousmane Diomande" },
    { MISSING, 0, "CIV10", "CIV", "Franck Kessie" },
    { MISSING, 0, "CIV11", "CIV", "Seko Fofana" },
    { MISSING, 0, "CIV12", "CIV", "Ibrahim Sangare" },
    { MISSING, 0, "CIV13", "CIV", "Team Photo" },
    { MISSING, 0, "CIV14", "CIV", "Jean-Philippe Gbamin" },
    { MISSING, 0, "CIV15", "CIV", "Amad Diallo" },
    { MISSING, 0, "CIV16", "CIV", "Sébastien Haller" },
    { MISSING, 0, "CIV17", "CIV", "Simon Adingra" },
    { MISSING, 0, "CIV18", "CIV", "Yan Diomande" },
    { MISSING, 0, "CIV19", "CIV", "Evann Guessand" },
    { MISSING, 0, "CIV20", "CIV", "Oumar Diakite" },
    { MISSING, 0, "ECU01", "ECU", "Emblem" },
    { MISSING, 0, "ECU02", "ECU", "Hernán Galíndez" },
    { MISSING, 0, "ECU03", "ECU", "Gonzalo Valle" },
    { MISSING, 0, "ECU04", "ECU", "Piero Hincapié" },
    { MISSING, 0, "ECU05", "ECU", "Pervis Estupiñán" },
    { MISSING, 0, "ECU06", "ECU", "Willian Pacho" },
    { MISSING, 0, "ECU07", "ECU", "Ángelo Preciado" },
    { MISSING, 0, "ECU08", "ECU", "Joel Ordóñez" },
    { MISSING, 0, "ECU09", "ECU", "Moises Caicedo" },
    { MISSING, 0, "ECU10", "ECU", "Alan Franco" },
    { MISSING, 0, "ECU11", "ECU", "Kendry Paez" },
    { MISSING, 0, "ECU12", "ECU", "Pedro Vite" },
    { MISSING, 0, "ECU13", "ECU", "Team Photo" },
    { MISSING, 0, "ECU14", "ECU", "John Veboah" },
    { MISSING, 0, "ECU15", "ECU", "Leonardo Campana" },
    { MISSING, 0, "ECU16", "ECU", "Gonzalo Plata" },
    { MISSING, 0, "ECU17", "ECU", "Nilson Angulo" },
    { MISSING, 0, "ECU18", "ECU", "Alan Minda" },
    { MISSING, 0, "ECU19", "ECU", "Kevin Rodriguez" },
    { MISSING, 0, "ECU20", "ECU", "Enner Valencia" },
    { MISSING, 0, "NED01", "NED", "Emblem" },
    { MISSING, 0, "NED02", "NED", "Bart Verbruggen" },
    { MISSING, 0, "NED03", "NED", "Virgil van Dijk" },
    { MISSING, 0, "NED04", "NED", "Micky van de Ven" },
    { MISSING, 0, "NED05", "NED", "Jurrien Timber" },
    { MISSING, 0, "NED06", "NED", "Denzel Dumfries" },
    { MISSING, 0, "NED07", "NED", "Nathan Aké" },
    { MISSING, 0, "NED08", "NED", "Jeremie Frimpong" },
    { MISSING, 0, "NED09", "NED", "Jan Paul van Hecke" },
    { MISSING, 0, "NED10", "NED", "Tijjani Reijnders" },
    { MISSING, 0, "NED11", "NED", "Ryan Gravenberch" },
    { MISSING, 0, "NED12", "NED", "Teun Koopmeiners" },
    { MISSING, 0, "NED13", "NED", "Team Photo" },
    { MISSING, 0, "NED14", "NED", "Frenkie de Jong" },
    { MISSING, 0, "NED15", "NED", "Xavi Simons" },
    { MISSING, 0, "NED16", "NED", "Justin Kluivert" },
    { MISSING, 0, "NED17", "NED", "Memphis Depay" },
    { MISSING, 0, "NED18", "NED", "Donyell Malen" },
    { MISSING, 0, "NED19", "NED", "Wout Weghorst" },
    { MISSING, 0, "NED20", "NED", "Cody Gakpo" },
    { MISSING, 0, "JPN01", "JPN", "Emblem" },
    { MISSING, 0, "JPN02", "JPN", "Zion Suzuki" },
    { MISSING, 0, "JPN03", "JPN", "Henry Heroki Mochizuki" },
    { MISSING, 0, "JPN04", "JPN", "Ayumu Seko" },
    { MISSING, 0, "JPN05", "JPN", "Junnosuke Suzuki" },
    { MISSING, 0, "JPN06", "JPN", "Shogo Taniguchi" },
    { MISSING, 0, "JPN07", "JPN", "Tsuyoshi Watanabe" },
    { MISSING, 0, "JPN08", "JPN", "Kaishu Sano" },
    { MISSING, 0, "JPN09", "JPN", "Yuki Soma" },
    { MISSING, 0, "JPN10", "JPN", "Ao Tanaka" },
    { MISSING, 0, "JPN11", "JPN", "Daichi Kamada" },
    { MISSING, 0, "JPN12", "JPN", "Takefusa Kubo" },
    { MISSING, 0, "JPN13", "JPN", "Team Photo" },
    { MISSING, 0, "JPN14", "JPN", "Ritsu Doan" },
    { MISSING, 0, "JPN15", "JPN", "Keito Nakamura" },
    { MISSING, 0, "JPN16", "JPN", "Takumi Minamino" },
    { MISSING, 0, "JPN17", "JPN", "Shuto Machino" },
    { MISSING, 0, "JPN18", "JPN", "Junya Ito" },
    { MISSING, 0, "JPN19", "JPN", "Koki Ogawa" },
    { MISSING, 0, "JPN20", "JPN", "Ayase Ueda" },
    { MISSING, 0, "SWE01", "SWE", "Emblem" },
    { MISSING, 0, "SWE02", "SWE", "Victor Johansson" },
    { MISSING, 0, "SWE03", "SWE", "Isak Hien" },
    { MISSING, 0, "SWE04", "SWE", "Gabriel Gudmundsson" },
    { MISSING, 0, "SWE05", "SWE", "Emil Holm" },
    { MISSING, 0, "SWE06", "SWE", "Victor Nilsson Lindelöf" },
    { MISSING, 0, "SWE07", "SWE", "Gustaf Lagerbielke" },
    { MISSING, 0, "SWE08", "SWE", "Lucas Bergvall" },
    { MISSING, 0, "SWE09", "SWE", "Hugo Larsson" },
    { MISSING, 0, "SWE10", "SWE", "Jesper Karlström" },
    { MISSING, 0, "SWE11", "SWE", "Yasin Ayari" },
    { MISSING, 0, "SWE12", "SWE", "Mattias Svanberg" },
    { MISSING, 0, "SWE13", "SWE", "Team Photo" },
    { MISSING, 0, "SWE14", "SWE", "Daniel Svensson" },
    { MISSING, 0, "SWE15", "SWE", "Ken Sema" },
    { MISSING, 0, "SWE16", "SWE", "Roony Bardghji" },
    { MISSING, 0, "SWE17", "SWE", "Dejan Kulusevski" },
    { MISSING, 0, "SWE18", "SWE", "Anthony Elanga" },
    { MISSING, 0, "SWE19", "SWE", "Alexander Isak" },
    { MISSING, 0, "SWE20", "SWE", "Viktor Gyökeres" },
    { MISSING, 0, "TUN01", "TUN", "Emblem" },
    { MISSING, 0, "TUN02", "TUN", "Bechir Ben Said" },
    { MISSING, 0, "TUN03", "TUN", "Aymen Dahmen" },
    { MISSING, 0, "TUN04", "TUN", "Yan Valery" },
    { MISSING, 0, "TUN05", "TUN", "Montassar Talbi" },
    { MISSING, 0, "TUN06", "TUN", "Yassine Meriah" },
    { MISSING, 0, "TUN07", "TUN", "Ali Abdi" },
    { MISSING, 0, "TUN08", "TUN", "Dylan Bronn" },
    { MISSING, 0, "TUN09", "TUN", "Ellyes Skhiri" },
    { MISSING, 0, "TUN10", "TUN", "Aissa Laidouni" },
    { MISSING, 0, "TUN11", "TUN", "Ferjani Sassi" },
    { MISSING, 0, "TUN12", "TUN", "Mohamed Ali Ben Romdhane" },
    { MISSING, 0, "TUN13", "TUN", "Team Photo" },
    { MISSING, 0, "TUN14", "TUN", "Hannibal Mejbri" },
    { MISSING, 0, "TUN15", "TUN", "Elias Achouri" },
    { MISSING, 0, "TUN16", "TUN", "Elias Saad" },
    { MISSING, 0, "TUN17", "TUN", "Hazem Mastouri" },
    { MISSING, 0, "TUN18", "TUN", "Ismael Gharbi" },
    { MISSING, 0, "TUN19", "TUN", "Sayfallah Ltaief" },
    { MISSING, 0, "TUN20", "TUN", "Naim Sliti" },
    { MISSING, 0, "BEL01", "BEL", "Emblem" },
    { MISSING, 0, "BEL02", "BEL", "Thibaut Courtois" },
    { MISSING, 0, "BEL03", "BEL", "Arthur Theate" },
    { MISSING, 0, "BEL04", "BEL", "Timothy Castagne" },
    { MISSING, 0, "BEL05", "BEL", "Zeno Debast" },
    { MISSING, 0, "BEL06", "BEL", "Brandon Mechele" },
    { MISSING, 0, "BEL07", "BEL", "Maxim De Cuyper" },
    { MISSING, 0, "BEL08", "BEL", "Thomas Meunier" },
    { MISSING, 0, "BEL09", "BEL", "Youri Tielemans" },
    { MISSING, 0, "BEL10", "BEL", "Amadou Onana" },
    { MISSING, 0, "BEL11", "BEL", "Nicolas Raskin" },
    { MISSING, 0, "BEL12", "BEL", "Alexis Saelemaekers" },
    { MISSING, 0, "BEL13", "BEL", "Team Photo" },
    { MISSING, 0, "BEL14", "BEL", "Hans Vanaken" },
    { MISSING, 0, "BEL15", "BEL", "Kevin De Bruyne" },
    { MISSING, 0, "BEL16", "BEL", "Jérémy Doku" },
    { MISSING, 0, "BEL17", "BEL", "Charles De Ketelaere" },
    { MISSING, 0, "BEL18", "BEL", "Leandro Trossard" },
    { MISSING, 0, "BEL19", "BEL", "Loïs Openda" },
    { MISSING, 0, "BEL20", "BEL", "Romelu Lukaku" },
    { MISSING, 0, "EGY01", "EGY", "Emblem" },
    { MISSING, 0, "EGY02", "EGY", "Mohamed El Shenawy" },
    { MISSING, 0, "EGY03", "EGY", "Mohamed Hany" },
    { MISSING, 0, "EGY04", "EGY", "Mohamed Hamdy" },
    { MISSING, 0, "EGY05", "EGY", "Yasser Ibrahim" },
    { MISSING, 0, "EGY06", "EGY", "Khaled Sobhi" },
    { MISSING, 0, "EGY07", "EGY", "Ramy Rabia" },
    { MISSING, 0, "EGY08", "EGY", "Hossam Abdelmaguid" },
    { MISSING, 0, "EGY09", "EGY", "Ahmed Fatouh" },
    { MISSING, 0, "EGY10", "EGY", "Marwan Attia" },
    { MISSING, 0, "EGY11", "EGY", "Zizo" },
    { MISSING, 0, "EGY12", "EGY", "Hamdy Fathy" },
    { MISSING, 0, "EGY13", "EGY", "Team Photo" },
    { MISSING, 0, "EGY14", "EGY", "Mohamed Lasheen" },
    { MISSING, 0, "EGY15", "EGY", "Emam Ashour" },
    { MISSING, 0, "EGY16", "EGY", "Osama Faisal" },
    { MISSING, 0, "EGY17", "EGY", "Mohamed Salah" },
    { MISSING, 0, "EGY18", "EGY", "Mostafa Mohamed" },
    { MISSING, 0, "EGY19", "EGY", "Trezeguet" },
    { MISSING, 0, "EGY20", "EGY", "Omar Marmoush" },
    { MISSING, 0, "IRN01", "IRN", "Emblem" },
    { MISSING, 0, "IRN02", "IRN", "Alireza Beiranvand" },
    { MISSING, 0, "IRN03", "IRN", "Morteza Pouraliganji" },
    { MISSING, 0, "IRN04", "IRN", "Ehsan Hajsafi" },
    { MISSING, 0, "IRN05", "IRN", "Milad Mohammadi" },
    { MISSING, 0, "IRN06", "IRN", "Shojae Khalilzadeh" },
    { MISSING, 0, "IRN07", "IRN", "Ramin Rezaeian" },
    { MISSING, 0, "IRN08", "IRN", "Hossein Kanaani" },
    { MISSING, 0, "IRN09", "IRN", "Sadegh Moharrami" },
    { MISSING, 0, "IRN10", "IRN", "Saleh Hardani" },
    { MISSING, 0, "IRN11", "IRN", "Saeed Ezatolahi" },
    { MISSING, 0, "IRN12", "IRN", "Saman Ghoddos" },
    { MISSING, 0, "IRN13", "IRN", "Team Photo" },
    { MISSING, 0, "IRN14", "IRN", "Omid Noorafkan" },
    { MISSING, 0, "IRN15", "IRN", "Roozbeh Cheshmi" },
    { MISSING, 0, "IRN16", "IRN", "Mohammad Mohebi" },
    { MISSING, 0, "IRN17", "IRN", "Sardar Azmoun" },
    { MISSING, 0, "IRN18", "IRN", "Mehdi Taremi" },
    { MISSING, 0, "IRN19", "IRN", "Alireza Jahanbakhsh" },
    { MISSING, 0, "IRN20", "IRN", "Ali Gholizadeh" },
    { MISSING, 0, "NZL01", "NZL", "Emblem" },
    { MISSING, 0, "NZL02", "NZL", "Max Crocombe Payne" },
    { MISSING, 0, "NZL03", "NZL", "Alex Paulsen" },
    { MISSING, 0, "NZL04", "NZL", "Michael Boxall" },
    { MISSING, 0, "NZL05", "NZL", "Liberato Cacace" },
    { MISSING, 0, "NZL06", "NZL", "Tim Payne" },
    { MISSING, 0, "NZL07", "NZL", "Tyler Bindon" },
    { MISSING, 0, "NZL08", "NZL", "Francis de Vries" },
    { MISSING, 0, "NZL09", "NZL", "Finn Surman" },
    { MISSING, 0, "NZL10", "NZL", "Joe Bell" },
    { MISSING, 0, "NZL11", "NZL", "Sarpreet Singh" },
    { MISSING, 0, "NZL12", "NZL", "Ryan Thomas" },
    { MISSING, 0, "NZL13", "NZL", "Team Photo" },
    { MISSING, 0, "NZL14", "NZL", "Matthew Garbett" },
    { MISSING, 0, "NZL15", "NZL", "Marko Stamenić" },
    { MISSING, 0, "NZL16", "NZL", "Ben Old" },
    { MISSING, 0, "NZL17", "NZL", "Chris Wood" },
    { MISSING, 0, "NZL18", "NZL", "Elijah Just" },
    { MISSING, 0, "NZL19", "NZL", "Callum McCowatt" },
    { MISSING, 0, "NZL20", "NZL", "Kosta Barbarouses" },
    { MISSING, 0, "ESP01", "ESP", "Emblem" },
    { MISSING, 0, "ESP02", "ESP", "Unai Simon" },
    { MISSING, 0, "ESP03", "ESP", "Robin Le Normand" },
    { MISSING, 0, "ESP04", "ESP", "Aymeric Laporte" },
    { MISSING, 0, "ESP05", "ESP", "Dean Huijsen" },
    { MISSING, 0, "ESP06", "ESP", "Pedro Porro" },
    { MISSING, 0, "ESP07", "ESP", "Dani Carvajal" },
    { MISSING, 0, "ESP08", "ESP", "Marc Cucurella" },
    { MISSING, 0, "ESP09", "ESP", "Martín Zubimendi" },
    { MISSING, 0, "ESP10", "ESP", "Rodri" },
    { MISSING, 0, "ESP11", "ESP", "Pedri" },
    { MISSING, 0, "ESP12", "ESP", "Fabian Ruiz" },
    { MISSING, 0, "ESP13", "ESP", "Team Photo" },
    { MISSING, 0, "ESP14", "ESP", "Mikel Merino" },
    { MISSING, 0, "ESP15", "ESP", "Lamine Yamal" },
    { MISSING, 0, "ESP16", "ESP", "Dani Olmo" },
    { MISSING, 0, "ESP17", "ESP", "Nico Williams" },
    { MISSING, 0, "ESP18", "ESP", "Ferran Torres" },
    { MISSING, 0, "ESP19", "ESP", "Álvaro Morata" },
    { MISSING, 0, "ESP20", "ESP", "Mikel Oyarzabal" },
    { MISSING, 0, "CPV01", "CPV", "Emblem" },
    { MISSING, 0, "CPV02", "CPV", "Vozinha" },
    { MISSING, 0, "CPV03", "CPV", "Logan Costa" },
    { MISSING, 0, "CPV04", "CPV", "Pico" },
    { MISSING, 0, "CPV05", "CPV", "Diney" },
    { MISSING, 0, "CPV06", "CPV", "Steven Moreira" },
    { MISSING, 0, "CPV07", "CPV", "Wagner Pina" },
    { MISSING, 0, "CPV08", "CPV", "Joao Paulo" },
    { MISSING, 0, "CPV09", "CPV", "Yannick Semedo" },
    { MISSING, 0, "CPV10", "CPV", "Kevin Pina" },
    { MISSING, 0, "CPV11", "CPV", "Patrick Andrade" },
    { MISSING, 0, "CPV12", "CPV", "Jamiro Monteiro" },
    { MISSING, 0, "CPV13", "CPV", "Team Photo" },
    { MISSING, 0, "CPV14", "CPV", "Deroy Duarte" },
    { MISSING, 0, "CPV15", "CPV", "Garry Rodrigues" },
    { MISSING, 0, "CPV16", "CPV", "Jovane Cabral" },
    { MISSING, 0, "CPV17", "CPV", "Ryan Mendes" },
    { MISSING, 0, "CPV18", "CPV", "Dailon Livramento" },
    { MISSING, 0, "CPV19", "CPV", "Willy Semedo" },
    { MISSING, 0, "CPV20", "CPV", "Bebe" },
    { MISSING, 0, "KSA01", "KSA", "Emblem" },
    { MISSING, 0, "KSA02", "KSA", "Nawaf Alaqidi" },
    { MISSING, 0, "KSA03", "KSA", "Abdulrahman Al-Sanbi" },
    { MISSING, 0, "KSA04", "KSA", "Saud Abdulhamid" },
    { MISSING, 0, "KSA05", "KSA", "Nawaf Bouwashl" },
    { MISSING, 0, "KSA06", "KSA", "Jihad Thakri" },
    { MISSING, 0, "KSA07", "KSA", "Moteb Al-Harbi" },
    { MISSING, 0, "KSA08", "KSA", "Hassan Altambakti" },
    { MISSING, 0, "KSA09", "KSA", "Musab Aljuwayr" },
    { MISSING, 0, "KSA10", "KSA", "Ziyad Aljohani" },
    { MISSING, 0, "KSA11", "KSA", "Abdullah Alkhaibari" },
    { MISSING, 0, "KSA12", "KSA", "Nasser Aldawsari" },
    { MISSING, 0, "KSA13", "KSA", "Team Photo" },
    { MISSING, 0, "KSA14", "KSA", "Saleh Abu Alshamat" },
    { MISSING, 0, "KSA15", "KSA", "Marwan Alsahafi" },
    { MISSING, 0, "KSA16", "KSA", "Salem Aldawsari" },
    { MISSING, 0, "KSA17", "KSA", "Abdulrahman Al-Aboud" },
    { MISSING, 0, "KSA18", "KSA", "Feras Akbrikan" },
    { MISSING, 0, "KSA19", "KSA", "Saleh Alshehri" },
    { MISSING, 0, "KSA20", "KSA", "Abdullah Al-Hamdan" },
    { MISSING, 0, "URU01", "URU", "Emblem" },
    { MISSING, 0, "URU02", "URU", "Sergio Rochet" },
    { MISSING, 0, "URU03", "URU", "Santiago Mele" },
    { MISSING, 0, "URU04", "URU", "Ronald Araujo" },
    { MISSING, 0, "URU05", "URU", "José María Giménez" },
    { MISSING, 0, "URU06", "URU", "Sebastian Caceres" },
    { MISSING, 0, "URU07", "URU", "Mathias Olivera" },
    { MISSING, 0, "URU08", "URU", "Guillermo Varela" },
    { MISSING, 0, "URU09", "URU", "Nahitan Nandez" },
    { MISSING, 0, "URU10", "URU", "Federico Valverde" },
    { MISSING, 0, "URU11", "URU", "Giorgian De Arrascaeta" },
    { MISSING, 0, "URU12", "URU", "Rodrigo Bentancur" },
    { MISSING, 0, "URU13", "URU", "Team Photo" },
    { MISSING, 0, "URU14", "URU", "Manuel Ugarte" },
    { MISSING, 0, "URU15", "URU", "Nicolás de la Cruz" },
    { MISSING, 0, "URU16", "URU", "Maxi Araujo" },
    { MISSING, 0, "URU17", "URU", "Darwin Núñez" },
    { MISSING, 0, "URU18", "URU", "Federico Viñas" },
    { MISSING, 0, "URU19", "URU", "Rodrigo Aguirre" },
    { MISSING, 0, "URU20", "URU", "Facundo Pellistri" },
    { MISSING, 0, "FRA01", "FRA", "Emblem" },
    { MISSING, 0, "FRA02", "FRA", "Mike Maignan" },
    { MISSING, 0, "FRA03", "FRA", "Theo Hernandez" },
    { MISSING, 0, "FRA04", "FRA", "William Saliba" },
    { MISSING, 0, "FRA05", "FRA", "Jules Kounde" },
    { MISSING, 0, "FRA06", "FRA", "Ibrahima Konate" },
    { MISSING, 0, "FRA07", "FRA", "Dayot Upamecano" },
    { MISSING, 0, "FRA08", "FRA", "Lucas Digne" },
    { MISSING, 0, "FRA09", "FRA", "Aurélien Tchouaméni" },
    { MISSING, 0, "FRA10", "FRA", "Eduardo Camavinga" },
    { MISSING, 0, "FRA11", "FRA", "Manu Kone" },
    { MISSING, 0, "FRA12", "FRA", "Adrien Rabiot" },
    { MISSING, 0, "FRA13", "FRA", "Team Photo" },
    { MISSING, 0, "FRA14", "FRA", "Michael Olise" },
    { MISSING, 0, "FRA15", "FRA", "Ousmane Dembele" },
    { MISSING, 0, "FRA16", "FRA", "Bradley Barcola" },
    { MISSING, 0, "FRA17", "FRA", "Désiré Doué" },
    { MISSING, 0, "FRA18", "FRA", "Kingsley Coman" },
    { MISSING, 0, "FRA19", "FRA", "Hugo Ekitike" },
    { MISSING, 0, "FRA20", "FRA", "Kylian Mbappe" },
    { MISSING, 0, "SEN01", "SEN", "Emblem" },
    { MISSING, 0, "SEN02", "SEN", "Edouard Mendy" },
    { MISSING, 0, "SEN03", "SEN", "Yehvann Diouf" },
    { MISSING, 0, "SEN04", "SEN", "Moussa Niakhaté" },
    { MISSING, 0, "SEN05", "SEN", "Abdoulaye Seck" },
    { MISSING, 0, "SEN06", "SEN", "Ismail Jakobs" },
    { MISSING, 0, "SEN07", "SEN", "El Hadji Malick Diouf" },
    { MISSING, 0, "SEN08", "SEN", "Kalidou Koulibaly" },
    { MISSING, 0, "SEN09", "SEN", "Idrissa Gana Gueye" },
    { MISSING, 0, "SEN10", "SEN", "Pape Matar Sarr" },
    { MISSING, 0, "SEN11", "SEN", "Pape Gueye" },
    { MISSING, 0, "SEN12", "SEN", "Habib Diarra" },
    { MISSING, 0, "SEN13", "SEN", "Team Photo" },
    { MISSING, 0, "SEN14", "SEN", "Lamine Camara" },
    { MISSING, 0, "SEN15", "SEN", "Sadio Mane" },
    { MISSING, 0, "SEN16", "SEN", "Ismaïla Sarr" },
    { MISSING, 0, "SEN17", "SEN", "Boulaye Dia" },
    { MISSING, 0, "SEN18", "SEN", "Iliman Ndiaye" },
    { MISSING, 0, "SEN19", "SEN", "Nicolas Jackson" },
    { MISSING, 0, "SEN20", "SEN", "Krepin Diatta" },
    { MISSING, 0, "IRQ01", "IRQ", "Emblem" },
    { MISSING, 0, "IRQ02", "IRQ", "Jalal Hassan" },
    { MISSING, 0, "IRQ03", "IRQ", "Rebin Sulaka" },
    { MISSING, 0, "IRQ04", "IRQ", "Hussein Ali" },
    { MISSING, 0, "IRQ05", "IRQ", "Akam Hashem" },
    { MISSING, 0, "IRQ06", "IRQ", "Merchas Doski" },
    { MISSING, 0, "IRQ07", "IRQ", "Zaid Tahseen" },
    { MISSING, 0, "IRQ08", "IRQ", "Manaf Younis" },
    { MISSING, 0, "IRQ09", "IRQ", "Zidane Iqbal" },
    { MISSING, 0, "IRQ10", "IRQ", "Amir Al-Ammari" },
    { MISSING, 0, "IRQ11", "IRQ", "Ibrahim Bavesh" },
    { MISSING, 0, "IRQ12", "IRQ", "Ali Jasim" },
    { MISSING, 0, "IRQ13", "IRQ", "Team Photo" },
    { MISSING, 0, "IRQ14", "IRQ", "Youssef Amyn" },
    { MISSING, 0, "IRQ15", "IRQ", "Aimar Sher" },
    { MISSING, 0, "IRQ16", "IRQ", "Marko Farji" },
    { MISSING, 0, "IRQ17", "IRQ", "Osama Rashid" },
    { MISSING, 0, "IRQ18", "IRQ", "Ali Al-Hamadi" },
    { MISSING, 0, "IRQ19", "IRQ", "Aymen Hussein" },
    { MISSING, 0, "IRQ20", "IRQ", "Mohanad Ali" },
    { MISSING, 0, "NOR01", "NOR", "Emblem" },
    { MISSING, 0, "NOR02", "NOR", "Orjan Nyland" },
    { MISSING, 0, "NOR03", "NOR", "Julian Ryerson" },
    { MISSING, 0, "NOR04", "NOR", "Leo Ostigård" },
    { MISSING, 0, "NOR05", "NOR", "Kristoffer Vassbakk Ajer" },
    { MISSING, 0, "NOR06", "NOR", "Marcus Holmgren Pedersen" },
    { MISSING, 0, "NOR07", "NOR", "David Møller Wolfe" },
    { MISSING, 0, "NOR08", "NOR", "Torbjørn Heggem" },
    { MISSING, 0, "NOR09", "NOR", "Morten Thorsby" },
    { MISSING, 0, "NOR10", "NOR", "Martin Ødegaard" },
    { MISSING, 0, "NOR11", "NOR", "Sander Berge" },
    { MISSING, 0, "NOR12", "NOR", "Andreas Schjelderup" },
    { MISSING, 0, "NOR13", "NOR", "Team Photo" },
    { MISSING, 0, "NOR14", "NOR", "Patrick Berg" },
    { MISSING, 0, "NOR15", "NOR", "Erling Haaland" },
    { MISSING, 0, "NOR16", "NOR", "Alexander Sørloth" },
    { MISSING, 0, "NOR17", "NOR", "Aron Dønnum" },
    { MISSING, 0, "NOR18", "NOR", "Jorgen Strand Larsen" },
    { MISSING, 0, "NOR19", "NOR", "Antonio Nusa" },
    { MISSING, 0, "NOR20", "NOR", "Oscar Bobb" },
    { MISSING, 0, "ARG01", "ARG", "Emblem" },
    { MISSING, 0, "ARG02", "ARG", "Emiliano Martinez" },
    { MISSING, 0, "ARG03", "ARG", "Nahuel Molina" },
    { MISSING, 0, "ARG04", "ARG", "Cristian Romero" },
    { MISSING, 0, "ARG05", "ARG", "Nicolas Otamendi" },
    { MISSING, 0, "ARG06", "ARG", "Nicolas Tagliafico" },
    { MISSING, 0, "ARG07", "ARG", "Leonardo Balerdi" },
    { MISSING, 0, "ARG08", "ARG", "Enzo Fernandez" },
    { MISSING, 0, "ARG09", "ARG", "Alexis Mac Allister" },
    { MISSING, 0, "ARG10", "ARG", "Rodrigo De Paul" },
    { MISSING, 0, "ARG11", "ARG", "Exequiel Palacios" },
    { MISSING, 0, "ARG12", "ARG", "Leandro Paredes" },
    { MISSING, 0, "ARG13", "ARG", "Team Photo" },
    { MISSING, 0, "ARG14", "ARG", "Nico Paz" },
    { MISSING, 0, "ARG15", "ARG", "Franco Mastantuono" },
    { MISSING, 0, "ARG16", "ARG", "Nico Gonzalez" },
    { MISSING, 0, "ARG17", "ARG", "Lionel Messi" },
    { MISSING, 0, "ARG18", "ARG", "Lautaro Martinez" },
    { MISSING, 0, "ARG19", "ARG", "Julian Alvarez" },
    { MISSING, 0, "ARG20", "ARG", "Giuliano Simeone" },
    { MISSING, 0, "ALG01", "ALG", "Emblem" },
    { MISSING, 0, "ALG02", "ALG", "Alexis Guendouz" },
    { MISSING, 0, "ALG03", "ALG", "Ramy Bensebaini" },
    { MISSING, 0, "ALG04", "ALG", "Youcef Atal" },
    { MISSING, 0, "ALG05", "ALG", "Rayan Aït-Nouri" },
    { MISSING, 0, "ALG06", "ALG", "Mohamed Amine Tougai" },
    { MISSING, 0, "ALG07", "ALG", "Aïssa Mandi" },
    { MISSING, 0, "ALG08", "ALG", "Ismael Bennacer" },
    { MISSING, 0, "ALG09", "ALG", "Houssem Aquar" },
    { MISSING, 0, "ALG10", "ALG", "Hicham Boudaoui" },
    { MISSING, 0, "ALG11", "ALG", "Ramiz Zerrouki" },
    { MISSING, 0, "ALG12", "ALG", "Nabil Bentalab" },
    { MISSING, 0, "ALG13", "ALG", "Team Photo" },
    { MISSING, 0, "ALG14", "ALG", "Farés Chaibi" },
    { MISSING, 0, "ALG15", "ALG", "Riyad Mahrez" },
    { MISSING, 0, "ALG16", "ALG", "Said Benrahma" },
    { MISSING, 0, "ALG17", "ALG", "Anis Hadj Moussa" },
    { MISSING, 0, "ALG18", "ALG", "Amine Gouiri" },
    { MISSING, 0, "ALG19", "ALG", "Baghdad Bounedjah" },
    { MISSING, 0, "ALG20", "ALG", "Mohammed Amoura" },
    { MISSING, 0, "AUT01", "AUT", "Emblem" },
    { MISSING, 0, "AUT02", "AUT", "Alexander Schlager" },
    { MISSING, 0, "AUT03", "AUT", "Patrick Pentz" },
    { MISSING, 0, "AUT04", "AUT", "David Alaba" },
    { MISSING, 0, "AUT05", "AUT", "Kevin Danso" },
    { MISSING, 0, "AUT06", "AUT", "Philipp Lienhart" },
    { MISSING, 0, "AUT07", "AUT", "Stefan Posch" },
    { MISSING, 0, "AUT08", "AUT", "Phillipp Mwene" },
    { MISSING, 0, "AUT09", "AUT", "Alexander Prass" },
    { MISSING, 0, "AUT10", "AUT", "Xaver Schlager" },
    { MISSING, 0, "AUT11", "AUT", "Marcel Sabitzer" },
    { MISSING, 0, "AUT12", "AUT", "Konrad Laimer" },
    { MISSING, 0, "AUT13", "AUT", "Team Photo" },
    { MISSING, 0, "AUT14", "AUT", "Florian Grillitsch" },
    { MISSING, 0, "AUT15", "AUT", "Nicolas Seiwald" },
    { MISSING, 0, "AUT16", "AUT", "Romano Schmid" },
    { MISSING, 0, "AUT17", "AUT", "Patrick Wimmer" },
    { MISSING, 0, "AUT18", "AUT", "Christoph Baumgartner" },
    { MISSING, 0, "AUT19", "AUT", "Michael Gregoritsch" },
    { MISSING, 0, "AUT20", "AUT", "Marko Arnautović" },
    { MISSING, 0, "JOR01", "JOR", "Emblem" },
    { MISSING, 0, "JOR02", "JOR", "Yazeed Abulaila" },
    { MISSING, 0, "JOR03", "JOR", "Ihsan Haddad" },
    { MISSING, 0, "JOR04", "JOR", "Mohammad Abu Hashish" },
    { MISSING, 0, "JOR05", "JOR", "Yazan Al-Arab" },
    { MISSING, 0, "JOR06", "JOR", "Abdallah Nasib" },
    { MISSING, 0, "JOR07", "JOR", "Saleem Obaid" },
    { MISSING, 0, "JOR08", "JOR", "Mohammad Abualnadi" },
    { MISSING, 0, "JOR09", "JOR", "Ibrahim Saadeh" },
    { MISSING, 0, "JOR10", "JOR", "Nizar Al-Rashdan" },
    { MISSING, 0, "JOR11", "JOR", "Noor Al-Rawabdeh" },
    { MISSING, 0, "JOR12", "JOR", "Mohannad Abu Taha" },
    { MISSING, 0, "JOR13", "JOR", "Team Photo" },
    { MISSING, 0, "JOR14", "JOR", "Amer Jamous" },
    { MISSING, 0, "JOR15", "JOR", "Musa Al-Taamari" },
    { MISSING, 0, "JOR16", "JOR", "Yazan Al-Naimat" },
    { MISSING, 0, "JOR17", "JOR", "Mahmoud Al-Mardi" },
    { MISSING, 0, "JOR18", "JOR", "Ali Olwan" },
    { MISSING, 0, "JOR19", "JOR", "Mohammad Abu Zrayq" },
    { MISSING, 0, "JOR20", "JOR", "Ibrahim Sabra" },
    { MISSING, 0, "POR01", "POR", "Emblem" },
    { MISSING, 0, "POR02", "POR", "Diogo Costa" },
    { MISSING, 0, "POR03", "POR", "Jose Sa" },
    { MISSING, 0, "POR04", "POR", "Ruben Dias" },
    { MISSING, 0, "POR05", "POR", "João Cancelo" },
    { MISSING, 0, "POR06", "POR", "Diogo Dalot" },
    { MISSING, 0, "POR07", "POR", "Nuno Mendes" },
    { MISSING, 0, "POR08", "POR", "Gonçalo Inácio" },
    { MISSING, 0, "POR09", "POR", "Bernardo Silva" },
    { MISSING, 0, "POR10", "POR", "Bruno Fernandes" },
    { MISSING, 0, "POR11", "POR", "Ruben Neves" },
    { MISSING, 0, "POR12", "POR", "Vitinha" },
    { MISSING, 0, "POR13", "POR", "Team Photo" },
    { MISSING, 0, "POR14", "POR", "João Neves" },
    { MISSING, 0, "POR15", "POR", "Cristiano Ronaldo" },
    { MISSING, 0, "POR16", "POR", "Francisco Trincao" },
    { MISSING, 0, "POR17", "POR", "João Felix" },
    { MISSING, 0, "POR18", "POR", "Gonçalo Ramos" },
    { MISSING, 0, "POR19", "POR", "Pedro Neto" },
    { MISSING, 0, "POR20", "POR", "Rafael Leão" },
    { MISSING, 0, "COD01", "COD", "Emblem" },
    { MISSING, 0, "COD02", "COD", "Lionel Mpasi" },
    { MISSING, 0, "COD03", "COD", "Aaron Wan-Bissaka" },
    { MISSING, 0, "COD04", "COD", "Axel Tuanzebe" },
    { MISSING, 0, "COD05", "COD", "Arthur Masuaku" },
    { MISSING, 0, "COD06", "COD", "Chancel Mbemba" },
    { MISSING, 0, "COD07", "COD", "Joris Kayembe" },
    { MISSING, 0, "COD08", "COD", "Charles Pickel" },
    { MISSING, 0, "COD09", "COD", "Ngal'ayel Mukau" },
    { MISSING, 0, "COD10", "COD", "Edo Kayembe" },
    { MISSING, 0, "COD11", "COD", "Samuel Moutoussamy" },
    { MISSING, 0, "COD12", "COD", "Noah Sadiki" },
    { MISSING, 0, "COD13", "COD", "Team Photo" },
    { MISSING, 0, "COD14", "COD", "Théo Bongonda" },
    { MISSING, 0, "COD15", "COD", "Meschak Elia" },
    { MISSING, 0, "COD16", "COD", "Yoane Wissa" },
    { MISSING, 0, "COD17", "COD", "Brian Cipenga" },
    { MISSING, 0, "COD18", "COD", "Fiston Mayele" },
    { MISSING, 0, "COD19", "COD", "Cédric Bakambu" },
    { MISSING, 0, "COD20", "COD", "Nathanaël Mbuku" },
    { MISSING, 0, "UZB01", "UZB", "Emblem" },
    { MISSING, 0, "UZB02", "UZB", "Utkir Yusupov" },
    { MISSING, 0, "UZB03", "UZB", "Farrukh Savfiev" },
    { MISSING, 0, "UZB04", "UZB", "Sherzod Nasrullaev" },
    { MISSING, 0, "UZB05", "UZB", "Umar Eshmurodov" },
    { MISSING, 0, "UZB06", "UZB", "Husniddin Aliqulov" },
    { MISSING, 0, "UZB07", "UZB", "Rustamjon Ashurmatov" },
    { MISSING, 0, "UZB08", "UZB", "Khojiakbar Alijonov" },
    { MISSING, 0, "UZB09", "UZB", "Abdukodir Khusanov" },
    { MISSING, 0, "UZB10", "UZB", "Odiljon Hamrobekov" },
    { MISSING, 0, "UZB11", "UZB", "Otabek Shukurov" },
    { MISSING, 0, "UZB12", "UZB", "Jamshid Iskanderov" },
    { MISSING, 0, "UZB13", "UZB", "Team Photo" },
    { MISSING, 0, "UZB14", "UZB", "Azizbek Turgunboev" },
    { MISSING, 0, "UZB15", "UZB", "Khojimat Erkinov" },
    { MISSING, 0, "UZB16", "UZB", "Eldor Shomurodov" },
    { MISSING, 0, "UZB17", "UZB", "Oston Urunov" },
    { MISSING, 0, "UZB18", "UZB", "Jaloliddin Masharipov" },
    { MISSING, 0, "UZB19", "UZB", "Igor Sergeev" },
    { MISSING, 0, "UZB20", "UZB", "Abbosbek Fayzullaev" },
    { MISSING, 0, "COL01", "COL", "Emblem" },
    { MISSING, 0, "COL02", "COL", "Camilo Vargas" },
    { MISSING, 0, "COL03", "COL", "David Ospina" },
    { MISSING, 0, "COL04", "COL", "Dávinson Sánchez" },
    { MISSING, 0, "COL05", "COL", "Yerry Mina" },
    { MISSING, 0, "COL06", "COL", "Daniel Munoz" },
    { MISSING, 0, "COL07", "COL", "Johan Mojica" },
    { MISSING, 0, "COL08", "COL", "Jhon Lucumí" },
    { MISSING, 0, "COL09", "COL", "Santiago Arias" },
    { MISSING, 0, "COL10", "COL", "Jefferson Lerma" },
    { MISSING, 0, "COL11", "COL", "Kevin Castaño" },
    { MISSING, 0, "COL12", "COL", "Richard Rios" },
    { MISSING, 0, "COL13", "COL", "Team Photo" },
    { MISSING, 0, "COL14", "COL", "James Rodriguez" },
    { MISSING, 0, "COL15", "COL", "Juan Fernando Quintero" },
    { MISSING, 0, "COL16", "COL", "Jorge Carrascal" },
    { MISSING, 0, "COL17", "COL", "Jon Arias" },
    { MISSING, 0, "COL18", "COL", "Jhon Cordova" },
    { MISSING, 0, "COL19", "COL", "Luis Suarez" },
    { MISSING, 0, "COL20", "COL", "Luis Diaz" },
    { MISSING, 0, "ENG01", "ENG", "Emblem" },
    { MISSING, 0, "ENG02", "ENG", "Jordan Pickford" },
    { MISSING, 0, "ENG03", "ENG", "John Stones" },
    { MISSING, 0, "ENG04", "ENG", "Marc Guéhi" },
    { MISSING, 0, "ENG05", "ENG", "Ezri Konsa" },
    { MISSING, 0, "ENG06", "ENG", "Trent Alexander-Arnold" },
    { MISSING, 0, "ENG07", "ENG", "Reece James" },
    { MISSING, 0, "ENG08", "ENG", "Dan Burn" },
    { MISSING, 0, "ENG09", "ENG", "Jordan Henderson" },
    { MISSING, 0, "ENG10", "ENG", "Declan Rice" },
    { MISSING, 0, "ENG11", "ENG", "Jude Bellingham" },
    { MISSING, 0, "ENG12", "ENG", "Cole Palmer" },
    { MISSING, 0, "ENG13", "ENG", "Team Photo" },
    { MISSING, 0, "ENG14", "ENG", "Morgan Rogers" },
    { MISSING, 0, "ENG15", "ENG", "Anthony Gordon" },
    { MISSING, 0, "ENG16", "ENG", "Phil Foden" },
    { MISSING, 0, "ENG17", "ENG", "Bukayo Saka" },
    { MISSING, 0, "ENG18", "ENG", "Harry Kane" },
    { MISSING, 0, "ENG19", "ENG", "Marcus Rashford" },
    { MISSING, 0, "ENG20", "ENG", "Ollie Watkins" },
    { MISSING, 0, "CRO01", "CRO", "Emblem" },
    { MISSING, 0, "CRO02", "CRO", "Dominik Livaković" },
    { MISSING, 0, "CRO03", "CRO", "Duje Caleta-Car" },
    { MISSING, 0, "CRO04", "CRO", "Josko Gvardiol" },
    { MISSING, 0, "CRO05", "CRO", "Josip Stanišić" },
    { MISSING, 0, "CRO06", "CRO", "Luka Vušković" },
    { MISSING, 0, "CRO07", "CRO", "Josip Sutalo" },
    { MISSING, 0, "CRO08", "CRO", "Kristijan Jakic" },
    { MISSING, 0, "CRO09", "CRO", "Luka Modrić" },
    { MISSING, 0, "CRO10", "CRO", "Mateo Kovacic" },
    { MISSING, 0, "CRO11", "CRO", "Martin Baturina" },
    { MISSING, 0, "CRO12", "CRO", "Lovro Majer" },
    { MISSING, 0, "CRO13", "CRO", "Team Photo" },
    { MISSING, 0, "CRO14", "CRO", "Mario Pasalic" },
    { MISSING, 0, "CRO15", "CRO", "Petar Sucic" },
    { MISSING, 0, "CRO16", "CRO", "Ivan Perišić" },
    { MISSING, 0, "CRO17", "CRO", "Marco Pasalic" },
    { MISSING, 0, "CRO18", "CRO", "Ante Budimir" },
    { MISSING, 0, "CRO19", "CRO", "Andrej Kramarić" },
    { MISSING, 0, "CRO20", "CRO", "Franjo Ivanovic" },
    { MISSING, 0, "GHA01", "GHA", "Emblem" },
    { MISSING, 0, "GHA02", "GHA", "Lawrence Ati Zigi" },
    { MISSING, 0, "GHA03", "GHA", "Tariq Lamptey" },
    { MISSING, 0, "GHA04", "GHA", "Mohammed Salisu" },
    { MISSING, 0, "GHA05", "GHA", "Alidu Seidu" },
    { MISSING, 0, "GHA06", "GHA", "Alexander Djiku" },
    { MISSING, 0, "GHA07", "GHA", "Gideon Mensah" },
    { MISSING, 0, "GHA08", "GHA", "Caleb Yirenkyi" },
    { MISSING, 0, "GHA09", "GHA", "Abdul Issahaku Fatawu" },
    { MISSING, 0, "GHA10", "GHA", "Thomas Partey" },
    { MISSING, 0, "GHA11", "GHA", "Salis Abdul Samed" },
    { MISSING, 0, "GHA12", "GHA", "Kamaldeen Sulemana" },
    { MISSING, 0, "GHA13", "GHA", "Team Photo" },
    { MISSING, 0, "GHA14", "GHA", "Mohammed Kudus" },
    { MISSING, 0, "GHA15", "GHA", "Inaki Williams" },
    { MISSING, 0, "GHA16", "GHA", "Jordan Ayew" },
    { MISSING, 0, "GHA17", "GHA", "Andrew Ayew" },
    { MISSING, 0, "GHA18", "GHA", "Joseph Paintsil" },
    { MISSING, 0, "GHA19", "GHA", "Osman Bukari" },
    { MISSING, 0, "GHA20", "GHA", "Antoine Semenyo" },
    { MISSING, 0, "PAN01", "PAN", "Emblem" },
    { MISSING, 0, "PAN02", "PAN", "Orlando Mosquera" },
    { MISSING, 0, "PAN03", "PAN", "Luis Mejia" },
    { MISSING, 0, "PAN04", "PAN", "Fidel Escobar" },
    { MISSING, 0, "PAN05", "PAN", "Andres Andrade" },
    { MISSING, 0, "PAN06", "PAN", "Michael Amir Murillo" },
    { MISSING, 0, "PAN07", "PAN", "Eric Davis" },
    { MISSING, 0, "PAN08", "PAN", "Jose Cordoba" },
    { MISSING, 0, "PAN09", "PAN", "Cesar Blackman" },
    { MISSING, 0, "PAN10", "PAN", "Cristian Martinez" },
    { MISSING, 0, "PAN11", "PAN", "Aníbal Godoy" },
    { MISSING, 0, "PAN12", "PAN", "Adalberto Carrasquilla" },
    { MISSING, 0, "PAN13", "PAN", "Team Photo" },
    { MISSING, 0, "PAN14", "PAN", "Édgar Bárcenas" },
    { MISSING, 0, "PAN15", "PAN", "Carlos Harvey" },
    { MISSING, 0, "PAN16", "PAN", "Ismael Díaz" },
    { MISSING, 0, "PAN17", "PAN", "Jose Fajardo" },
    { MISSING, 0, "PAN18", "PAN", "Cecilio Waterman" },
    { MISSING, 0, "PAN19", "PAN", "Jose Luiz Rodriguez" },
    { MISSING, 0, "PAN20", "PAN", "Alberto Quintero" },
    { MISSING, 0, "FWC09", "FWC", "Italy 1934" },
    { MISSING, 0, "FWC10", "FWC", "Uruguay 1950" },
    { MISSING, 0, "FWC11", "FWC", "West Germany 1954" },
    { MISSING, 0, "FWC12", "FWC", "Brazil 1962" },
    { MISSING, 0, "FWC13", "FWC", "West Germany 1974" },
    { MISSING, 0, "FWC14", "FWC", "Argentina 1986" },
    { MISSING, 0, "FWC15", "FWC", "Brazil 1994" },
    { MISSING, 0, "FWC16", "FWC", "Brazil 2002" },
    { MISSING, 0, "FWC17", "FWC", "Italy 2006" },
    { MISSING, 0, "FWC18", "FWC", "Germany 2014" },
    { MISSING, 0, "FWC19", "FWC", "Argentina 2022" },
};
