/**
 * @author Jack. Simonton <jack.simonton@uleth.ca>
 * @date Fall 2026
 */

#ifndef SQUARE_HPP
#define SQUARE_HPP

#include <optional>
#include <string>

/**
 * @class Square Class
 * @brief Used for square translation betweem model and json
 * i.e e2e4 into two squares: board[1][4]
 */

class Square{
    public:

        /**
         * @brief Translate from FEN to board index's
         */
        static std::optional<Square> squares_from_fen(const std::string& pos);

        /**
         * @brief Translate board index's to FEN code
         */
        std::string squares_to_fen() const;

        /**
         * @brief Takes a rank and file and returns a bool
         */
        static bool on_board(int rank, int file);

        /**
         * @brief Comparison Operator compares rank and file
         */
        friend bool operator==(const Square& sq1, const Square& sq2);

        int file;
        int rank;
};

#endif
