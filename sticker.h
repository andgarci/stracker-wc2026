#ifndef STICKER_H
#define STICKER_H

#define MAX_STICKERS 980

typedef enum {
	MISSING = 0,
	HAVE = 1,
	DUPLICATE = 2
} Status;

extern const char *status_names[];

typedef struct {
	Status status;
	int quantity;
	char code[6];
	char team_code[4];
	char name[64];
} Sticker;

Sticker *sticker_find(Sticker stickers[], char code[], char message[]);
void sticker_add(Sticker stickers[], char code[], char message[]);
int sticker_remove(Sticker stickers[], char code[], char message[]);
void sticker_list(Sticker stickers[], int argc, char *argv[], char message[]);
void album_page(Sticker stickers[], char team_code[], char message[]);

extern Sticker CATALOG[MAX_STICKERS];

#endif
