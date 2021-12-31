#include "Hand.h"
#include <locale>
#include <iostream>
#include <cassert>

namespace egn {

Hand::Hand(const std::string& handStr)
{
    if (handStr == "")
        *this = Hand::empty();

    else {
        // Ignore whitespaces and turn to lowercase.
        std::string s;
        for (char c : handStr) {
            if (std::isgraph(c))
                s += std::tolower(c);
        }

        *this = Hand(getIdx(s.substr(0, 2)));
        for (size_t i = 2; i + 1 < s.size(); i += 2)
            *this += Hand(getIdx(s.substr(i, 2)));
    }
}

std::string Hand::getStr() const
{
    std::string s;
    uint64_t handMask = mask();
    uint64_t cardMask;
    unsigned rank, suit;
    for (unsigned card = 0; card < omp::CARD_COUNT; ++card) {
        rank = card / omp::SUIT_COUNT, suit = card % omp::SUIT_COUNT;
        // cardMask as defined in omp::HandEvaluator::initCardConstants
        cardMask = 1ull << ((3 - suit) * 16 + rank);
        if (handMask & cardMask) {
            if (s.size() > 0)
                s.push_back(' ');
            s.push_back(intToRank(rank));
            s.push_back(intToSuit(suit));
        }
    }
    return s;
}

uint8_t Hand::countCards() const
{
    uint8_t count = 0;
    uint64_t handMask = mask();
    uint64_t cardMask;
    unsigned rank, suit;
    for (unsigned card = 0; card < omp::CARD_COUNT; ++card) {
        rank = card / omp::SUIT_COUNT, suit = card % omp::SUIT_COUNT;
        // cardMask as defined in omp::HandEvaluator::initCardConstants
        cardMask = 1ull << ((3 - suit) * 16 + rank);
        if (handMask & cardMask) ++count;
    }
    return count;
}

unsigned Hand::getIdx(const std::string& cardStr)
{
    unsigned rank = charToRank(cardStr[0]);
    unsigned suit = charToSuit(cardStr[1]);
    return omp::SUIT_COUNT * rank + suit;
}

unsigned Hand::charToRank(char c)
{
    switch (c) {
    case 'a': return 12;
    case 'k': return 11;
    case 'q': return 10;
    case 'j': return 9;
    case 't': return 8;
    case '9': return 7;
    case '8': return 6;
    case '7': return 5;
    case '6': return 4;
    case '5': return 3;
    case '4': return 2;
    case '3': return 1;
    case '2': return 0;
    default: return ~0u;
    }
}

unsigned Hand::charToSuit(char c)
{
    switch (c) {
    case 's': return 0;
    case 'h': return 1;
    case 'c': return 2;
    case 'd': return 3;
    default: return ~0u;
    }
}

char Hand::intToRank(unsigned idx)
{
    switch (idx) {
    case 12: return 'a';
    case 11: return 'k';
    case 10: return 'q';
    case 9: return 'j';
    case 8: return 't';
    case 7: return '9';
    case 6: return '8';
    case 5: return '7';
    case 4: return '6';
    case 3: return '5';
    case 2: return '4';
    case 1: return '3';
    case 0: return '2';
    default: return ' ';
    }
}

char Hand::intToSuit(unsigned idx)
{
    switch (idx) {
    case 0: return 's';
    case 1: return 'h';
    case 2: return 'c';
    case 3: return 'd';
    default: return ' ';
    }
}

std::ostream& operator<<(std::ostream& os, const Hand& hand)
{
    return os << hand.getStr();
}

} // egn