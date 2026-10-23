/**
 * Tests for Piece, Square, and Move.
 * Run with: make run
 * Prints only failures, then a summary.
 */

#include <iostream>
#include <optional>
#include <sstream>
#include <string>

#include "Piece.hpp"
#include "Square.hpp"
#include "Move.hpp"
#include "Board.hpp"
#include "MoveGenerator.hpp"
#include "Game.hpp"
#include <stdexcept>

static int passed = 0;
static int failed = 0;

static void check(bool condition, const std::string& label) {
    if (condition) {
        ++passed;
    } else {
        ++failed;
        std::cout << "FAIL: " << label << '\n';
    }
}

// Like check, but prints what was expected and what came out
static void check_eq(const std::string& got, const std::string& expected, const std::string& label) {
    if (got == expected) {
        ++passed;
    } else {
        ++failed;
        std::cout << "FAIL: " << label << '\n'
                  << "   expected: " << expected << '\n'
                  << "   got:      " << got << '\n';
    }
}

// ---------------------------------------------------------------- Piece

static void test_piece() {
    // Every valid letter converts to a piece and back to the same letter
    const std::string letters = "PRNBQKprnbqk";
    for (char c : letters) {
        std::optional<Piece> p = Piece::piece_from_fen(c);
        check(p.has_value(), std::string("piece_from_fen('") + c + "') is a piece");
        if (p) {
            check(Piece::piece_to_fen(p->kind, p->colour) == c,
                  std::string("round trip '") + c + "'");
        }
    }

    // Specific kinds and colours
    std::optional<Piece> white_rook = Piece::piece_from_fen('R');
    check(white_rook && white_rook->kind == Piece::ROOK && white_rook->colour == Piece::WHITE,
          "'R' is a white rook");

    std::optional<Piece> black_knight = Piece::piece_from_fen('n');
    check(black_knight && black_knight->kind == Piece::KNIGHT && black_knight->colour == Piece::BLACK,
          "'n' is a black knight");

    // Characters that are not pieces
    const std::string bad = "x3/ ?";
    for (char c : bad) {
        check(!Piece::piece_from_fen(c).has_value(),
              std::string("piece_from_fen('") + c + "') is empty");
    }
}

// ---------------------------------------------------------------- Square

static void test_square() {
    std::optional<Square> e4 = Square::squares_from_fen("e4");
    check(e4 && e4->rank == 3 && e4->file == 4, "\"e4\" is rank 3, file 4");
    check(e4 && e4->squares_to_fen() == "e4", "\"e4\" round trip");

    std::optional<Square> a1 = Square::squares_from_fen("a1");
    check(a1 && a1->rank == 0 && a1->file == 0, "\"a1\" is rank 0, file 0");

    std::optional<Square> h8 = Square::squares_from_fen("h8");
    check(h8 && h8->rank == 7 && h8->file == 7, "\"h8\" is rank 7, file 7");

    // Bad names
    const std::string bad[] = {"i1", "a9", "a0", "e", "e44", "", "E4"};
    for (const std::string& name : bad) {
        check(!Square::squares_from_fen(name).has_value(),
              "squares_from_fen(\"" + name + "\") is empty");
    }

    // on_board(rank, file)
    check(Square::on_board(0, 7), "on_board(0, 7) is true");
    check(Square::on_board(3, 4), "on_board(3, 4) is true");
    check(!Square::on_board(-1, 3), "on_board(-1, 3) is false");
    check(!Square::on_board(8, 0), "on_board(8, 0) is false");
    check(!Square::on_board(0, 8), "on_board(0, 8) is false");

    // Equality
    std::optional<Square> also_e4 = Square::squares_from_fen("e4");
    std::optional<Square> d5 = Square::squares_from_fen("d5");
    check(e4 && also_e4 && *e4 == *also_e4, "e4 == e4");
    check(e4 && d5 && !(*e4 == *d5), "e4 != d5");
}

// ---------------------------------------------------------------- Move

static void test_move() {
    // Valid moves convert to a Move and back to the same text
    const std::string good[] = {"e2e4", "g1f3", "e1g1", "e7e8q", "a7a8n", "h2h1r", "b7b8b"};
    for (const std::string& text : good) {
        std::optional<Move> m = Move::from_uci(text);
        check(m.has_value(), "from_uci(\"" + text + "\") is a move");
        if (m) {
            check(m->to_uci() == text, "round trip \"" + text + "\"");
        }
    }

    // Fields of a promotion
    std::optional<Move> promo = Move::from_uci("e7e8q");
    std::optional<Square> e7 = Square::squares_from_fen("e7");
    std::optional<Square> e8 = Square::squares_from_fen("e8");
    check(promo && promo->from == *e7, "e7e8q: from is e7");
    check(promo && promo->to == *e8, "e7e8q: to is e8");
    check(promo && promo->promotion_piece == Piece::QUEEN, "e7e8q: promotes to a queen");

    // A normal move has no promotion
    std::optional<Move> normal = Move::from_uci("e2e4");
    check(normal && !normal->promotion_piece.has_value(), "e2e4: no promotion");

    // Bad moves
    const std::string bad[] = {"e7e8k", "e7e8p", "e7e8Q", "e7e8x", "e2", "e2e", "z2e4", "e2e9",
                               "e2e4qq", ""};
    for (const std::string& text : bad) {
        check(!Move::from_uci(text).has_value(), "from_uci(\"" + text + "\") is empty");
    }
}

// ---------------------------------------------------------------- Board: reading and writing FEN

static const std::string START_FEN =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

static void test_board_fen() {
    // Any valid FEN read in should come back out unchanged
    const std::string round_trips[] = {
        START_FEN,
        "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1",
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        "rnbqkb1r/ppp1pppp/5n2/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 3",
        "8/8/8/8/8/8/8/k6K b - - 12 40",
    };
    for (const std::string& fen : round_trips) {
        std::optional<Board> b = Board::read_all_fen(fen);
        check(b.has_value(), "read_all_fen accepts: " + fen);
        if (b) check_eq(b->all_to_fen(), fen, "FEN round trip");
    }

    // Malformed FENs must be rejected
    const std::string bad[] = {
        "",
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR",                    // missing fields
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP w KQkq - 0 1",                 // 7 rows
        "rnbqkbnr/pppppppp/8/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",      // 9 rows
        "rnbqkbnr/ppppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",       // row too long
        "rnbqkbnr/ppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",         // row too short
        "rnbqkbnr/pppppppp/9/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",        // digit 9
        "rnbqkbnr/pppxpppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",        // bad letter
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR x KQkq - 0 1",        // bad turn
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQxq - 0 1",        // bad castling
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq z9 0 1",       // bad en passant
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 0",        // fullmove 0
    };
    for (const std::string& fen : bad) {
        check(!Board::read_all_fen(fen).has_value(), "read_all_fen rejects: " + fen);
    }

    // piece_at on the starting position
    std::optional<Board> start = Board::read_all_fen(START_FEN);
    if (start) {
        std::optional<Piece> e1 = start->piece_at(*Square::squares_from_fen("e1"));
        std::optional<Piece> e8 = start->piece_at(*Square::squares_from_fen("e8"));
        std::optional<Piece> e4 = start->piece_at(*Square::squares_from_fen("e4"));
        check(e1 && e1->kind == Piece::KING && e1->colour == Piece::WHITE, "start: white king on e1");
        check(e8 && e8->kind == Piece::KING && e8->colour == Piece::BLACK, "start: black king on e8");
        check(!e4.has_value(), "start: e4 is empty");
    }
}

// ---------------------------------------------------------------- Board: apply

// Reads `fen`, applies each move in `moves` (space separated), and compares the result
static void check_apply(const std::string& fen, const std::string& moves,
                        const std::string& expected, const std::string& label) {
    std::optional<Board> b = Board::read_all_fen(fen);
    if (!b) {
        check(false, label + " (could not read starting FEN)");
        return;
    }
    std::stringstream ss(moves);
    std::string uci;
    while (ss >> uci) {
        std::optional<Move> m = Move::from_uci(uci);
        if (!m) {
            check(false, label + " (bad move text: " + uci + ")");
            return;
        }
        b->apply(*m);
    }
    check_eq(b->all_to_fen(), expected, label);
}

static void test_board_apply() {
    // Normal moves, counters, turn
    check_apply(START_FEN, "g1f3",
        "rnbqkbnr/pppppppp/8/8/8/5N2/PPPPPPPP/RNBQKB1R b KQkq - 1 1",
        "knight move: halfmove +1, turn flips");

    // Two-square pawn move sets the en passant square
    check_apply(START_FEN, "e2e4",
        "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1",
        "e2e4 sets en passant e3");

    // The en passant square is cleared after the next move
    check_apply(START_FEN, "e2e4 g8f6",
        "rnbqkb1r/pppppppp/5n2/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 1 2",
        "en passant square cleared; fullmove +1 after black");

    // En passant capture removes the pawn beside the landing square
    check_apply("rnbqkb1r/ppp1pppp/5n2/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 3", "e5d6",
        "rnbqkb1r/ppp1pppp/3P1n2/8/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 3",
        "en passant capture removes d5 pawn");

    // Castling moves the rook too, and removes that side's rights
    check_apply("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1", "e1g1",
        "r3k2r/8/8/8/8/8/8/R4RK1 b kq - 1 1",
        "white castles kingside");
    check_apply("r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1", "e8c8",
        "2kr3r/8/8/8/8/8/8/R3K2R w KQ - 1 2",
        "black castles queenside");

    // A rook leaving its corner removes that one right
    check_apply("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1", "h1h2",
        "r3k2r/8/8/8/8/8/7R/R3K3 b Qkq - 1 1",
        "h1 rook moves: lose K");

    // Capturing a rook on its corner removes the opponent's right too
    check_apply("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1", "a1a8",
        "R3k2r/8/8/8/8/8/8/4K2R b Kk - 0 1",
        "a1 rook takes a8 rook: lose Q and q, halfmove resets");

    // Losing the last right leaves "-"
    check_apply("4k3/8/8/8/8/8/8/R3K3 w Q - 0 1", "e1d1",
        "4k3/8/8/8/8/8/8/R2K4 b - - 1 1",
        "king moves: last right gone, castling is -");

    // Promotion, plain and with capture
    check_apply("8/P7/8/8/8/8/8/k6K w - - 0 1", "a7a8q",
        "Q7/8/8/8/8/8/8/k6K b - - 0 1",
        "pawn promotes to queen");
    check_apply("1r6/P7/8/8/8/8/8/k6K w - - 0 1", "a7b8n",
        "1N6/8/8/8/8/8/8/k6K b - - 0 1",
        "pawn captures and promotes to knight");
}

// ---------------------------------------------------------------- MoveGenerator

static Square sq(const std::string& name) {
    return *Square::squares_from_fen(name);
}

static std::optional<Board> load(const std::string& fen, const std::string& label) {
    std::optional<Board> b = Board::read_all_fen(fen);
    check(b.has_value(), label + " (FEN reads)");
    return b;
}

// Counts every move sequence of the given length. Known answers catch any bug.
static long long perft(const Board& board, int depth) {
    if (depth == 0) return 1;
    std::vector<Move> moves = MoveGenerator::legal_moves(board);
    if (depth == 1) return static_cast<long long>(moves.size());
    long long nodes = 0;
    for (const Move& m : moves) {
        Board copy = board;
        copy.apply(m);
        nodes += perft(copy, depth - 1);
    }
    return nodes;
}

static void check_count(const std::string& fen, std::size_t expected, const std::string& label) {
    std::optional<Board> b = load(fen, label);
    if (!b) return;
    std::size_t got = MoveGenerator::legal_moves(*b).size();
    check_eq(std::to_string(got), std::to_string(expected), label);
}

static void check_perft(const std::string& fen, int depth, long long expected, const std::string& label) {
    std::optional<Board> b = load(fen, label);
    if (!b) return;
    check_eq(std::to_string(perft(*b, depth)), std::to_string(expected), label);
}

static void test_is_attacked() {
    std::optional<Board> start = load(START_FEN, "is_attacked start");
    if (start) {
        check(MoveGenerator::is_attacked(*start, sq("e3"), Piece::WHITE), "start: e3 attacked by white pawns");
        check(MoveGenerator::is_attacked(*start, sq("f3"), Piece::WHITE), "start: f3 attacked by white");
        check(MoveGenerator::is_attacked(*start, sq("a3"), Piece::WHITE), "start: a3 attacked by the b1 knight");
        check(!MoveGenerator::is_attacked(*start, sq("e5"), Piece::WHITE), "start: e5 not attacked by white");
        check(MoveGenerator::is_attacked(*start, sq("e6"), Piece::BLACK), "start: e6 attacked by black pawns");
        check(!MoveGenerator::is_attacked(*start, sq("e4"), Piece::BLACK), "start: e4 not attacked by black");
    }

    std::optional<Board> rook = load("8/8/8/8/8/8/8/R6k w - - 0 1", "is_attacked rook");
    if (rook) check(MoveGenerator::is_attacked(*rook, sq("h1"), Piece::WHITE), "a1 rook attacks h1");

    std::optional<Board> blocked = load("8/8/8/8/8/8/8/R2P3k w - - 0 1", "is_attacked blocked");
    if (blocked) check(!MoveGenerator::is_attacked(*blocked, sq("h1"), Piece::WHITE), "d1 pawn blocks the rook");

    std::optional<Board> bishop = load("8/8/8/8/8/8/8/B6k w - - 0 1", "is_attacked bishop");
    if (bishop) check(MoveGenerator::is_attacked(*bishop, sq("h8"), Piece::WHITE), "a1 bishop attacks h8");

    std::optional<Board> king = load("8/8/8/8/8/8/1K6/7k w - - 0 1", "is_attacked king");
    if (king) check(MoveGenerator::is_attacked(*king, sq("c3"), Piece::WHITE), "b2 king attacks c3");
}

static void test_in_check() {
    std::optional<Board> start = load(START_FEN, "in_check start");
    if (start) {
        check(!MoveGenerator::in_check(*start, Piece::WHITE), "start: white not in check");
        check(!MoveGenerator::in_check(*start, Piece::BLACK), "start: black not in check");
    }
    std::optional<Board> checked = load("4k3/8/8/8/8/8/8/4R1K1 b - - 0 1", "in_check rook");
    if (checked) check(MoveGenerator::in_check(*checked, Piece::BLACK), "e1 rook checks e8 king");

    std::optional<Board> shielded = load("4k3/4p3/8/8/8/8/8/4R1K1 b - - 0 1", "in_check blocked");
    if (shielded) check(!MoveGenerator::in_check(*shielded, Piece::BLACK), "e7 pawn blocks the check");

    std::optional<Board> knight = load("4k3/8/3N4/8/8/8/8/6K1 b - - 0 1", "in_check knight");
    if (knight) check(MoveGenerator::in_check(*knight, Piece::BLACK), "d6 knight checks e8 king");
}

static void test_legal_moves() {
    check_count(START_FEN, 20, "start: 20 legal moves");
    check_count("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1", 20, "after e4: black has 20");
    check_count("4k3/8/8/8/8/8/8/4R1K1 b - - 0 1", 4, "in check: 4 king moves");
    check_count("4k3/4r3/8/8/8/8/8/4R1K1 b - - 0 1", 10, "pinned rook: 6 rook + 4 king");
    check_count("8/8/8/3Q4/8/8/8/k6K w - - 0 1", 29, "queen d5 (26, own king blocks h1) + king h1 (3)");
    check_count("8/P7/8/8/8/8/8/k6K w - - 0 1", 7, "a7 pawn promotes 4 ways + king 3");
    check_count("R6k/8/7K/8/8/8/8/8 b - - 0 1", 0, "checkmate: black has no moves");
    check_count("7k/5Q2/6K1/8/8/8/8/8 b - - 0 1", 0, "stalemate: black has no moves");

    std::optional<Board> start = load(START_FEN, "is_legal start");
    if (start) {
        check(MoveGenerator::is_legal(*start, *Move::from_uci("e2e4")), "start: e2e4 legal");
        check(MoveGenerator::is_legal(*start, *Move::from_uci("g1f3")), "start: g1f3 legal");
        check(!MoveGenerator::is_legal(*start, *Move::from_uci("e2e5")), "start: e2e5 illegal");
        check(!MoveGenerator::is_legal(*start, *Move::from_uci("e1e2")), "start: e1e2 illegal");
        check(!MoveGenerator::is_legal(*start, *Move::from_uci("a1a3")), "start: a1a3 illegal");
        check(!MoveGenerator::is_legal(*start, *Move::from_uci("e7e5")), "start: black pawn can't move on white's turn");
    }

    std::optional<Board> pinned = load("4k3/4r3/8/8/8/8/8/4R1K1 b - - 0 1", "is_legal pinned");
    if (pinned) {
        check(MoveGenerator::is_legal(*pinned, *Move::from_uci("e7e1")), "pinned rook may capture e1");
        check(!MoveGenerator::is_legal(*pinned, *Move::from_uci("e7a7")), "pinned rook can't leave the file");
    }

    std::optional<Board> ep = load("rnbqkb1r/ppp1pppp/5n2/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 3", "is_legal en passant");
    if (ep) {
        check(MoveGenerator::is_legal(*ep, *Move::from_uci("e5d6")), "en passant e5d6 legal");
        check(MoveGenerator::is_legal(*ep, *Move::from_uci("e5f6")), "e5f6 captures the knight");
    }
}

static void test_perft() {
    // No castling is possible this early, so these pass before castling exists
    check_perft(START_FEN, 1, 20, "perft start depth 1");
    check_perft(START_FEN, 2, 400, "perft start depth 2");
    check_perft(START_FEN, 3, 8902, "perft start depth 3");
    check_perft(START_FEN, 4, 197281, "perft start depth 4");

    // Kiwipete exercises castling, en passant and promotion.
    // Depth 1 includes 2 castling moves, so expect 46 until castling is added.
    const std::string kiwipete = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    check_perft(kiwipete, 1, 48, "perft kiwipete depth 1 (needs castling)");
    check_perft(kiwipete, 2, 2039, "perft kiwipete depth 2 (needs castling)");
}

// ---------------------------------------------------------------- Game

// Plays each move in `moves` (space separated); returns false if any is rejected
static bool play(Game& game, const std::string& moves) {
    std::stringstream ss(moves);
    std::string uci;
    while (ss >> uci) {
        if (!game.try_move(uci)) return false;
    }
    return true;
}

static void test_game() {
    // A new game is in progress
    {
        Game game(START_FEN);
        check(game.status() == Game::IN_PROGRESS, "new game is in progress");
        check_eq(game.to_fen(), START_FEN, "new game starts at the start position");
        check(game.history_uci().empty(), "new game has no history");
    }

    // A bad FEN throws
    {
        bool threw = false;
        try {
            Game game("not a fen");
        } catch (const std::invalid_argument&) {
            threw = true;
        }
        check(threw, "bad FEN throws invalid_argument");
    }

    // Legal moves are accepted and recorded
    {
        Game game(START_FEN);
        check(play(game, "e2e4 e7e5 g1f3"), "e2e4 e7e5 g1f3 accepted");
        std::vector<std::string> h = game.history_uci();
        check(h.size() == 3 && h[0] == "e2e4" && h[2] == "g1f3", "history records the moves in order");
        check(game.status() == Game::IN_PROGRESS, "still in progress after 3 moves");
    }

    // Illegal and malformed moves are rejected and change nothing
    {
        Game game(START_FEN);
        check(!game.try_move("e2e5"), "e2e5 rejected");
        check(!game.try_move("e7e5"), "black move on white's turn rejected");
        check(!game.try_move("zzzz"), "malformed text rejected");
        check_eq(game.to_fen(), START_FEN, "board unchanged after rejected moves");
        check(game.history_uci().empty(), "history unchanged after rejected moves");
    }

    // Fool's mate: black checkmates white
    {
        Game game(START_FEN);
        check(play(game, "f2f3 e7e5 g2g4 d8h4"), "fool's mate moves accepted");
        check(game.status() == Game::BLACK_WINS, "fool's mate: black wins");
        check(!game.try_move("a2a3"), "no moves accepted after the game ends");
    }

    // Scholar's mate: white checkmates black
    {
        Game game(START_FEN);
        check(play(game, "e2e4 e7e5 f1c4 b8c6 d1h5 g8f6 h5f7"), "scholar's mate moves accepted");
        check(game.status() == Game::WHITE_WINS, "scholar's mate: white wins");
    }

    // Stalemate
    {
        Game game("7k/8/6K1/8/8/8/8/5Q2 w - - 0 1");
        check(game.try_move("f1f7"), "f1f7 accepted");
        check(game.status() == Game::DRAW, "stalemate is a draw");
    }

    // Threefold repetition
    {
        Game game(START_FEN);
        check(play(game, "g1f3 g8f6 f3g1 f6g8"), "first shuffle accepted");
        check(game.status() == Game::IN_PROGRESS, "start position seen twice: still in progress");
        check(play(game, "g1f3 g8f6 f3g1 f6g8"), "second shuffle accepted");
        check(game.status() == Game::DRAW, "start position seen three times: draw");
    }

    // 50-move rule: halfmove already at 99, one quiet move makes 100
    {
        Game game("4k3/8/8/8/8/8/8/4K2R w - - 99 80");
        check(game.try_move("h1h2"), "quiet rook move accepted");
        check(game.status() == Game::DRAW, "halfmove 100: draw");
    }

    // Starting from a position that is already checkmate
    {
        Game game("R6k/8/7K/8/8/8/8/8 b - - 0 1");
        check(game.status() == Game::WHITE_WINS, "game created in checkmate is already over");
    }
}

// ---------------------------------------------------------------- Show one move

static void show_move(const std::string& text) {
    std::optional<Move> m = Move::from_uci(text);
    std::cout << "from_uci(\"" << text << "\"): ";
    if (!m) {
        std::cout << "empty\n";
        return;
    }
    std::cout << "from " << m->from.squares_to_fen()
              << ", to " << m->to.squares_to_fen();
    if (m->promotion_piece) {
        std::cout << ", promotes to "
                  << Piece::piece_to_fen(*m->promotion_piece, Piece::BLACK);
    }
    std::cout << " | to_uci(): " << m->to_uci() << '\n';
}

int main() {
    test_piece();
    test_square();
    test_move();
    test_board_fen();
    test_board_apply();
    test_is_attacked();
    test_in_check();
    test_legal_moves();
    test_perft();
    test_game();

    std::cout << '\n';
    show_move("e2e4");
    show_move("e7e8q");
    show_move("e7e8k");

    std::cout << '\n' << passed << " passed, " << failed << " failed\n";
    return failed == 0 ? 0 : 1;
}
