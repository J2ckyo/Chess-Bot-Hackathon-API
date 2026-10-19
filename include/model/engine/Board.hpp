/**
 * @author Jack. Simonton <jack.simonton@uleth.ca>
 * @date Fall 2026
 */

#ifndef BOARD_HPP
#define BOARD_HPP

#include <vector>
#include <array>
#include <optional>
#include <string>
#include <cctype>
#include <sstream>
#include <cmath>
#include "Piece.hpp"
#include "Square.hpp"
#include "Move.hpp"

class Board{
    public:
        /**
         * @brief Returns what's on that square, everything uses this
         * 
         * @param square
         * @return std::optional<Piece> 
         */
        std::optional<Piece> piece_at( const Square& square) const;

        /**
         * @brief Puts a piece on a square or empties it. 
         * 
         * @param square 
         * @param piece 
         */
        void place(const Square& square, std::optional<Piece> piece);

        /**
         * @brief Splits FEN Json data into 6 fields
         * 
         * @param fen_json 
         * @return std::optional<Board> 
         */
        static std::optional<Board> read_all_fen(std::string fen_json);

        /**
         * @brief Does the reverse, translates board data into FEN
         * 
         * 
         * @param board 
         * @return std::string 
         */
        std::string all_to_fen() const;

        /**
         * @brief Applies the move in two ways
         * Noramal move - 
         * Special move - 
         * @param move 
         */
        void apply(const Move& move);

        /**
         * @brief Getter function for MoveGenerator
         * @return Piece::Colour
         */
        Piece::Colour get_side_to_move() const {return side_to_move;}

        /**
         * @brief Getter function for MoveGenerator
         * @return Castling String
         */
        std::string get_castling() const {return castling;}

        /**
         * @brief Getter function for MoveGenerator
         * @return En-passant
         */
        std::optional<Square> get_en_passant() const {return enp_square;}

    private:
        
        /**
         * @brief 8x8 Array of type optional<Piece>
         * 
         */
        std::optional<Piece> board[8][8];

        /**
         * @brief Which colour is the side to move
         * type Colour
         * 
         */
        Piece::Colour side_to_move;

        /**
         * @brief Holds a string like "KQkq" or "-"
         * 
         */
        std::string castling;

        /**
         * @brief Enpassant
         * 
         */
        std::optional<Square> enp_square;

        /**
         * @brief Keeps track of halfmoves
         * 
         */
        int halfmove = 0;

        /**
         * @brief Keeps track of full moves
         * 
         */
        int fullmove = 1;

};

#endif // BOARD_HPP
 