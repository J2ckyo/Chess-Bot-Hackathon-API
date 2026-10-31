# Chess Bot Hackathon API

A C++ server where chess bots play each other over HTTP. Built for the University of Lethbridge Computer Science Club's chess bot hackathon.

The server holds the real board for every game. Bots never send a board: they read the position, then send back a single move. The server checks that move, applies it, and decides when the game is over. Bots can be written in any language that can make HTTP requests.

- [For participants](#for-participants)
  - [Quick start](#quick-start)
  - [How a game works](#how-a-game-works)
  - [API reference](#api-reference)
  - [Tournament format](#tournament-format)
  - [Watching games](#watching-games)
- [For organizers](#for-organizers)
  - [Building the server](#building-the-server)
  - [Running the server](#running-the-server)
  - [Running a tournament](#running-a-tournament)
- [Development](#development)
- [Known limitations](#known-limitations)

---

# For participants

## Quick start

You need Python 3.10 or newer.

```bash
cd bots_py
pip install -r requirements.txt
python Demo_Bot2.py --name YourBotName --server http://SERVER-ADDRESS:18080
```

Replace `SERVER-ADDRESS` with the address the organizers give you (`localhost` if the server runs on your own machine).

The bot registers once, saves its API key to `YourBotName.key.json`, then keeps checking for games and plays whenever it's its turn. Leave it running for the whole tournament.

**To write your own bot,** copy one of the demo bots in `bots_py/` and change `choose_move()`. That's the only function you need to touch: it receives the current position as a [python-chess](https://python-chess.readthedocs.io/) `Board` and returns a move. One demo plays random legal moves; the other searches ahead with alpha-beta pruning.

```python
def choose_move(board: chess.Board) -> chess.Move:
    return random.choice(list(board.legal_moves))
```

Keep your `.key.json` file private: anyone with your key can play moves as your bot. Bot names must be unique, 1 to 32 characters.

## How a game works

1. **Your bot finds its games:** `GET /api/my-games` lists the games it's currently playing.
2. **It reads the position:** `GET /api/games/<game_id>` returns the board as a [FEN](https://www.chessprogramming.org/Forsyth-Edwards_Notation) string, your colour, the moves so far, and the game's status.
3. **If it's your turn, send a move:** `POST /api/games/<game_id>/move` with the move in UCI notation.

### Moves (UCI notation)

A move is the starting square followed by the ending square:

| Move | Meaning |
|---|---|
| `e2e4` | piece on e2 moves to e4 |
| `e1g1` | castling (written as the king's move) |
| `e7e8q` | pawn promotes to a queen (`q`, `r`, `b` or `n`, always lowercase) |

### The rules the server enforces

- **Illegal moves are rejected** with a `409`. The board doesn't change and your bot can try again.
- **Moving out of turn** is rejected with a `403`.
- **A game ends** by checkmate, stalemate, the 50-move rule (100 half-moves with no capture or pawn move), or threefold repetition.
- **There is no time limit per move yet,** but please keep your bot under a few seconds a move.

## API reference

The server listens on port **18080**. All request and response bodies are JSON.

**Authentication:** send your bot's key in the `X-API-Key` header. Organizer routes need the `X-Admin-Key` header instead.

### Endpoints

| Method | Path | Auth | Purpose |
|---|---|---|---|
| POST | `/api/bots/register` | none | Register a bot, get its API key |
| GET | `/api/my-games` | bot key | List your bot's games in progress |
| GET | `/api/games/<game_id>` | bot key | Current state of one of your games |
| POST | `/api/games/<game_id>/move` | bot key | Play a move |
| GET | `/api/leaderboard` | none | Top 10 bots by rating |
| GET | `/api/tournament` | none | Current round and pairings |
| GET | `/api/games` | none | Every game and its status |
| GET | `/api/games/<game_id>/watch` | none | Spectator view of any game |
| POST | `/api/games` | admin key | Start a single game between two bots |
| POST | `/api/tournament/start` | admin key | Build the schedule, start round 1 |
| POST | `/api/tournament/next` | admin key | Start the next round |

### Register a bot

```
POST /api/bots/register
{ "name": "YourBotName" }
```

```json
{ "bot_id": "bot-1", "api_key": "3f9a0c7e..." }
```

The key is shown only once. `400` if the name is missing, too long, or already taken.

### Get your games

```
GET /api/my-games
X-API-Key: <your key>
```

```json
{ "games": ["game-1", "game-4"] }
```

### Get a game's state

```
GET /api/games/game-1
X-API-Key: <your key>
```

```json
{
  "ok": true,
  "game_id": "game-1",
  "fen": "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1",
  "your_colour": "black",
  "status": "in_progress",
  "moves": ["e2e4"]
}
```

`status` is one of `in_progress`, `white_wins`, `black_wins`, `draw`. Whose turn it is comes from the FEN's second field (`w` or `b`).

### Play a move

```
POST /api/games/game-1/move
X-API-Key: <your key>
{ "move": "e7e5" }
```

The reply has the same shape as the game state above, after your move.

### Errors

Game routes reply with `{ "ok": false, "error": "<message>" }` and one of these codes:

| Code | Meaning |
|---|---|
| 400 | Request body missing or malformed |
| 401 | Missing or wrong API key |
| 403 | Not your game, or not your turn |
| 404 | Game not found |
| 409 | Illegal move, or the game is already over |

## Tournament format

- **Round robin:** every bot plays every other bot once. With an odd number of bots, one bot sits out each round.
- **Colours alternate** between rounds.
- **Ratings are Elo,** starting at 1200 (K = 32). Beating a stronger bot gains more points than beating a weaker one; a draw between equal bots changes nothing.
- See the standings at `/api/leaderboard`, and the current round at `/api/tournament`.

## Watching games

Open `http://SERVER-ADDRESS:18080/static/watch.html` in a browser. It lists every game and draws the selected one as an ASCII board that updates twice a second, with the last move marked in brackets.

---

# For organizers

## Building the server

### Requirements

- **A C++23 compiler:** g++ 13 or newer. On Windows, install [MSYS2](https://www.msys2.org/) and, from its UCRT64 terminal, run:
  ```bash
  pacman -S mingw-w64-ucrt-x86_64-gcc make
  ```
  Then add `C:\msys64\ucrt64\bin` (and `C:\msys64\usr\bin` for `make`) to your `PATH`, so `g++` and `make` work from PowerShell. Check with `g++ --version`.
- **Git,** to fetch the two libraries the server uses: [Crow](https://github.com/CrowCpp/Crow) (the HTTP framework) and [Asio](https://think-async.com/Asio/) (networking, used by Crow). Both are header-only and included as git submodules, so there's nothing else to install.

### Get the code

```bash
git clone --recurse-submodules https://github.com/J2ckyo/Chess-Bot-Hackathon-API.git
cd Chess-Bot-Hackathon-API
```

If you already cloned without `--recurse-submodules`, fetch the libraries with:

```bash
git submodule update --init --recursive
```

If the build later says `asio.hpp: No such file or directory`, the submodules are missing or the paths at the top of the `Makefile` (`CROW_INC` and `ASIO_INC`) don't match where they ended up.

### Build

From the project folder:

| Command | What it does |
|---|---|
| `make` | Builds the server (`server.exe`) and the unit tests (`tests.exe`) |
| `make run` | Builds if needed, then starts the server |
| `make test` | Builds if needed, then runs the unit tests |
| `make clean` | Deletes all build output, for a fresh build |

Only changed files are recompiled, so rebuilds are quick.

**On Linux or macOS,** remove `-lws2_32 -lmswsock` (Windows networking libraries) from `LDLIBS` in the `Makefile`, add `-pthread`, and drop the `.exe` from the two output names.

## Running the server

The server refuses to start without an admin key, which protects the organizer routes. Set it in the same terminal you start the server from:

```powershell
# PowerShell
$env:ADMIN_KEY = "a-long-random-secret"
make run
```

```bash
# bash
export ADMIN_KEY="a-long-random-secret"
make run
```

The server then listens on port 18080. Check it's up by opening `http://localhost:18080/api/leaderboard`, which should show `{"leaderboard":[]}`. Stop it with `Ctrl + C`.

**For participants on other machines,** they connect to this computer's local IP address (find it with `ipconfig` on Windows). The firewall must allow incoming connections on port 18080.

## Running a tournament

A tournament is a series of **rounds**. In each round every bot plays at most one game, and the server starts those games for you. You, the organizer, decide when to start the tournament and when to move on to the next round.

### Before you start

- [ ] The server is running (see [Running the server](#running-the-server)).
- [ ] **Every participant's bot is running and has registered.** Each bot prints a line like `Registered RandomBot as bot-3` when it does.
- [ ] Every registered bot is **still running**. A bot that registered and was then closed will never move, so its games never finish and the round gets stuck.

The schedule is built from the bots registered at the moment you start. A bot that registers later is left out until you start a new tournament.

### Step 1: set up your terminal

Open a **new** PowerShell window (the server's window is busy running the server) and store the address and admin key once, so the commands below stay short:

```powershell
$api   = "http://localhost:18080/api"
$admin = @{ "X-Admin-Key" = "a-long-random-secret" }   # the same value as ADMIN_KEY
```

### Step 2: start the tournament

```powershell
Invoke-RestMethod -Method Post "$api/tournament/start" -Headers $admin
```

You should see:

```
round total_rounds
----- ------------
    1            3
```

The round 1 games have now started. Every bot's next `/api/my-games` check finds its game, and the bots start playing on their own.

### Step 3: watch the round

Check the current round at any time:

```powershell
(Invoke-RestMethod "$api/tournament").pairings
```

```
white_id black_id game_id
-------- -------- -------
bot-1    bot-4    game-1
bot-2    bot-3    game-2
```

and each game's status:

```powershell
(Invoke-RestMethod "$api/games").games
```

```
game_id white_id black_id status
------- -------- -------- ------
game-1  bot-1    bot-4    white_wins
game-2  bot-2    bot-3    in_progress
```

The round is finished when **none** of its games say `in_progress`. You can also watch the games live at `http://localhost:18080/static/watch.html`.

### Step 4: start the next round

```powershell
Invoke-RestMethod -Method Post "$api/tournament/next" -Headers $admin
```

- **The next round started:** the reply shows the new `round` number. Go back to step 3.
- **`409 Current round still in progress`:** at least one game is still being played. Wait and try again.
- **`finished: True`:** that was the last round, and the tournament is over.

### Step 5: the results

```powershell
(Invoke-RestMethod "$api/leaderboard").leaderboard
```

```
bot_id rating wins losses draws
------ ------ ---- ------ -----
bot-3    1231    3      0     0
bot-1    1214    2      1     0
...
```

### Quick reference

| To do this | Run |
|---|---|
| Start the tournament | `Invoke-RestMethod -Method Post "$api/tournament/start" -Headers $admin` |
| See this round's pairings | `(Invoke-RestMethod "$api/tournament").pairings` |
| See every game's status | `(Invoke-RestMethod "$api/games").games` |
| Start the next round | `Invoke-RestMethod -Method Post "$api/tournament/next" -Headers $admin` |
| See the standings | `(Invoke-RestMethod "$api/leaderboard").leaderboard` |

### If something goes wrong

| You see | It means | Fix |
|---|---|---|
| `Need 2 registered bots` | Fewer than 2 bots have registered since the server started | Start the bots, then run step 2 again |
| `Not admin` (403) | The admin key in `$admin` doesn't match the server's `ADMIN_KEY` | Re-run step 1 with the right key |
| `Current round still in progress` (409) | A game in this round hasn't ended | Wait; if it never ends, see the next row |
| A game stays `in_progress` with no new moves | One of its bots has stopped or crashed | Restart that bot (it reuses its saved key) and it carries on |
| `Unable to connect` | The server isn't running, or the address is wrong | Start the server, check `$api` |

**Starting over:** running step 2 again builds a fresh schedule from every bot currently registered and starts a new round 1. Ratings carry over between tournaments; restart the server to reset everything.

### Playing a single game instead

To pair two specific bots without a tournament (useful for testing a bot):

```powershell
Invoke-RestMethod -Method Post "$api/games" -Headers $admin -ContentType "application/json" -Body '{"white_id":"bot-1","black_id":"bot-2"}'
```

Use the ids the bots printed when they registered. `Can't start game` means one of the ids isn't registered on this server run, or both are the same bot.

---

# Development

## Project structure

The server follows MVC. Each layer only depends on the layers below it, so the chess engine and models can be tested without the server running.

```
include/            headers        src/              implementations
  model/engine/       Piece, Square, Move, Board, MoveGenerator   chess rules
  model/              Game, Bot, BotRegistry, Leaderboard, GameState
  controller/         GameController, TournamentManager
  server/             ApiRoutes      HTTP routes (Crow), JSON in and out
main.cpp            builds everything and starts the server
static/             watch.html, served at /static/
test/test.cpp       unit tests
bots_py/            demo bots for participants
docs/               UML and data-flow diagrams
```

| Layer | Classes | Job |
|---|---|---|
| Engine | `Piece`, `Square`, `Move`, `Board`, `MoveGenerator` | Board state, FEN in and out, legal move generation |
| Models | `Game`, `BotRegistry`, `Leaderboard` | One match, bot identities and keys, ratings |
| Controllers | `GameController`, `TournamentManager` | Authentication, turn order, pairings |
| HTTP | `ApiRoutes` | Turns requests into controller calls, and results into JSON |

## Testing

- **Unit tests:** `make test` runs about 270 checks on every layer below HTTP, including [perft](https://www.chessprogramming.org/Perft) counts that confirm move generation is correct (20, 400, 8902 and 197281 nodes from the start position).
- **HTTP tests:** with the server running, from a second terminal with the same `ADMIN_KEY`:
  ```powershell
  powershell -ExecutionPolicy Bypass -File .\test_api.ps1
  ```
  This registers bots, plays a game over HTTP, and checks every route and error code.

# Known limitations

- **Everything is in memory.** Restarting the server deletes all bots, games and ratings; participants must then delete their `.key.json` file and re-register. Database persistence is planned.
- **No move time limit** yet, so a stalled bot can hold up a round.
- **The leaderboard shows bot ids,** not names.
- **Plain HTTP,** so keys travel unencrypted. Fine on a local network; put the server behind HTTPS before exposing it to the internet.
