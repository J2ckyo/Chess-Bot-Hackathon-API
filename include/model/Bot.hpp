/**
 * @author Jack. Simonton <jack.simonton@uleth.ca>
 * @date Fall 2026
 */

#ifndef BOT_HPP
#define BOT_HPP

#include <string>

class Bot {
    public:
        /**
         * @brief Public identifier 
         * 
         */   
        std::string id;

        /**
         * @brief Display name
         */
        std::string name;

        /**
         * @brief scrambled API key
         */
        std::string key_hash;
};



#endif
