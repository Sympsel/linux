# AGENTS.md - C++ Development Workspace

## Build Commands

```bash
# Build a project (from project directory)
cmake -B build -G Ninja && ninja -C build

# Or use cmake-build-debug (CLion default)
cmake -B cmake-build-debug -G Ninja && ninja -C cmake-build-debug

# Run executable (snake example)
./build/snake  # or ./cmake-build-debug/snake
```

## Project Structure

| Directory | Description |
|-----------|-------------|
| `snake/` | Terminal-based Snake game (ncurses, mp3 audio) |
| `symnet/` | Networking library (UDP, TCP utilities) |
| `http_server/` | HTTP server implementation |
| `cpp20L/` | C++20 learning examples |
| `012_chatroom/` | Chatroom server/client |
| `013_udpechoserver/` | UDP echo server |

## Dependencies

- **snake**: ncurses, jsoncpp, pthread, mpg123 (audio)
- **All projects**: C++20 (cmake minimum 4.2)

## Key Conventions

- Build outputs go in `build/` or `cmake-build-debug/`
- Each project has its own `CMakeLists.txt`
- Config via `config.json` (snake project)
- Run from project root (not build dir) - assets/paths resolved relative to cwd
