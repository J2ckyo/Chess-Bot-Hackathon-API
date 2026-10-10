#ifndef TYPES_HPP
#define TYPES_HPP

/**
 * @brief Shared
 */
enum class Colour {White, Black};
enum class PieceKind {Pawn, Knight, Bishop, Rook, Queen, King};

struct Position {
    int row = 0;
    int col = 0;
        };

#endif // TYPES_HPP