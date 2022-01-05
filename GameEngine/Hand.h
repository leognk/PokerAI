#ifndef EGN_HAND_H
#define EGN_HAND_H

#include "../OMPEval/omp/Hand.h"
#include <string>

namespace egn {

// Structure that adds useful features to the omp::Hand structure:
// constructing a hand from the string representation of a card,
// getting the string representation of a hand.
struct Hand :
    public omp::Hand
{
public:
    Hand() : omp::Hand() {}
    // Allows conversion from omp::Hand to egn::Hand.
    Hand(const omp::Hand& hand) : omp::Hand(hand) {}
    Hand(const std::string& handStr);

    template<unsigned nCards>
    Hand(const std::array<uint8_t, nCards>& handArr)
    {
        if (nCards == 0)
            *this = Hand::empty();
        else {
            *this = Hand(handArr[0]);
            for (uint8_t i = 1; i < nCards; ++i)
                *this += Hand(handArr[i]);
        }
    };

    Hand(const uint8_t handArr[], uint8_t nCards)
    {
        if (nCards == 0)
            *this = Hand::empty();
        else {
            *this = Hand(handArr[0]);
            for (uint8_t i = 1; i < nCards; ++i)
                *this += Hand(handArr[i]);
        }
    }

    std::string getStr() const;

    template<unsigned nCards>
    std::array<uint8_t, nCards> getArr() const
    {
        std::array<uint8_t, nCards> a{};
        uint8_t cardCount = 0;
        uint64_t handMask = mask();
        uint64_t cardMask;
        unsigned rank, suit;
        for (unsigned card = 0; card < omp::CARD_COUNT; ++card) {
            rank = card / omp::SUIT_COUNT, suit = card % omp::SUIT_COUNT;
            // cardMask as defined in omp::HandEvaluator::initCardConstants
            cardMask = 1ull << ((3 - suit) * 16 + rank);
            if (handMask & cardMask) {
                a[cardCount++] = card;
                if (cardCount == nCards) return a;
            }
        }
        return a;
    };

    uint8_t countCards() const;

    template<uint8_t nCards>
    static std::array<uint8_t, nCards> stringToArray(const std::string& handStr)
    {
        // Ignore whitespaces and turn to lowercase.
        std::string s;
        for (char c : handStr) {
            if (std::isgraph(c))
                s += std::tolower(c);
        }
        std::array<uint8_t, nCards> handArr{};
        for (size_t i = 0; i + 1 < s.size(); i += 2)
            handArr[i / 2] = getIdx(s.substr(i, 2));
        return handArr;
    }

private:
    static unsigned getIdx(const std::string& cardStr);
    static unsigned charToRank(char c);
    static unsigned charToSuit(char c);
    static char intToRank(unsigned idx);
    static char intToSuit(unsigned idx);
};

std::ostream& operator<<(std::ostream& os, const Hand& hand);

} // egn

#endif // EGN_HAND_H