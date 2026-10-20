/**
 * @author Jack Simonton <Jack.simonton@uleth.ca>
 * @date Fall 2026
 */

 #include "Square.hpp"
 #include <string>

std::optional<Square> Square::squares_from_fen(const std::string& pos){

    // Checks if pos is 2 chars
    if (pos.length() != 2){
        return std::nullopt;
    }

    // Gets values at index's
    int file_pos = pos[0];
    int rank_pos = pos[1];

    // Checks if values are in range.
    if (file_pos < 'a' || file_pos > 'h' || rank_pos < '1' || rank_pos > '8'){
        return std::nullopt;
    }

    // Computes index for file and rank on the board
    int file = file_pos - 'a';
    int rank = rank_pos - '1';

    return Square{file, rank};
}

std::string Square::squares_to_fen() const{
    char file_letter = 'a' + file;
    char rank_letter = '1' + rank;
    std::string fen;
    fen.push_back(file_letter);
    fen.push_back(rank_letter);

    return fen;
}

bool Square::on_board(int file, int rank){
    if (file < 0 || file > 7 || rank < 0 || rank > 7){
        return false;
    }
    return true;
}


bool operator==(const Square& sq1, const Square& sq2){
    return sq1.rank == sq2.rank && sq1.file == sq2.file;
}

