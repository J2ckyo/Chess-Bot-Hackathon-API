#include "Piece.hpp"
#include <iostream>

int main(){

    Piece piece;

    char ex = piece.piece_to_fen(Piece::QUEEN, Piece::BLACK);
    std::cout << ex;

    return 0;
};  