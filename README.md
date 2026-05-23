# Stracker WC2026
## A CLI sticker tracker for the Panini's 2026 World Cup Album.

This tracker does not aim to be the most practical, but rather an open-source experiment focused on customizability and full CLI-based control over your sticker collection.

---
![img alt](https://i.imgur.com/AfDzyCt.png)
---

## Requirements

- C compiler (clang or gcc)
- Make (optional, if using Makefile)
- Unix-like environment recommended (Linux / macOS)
- Windows supported if compiled via MinGW or WSL

---

## Build

### Using clang

```bash
clang -o stracker main.c sticker.c storage.c
```

### Using make 
```bash
make
```

## Run
```bash
./stracker [command]
```

## Help
To see available commands and usage, run:
```bash
./stracker help
```
If you run `./stracker` with no arguments, help will be shown by default.

## Usage
### List stickers
```bash
./stracker list [missing|have|duplicate|m|h|d] (optional)[TEAM e.g. "mex" "arg" ...] (optional)[--oneline]
./stracker list d mex arg --oneline
./stracker list d
```
![img alt](https://i.imgur.com/fywN2Di.png)

### Album view (interactive)
```bash
./stracker album [TEAM_CODE e.g. MEX]
```

### Add / Remove stickers
```bash
./stracker add [CODE e.g. MEX01]
./stracker remove [CODE e.g. MEX01]
```

![img alt](https://i.imgur.com/z2wAHnE.png)

### Compare collections with a friend
To find out which stickers you can trade with someone, share your `storage.dat` files and run:
```bash
./stracker compare storage.dat friend_storage.dat
```
This will show two lists:
- **They can give you** — stickers you're missing that they already have
- **You can give them** — your duplicates that they're still missing
