#ifndef STICKER_H
#define STICKER_H

#define MAX_STICKERS 980

typedef enum {
	MISSING = 0,
	HAVE = 1,
	DUPLICATE = 2
} Status;

const char *status_names[];

typedef struct {
	Status status;
	int quantity;
	char code[6];
	char team_code[4];
	char name[64];
} Sticker;

Sticker *sticker_find(Sticker stickers[], int count, char code[], char message[]);
void sticker_add(Sticker stickers[], int *count, char code[], char message[]);
void sticker_remove(Sticker stickers[], int *count, char code[], char message[]);
void sticker_list(Sticker stickers[], int *count, int argc, char *argv[], char message[]);
void album_page(Sticker stickers[], char team_code[], char message[]);

extern Sticker CATALOG[MAX_STICKERS];

#endif
