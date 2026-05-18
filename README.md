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

### Using clang (recommended)

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

## Usage
### List stickers
```bash
./stracker list [missing/have/duplicates] (optional)[country e.g. "mex" "arg" ...] (optional)[--oneline]
./stracker list d mex arg --oneline
./stracker list d
```
![img alt](https://i.imgur.com/fywN2Di.png)

### Album view (interactive)
```bash
./stracker album [TEAM_CODE e.g. MEX]
```
### Manually Add / Remove
```bash
./stracker add [CODE e.g. MEX01]
```
```bash
./stracker remove [CODE e.g. MEX01]
```

![img alt](https://i.imgur.com/z2wAHnE.png)
