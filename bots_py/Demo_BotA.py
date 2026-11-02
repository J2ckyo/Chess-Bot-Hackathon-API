"""
Starter bot for the Chess Bot Hackathon.

It registers once, then keeps checking for games. Whenever it's this bot's
turn, it asks choose_move() for a move and sends it to the server.

The only part you need to change is choose_move() at the bottom.

Run:
    pip install -r requirements.txt
    python bot.py --name MyBot
"""

import argparse
import json
import random
import sys
import time
from pathlib import Path

import chess
import requests

POLL_SECONDS = 1.0  # how often to check for games


# ---------------------------------------------------------------- Talking to the server

class Server:
    def __init__(self, base_url: str):
        self.base = base_url.rstrip("/") + "/api"
        self.key = None

    def _headers(self):
        return {"X-API-Key": self.key} if self.key else {}

    def register(self, name: str) -> dict:
        r = requests.post(f"{self.base}/bots/register", json={"name": name}, timeout=10)
        if r.status_code != 201:
            raise RuntimeError(f"Registration failed ({r.status_code}): {r.text}")
        return r.json()  # {"bot_id": ..., "api_key": ...}

    def my_games(self) -> list:
        r = requests.get(f"{self.base}/my-games", headers=self._headers(), timeout=10)
        r.raise_for_status()
        return r.json()["games"]

    def game_state(self, game_id: str) -> dict:
        r = requests.get(f"{self.base}/games/{game_id}", headers=self._headers(), timeout=10)
        return r.json()

    def send_move(self, game_id: str, uci: str) -> dict:
        r = requests.post(
            f"{self.base}/games/{game_id}/move",
            headers=self._headers(),
            json={"move": uci},
            timeout=10,
        )
        try:
            return r.json()
        except ValueError:
            return {"ok": False, "error": r.text}


# ---------------------------------------------------------------- Remembering the API key

def load_or_register(server: Server, name: str) -> str:
    """Registers the first time; after that, reuses the key saved in <name>.key.json."""
    key_file = Path(f"{name}.key.json")
    if key_file.exists():
        saved = json.loads(key_file.read_text())
        print(f"Using saved key for {name} ({saved['bot_id']})")
        return saved["api_key"]

    info = server.register(name)
    key_file.write_text(json.dumps(info, indent=2))
    print(f"Registered {name} as {info['bot_id']} (key saved to {key_file})")
    return info["api_key"]


# ---------------------------------------------------------------- The main loop

def play_turn_if_mine(server: Server, game_id: str) -> None:
    state = server.game_state(game_id)
    if not state.get("ok") or state.get("status") != "in_progress":
        return

    board = chess.Board(state["fen"])
    my_colour = chess.WHITE if state["your_colour"] == "white" else chess.BLACK
    if board.turn != my_colour:
        return  # opponent's turn

    move = choose_move(board)
    result = server.send_move(game_id, move.uci())
    if result.get("ok"):
        print(f"[{game_id}] played {move.uci()}  -> {result['status']}")
    else:
        print(f"[{game_id}] move {move.uci()} rejected: {result.get('error')}")


def main() -> None:
    parser = argparse.ArgumentParser(description="Chess Bot Hackathon starter bot")
    parser.add_argument("--name", required=True, help="your bot's name (must be unique)")
    parser.add_argument("--server", default="http://localhost:18080", help="server address")
    args = parser.parse_args()

    server = Server(args.server)
    try:
        server.key = load_or_register(server, args.name)
    except (RuntimeError, requests.RequestException) as e:
        print(e)
        sys.exit(1)

    print("Waiting for games... (Ctrl + C to stop)")
    while True:
        try:
            for game_id in server.my_games():
                play_turn_if_mine(server, game_id)
        except requests.RequestException as e:
            print(f"Server problem: {e}")
        time.sleep(POLL_SECONDS)


# ---------------------------------------------------------------- Your strategy goes here

def choose_move(board: chess.Board) -> chess.Move:
    """
    Return the move to play. `board` is the current position, and it's your turn.

    This starter version picks a random legal move. Replace it with something smarter:
    prefer captures, look a few moves ahead (minimax), score positions, ...

    Useful python-chess tools:
        board.legal_moves          every legal move
        board.is_capture(move)     does this move capture?
        board.push(move) / board.pop()   try a move, then undo it
        board.is_checkmate()       after a push: did that move win?
    """
    return random.choice(list(board.legal_moves))


if __name__ == "__main__":
    main()