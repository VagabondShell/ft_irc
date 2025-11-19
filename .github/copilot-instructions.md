## Quick context

This repo implements a small IRC server (C++98) and a separate bot client.
Key folders: `includes/` (headers) and `srcs/` (sources). The server binary is
built by the `Makefile` and the main server entry is `srcs/main.cpp`.

## Big-picture architecture (what to know first)
- Server (`includes/Server.hpp`, `srcs/Server.cpp`): event loop uses `poll()` on
  `std::vector<pollfd> _pollFds`. Listener FD is stored in `_listenerFd`. New
  clients are accepted in `handleNewConnection()` and stored in `_clients` map
  (key = fd) and `_pollFds`.
- Client (`includes/Client.hpp`, `srcs/Client.cpp`): keeps read/out buffers
  (`_ReadBuffer`, `_OutBuffer`) and small registration state. `SendReply()`
  appends to `_OutBuffer` and calls `SetPollOut(true)` which flips POLLOUT in the
  global `_pollFds` (via `Server::getPollfds()` reference).
- Commands: parsing happens in `Server::commandDispatcher()` which uses
  `split_string_to_vector()` to split on spaces and treat a trailing `:text`
  token as a single final argument (e.g. PRIVMSG target :hello world ->
  tokens {"PRIVMSG","target","hello world"}). Command string -> enum map
  lives in `_commandMap` (see `Server` constructor).
- Bot (`srcs/bot.cpp`): separate executable (has its own `main`) that can
  connect to the server; includes `start_bot_loop()` and `processBotCommand()`.

## Build & run (concrete commands)
- Build server: run `make` at the repository root (target produces `ircserv`).
- Run server: `./ircserv <port> <password>` (example: `./ircserv 6667 mypass`).
- Bot: not included in `Makefile`. Build manually when needed, for example:
  `c++ -Wall -Wextra -Werror -std=c++98 -fsanitize=address srcs/bot.cpp -o ftbot`
  then `./ftbot 127.0.0.1 6667 mypass` (port must be >1024 per bot checks).

## Project-specific patterns & conventions for code changes
- C++ standard: codebase uses C++98 flags; follow legacy style (no modern C++
  features like smart pointers). Keep ABI-compatible with this standard.
- Network I/O: sockets are non-blocking; server relies on `poll()` and manual
  send/recv handling. When editing buffering or socket code, preserve the
  non-blocking behavior and EAGAIN/EWOULDBLOCK checks.
- Poll array invariants: `_pollFds` and `_clients` are parallel structures.
  When adding a client do three things: accept(), create `Client*`, add to
  `_clients[new_fd]`, and push a `pollfd` into `_pollFds`. When removing a
  client, remove the `pollfd` entry and delete the `Client*` and erase from the
  map (see `Server::disconnectClient()` and `run()` where entries are erased).
- Outgoing messages: `Client::SendReply()` writes into `_OutBuffer` and sets
  the pollfd's POLLOUT flag via `SetPollOut(true)`. To reliably send data, use
  `Server::handleOutgoingData()` which erases sent bytes from `_OutBuffer` and
  clears POLLOUT when empty.

## Common fixes & hotspots (examples to check when editing)
- Registration state: registration requires PASS + NICK + USER (see
  `Server::checkRegistration()`). Avoid bypassing this flow when adding new
  commands that should only work after registration.
- Thread-safety/memory: clients are raw `new`/`delete`. If you touch lifetime
  logic, ensure you don't use deleted pointers elsewhere (search for usages of
  `_clients[...]` and `_nicknames`).
- Duplicate functionality: there are two very similar split functions
  (`split_string_to_vector` in `Server.cpp` and `splitVector` in `bot.cpp`). Be
  cautious about changing one style without updating the other if you intend a
  consistent command format.
- Command list: command strings are uppercase in `_commandMap`. New server
  commands should be added there and handled in `Server::commandDispatcher()`.

## Integration points to watch
- `Client::SetPollOut()` mutates `Server::_pollFds` by reference from the
  client. Edits that change how `pollfd` entries are stored/identified must
  also update this method.
- `_nicknames` maps nickname -> `Client*` and is used by `handlePrivmsgCommand`
  to route messages. Ensure nickname uniqueness logic (`handleNickCommand`) is
  preserved when changing naming rules.
- Error handling: `Server` constructor throws on socket errors; callers (main)
  catch std::exception and print to stderr. Tests or automation should expect
  non-zero exit codes for bind/connect failures.

## Useful code pointers (where to look for examples)
- Accept and poll loop: `srcs/Server.cpp` (constructor, `run()`,
  `handleNewConnection()`).
- Command parsing and replies: `srcs/Server.cpp` (`commandDispatcher()`,
  `handlePassCommand()`, `handleNickCommand()`, `handleUserCommand()`,
  `handlePrivmsgCommand()` in `srcs/Command.cpp`).
- Client buffer & POLLOUT handling: `srcs/Client.cpp` (`SendReply()`,
  `ProcessAndExtractCommands()`, `SetPollOut()`).
- Bot examples and message handling: `srcs/bot.cpp`.

## When proposing changes, be explicit about:
- Which files you will edit and why (e.g. "change Client::SendReply to include
  full prefix -> edit `srcs/Client.cpp` and adjust tests").
- Any invariant you rely on (e.g. "I will preserve non-blocking sockets and
  `poll` semantics: no blocking reads in main thread").
- Run steps you used to validate (e.g. `make && ./ircserv 6667 secret` and
  `./ftbot 127.0.0.1 6667 secret`).

If anything here is unclear or you'd like me to expand a specific section
(examples of commands, tests to add, or a checklist for safely changing the
poll/fd logic), tell me which area and I'll iterate.  