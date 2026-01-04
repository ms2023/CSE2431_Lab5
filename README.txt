# Unix Shell Extensions with Process Management

This project extends a provided Unix shell skeleton by implementing core
systems-level functionality related to process creation, execution, signal
handling, and persistent command history. The original skeleton handled basic
command parsing and input handling; all process execution logic and supporting
features were implemented independently.

---

## Project Overview

The shell provides an interactive command-line interface that supports execution
of Unix commands, background processes, and persistent command history. It
demonstrates fundamental operating systems concepts by directly interfacing
with the Unix process model and signal system.

The project emphasizes extending an existing codebase, managing system
resources safely, and building reliable low-level software.

---

## Implemented Features

### Process Execution
- Implemented command execution using `fork`, `execvp`, and `wait`
- Supports both foreground and background process execution
- Ensures proper parent-child synchronization

### Signal Handling
- Added a `SIGINT` handler to support asynchronous behavior
- Allows users to view command history without terminating the shell

### Command History
- Implemented a fixed-size circular buffer to store recent commands
- Supports history recall using a custom `r` command
- Allows re-execution of:
  - the most recent command
  - the most recent command beginning with a given character

### Persistent State
- Command history is saved to disk using file I/O
- History persists across shell sessions

---

## What Was Provided vs Implemented

**Provided:**
- Base shell loop
- Command parsing logic
- Input tokenization framework

**Implemented:**
- Process creation and execution (`fork`, `execvp`, `wait`)
- Background process handling
- Signal handling (`SIGINT`)
- Command history system
- History persistence using file I/O
- Circular buffer management

---

## How to Build and Run

### Requirements
- Unix-like operating system (Linux or macOS)
- `make`
- C compiler (e.g., `gcc`)

### Build
```bash
make shell
```
### Run
```bash
./shell
```
