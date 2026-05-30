# Morse Code Translator & Logger

This repository contains a complete C project for a Morse code translator, session logger, user manager, alphabet manager, and analytics report generator.

## Build

```sh
make
```

## Run

```sh


```

This launches a menu-driven console application that supports:
- text -> Morse encoding
- Morse -> text decoding
- session history viewing
- user registration and login
- alphabet switching and custom entries
- analytics report generation

## Test

```sh
make test
```

The test target builds the application and runs a smoke test plus module coverage tests.

## Data files

- `data/morse_table.txt` contains the Morse alphabet mapping.
- `data/sessions.txt` stores session history between runs.

## Notes

- The app saves session logs automatically on exit.
- A default alphabet is loaded from `data/morse_table.txt`.
- Use option `0` at the main menu to exit cleanly.

