/**
 * Tests for Piece, Square, and Move.
 * Run with: make run
 * Prints only failures, then a summary.
 */

#include <iostream>
#include <optional>
#include <string>

#include "Piece.hpp"
#include "Square.hpp"
#include "Move.hpp"
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

    std::cout << '\n';
    show_move("e2e4");
    show_move("e7e8q");
    show_move("e7e8k");

    std::cout << '\n' << passed << " passed, " << failed << " failed\n";
    return failed == 0 ? 0 : 1;
}
