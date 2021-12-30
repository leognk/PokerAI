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
    Hand(const std::array<uint8_t, nCards>& handArr);

    std::string getStr() const;
    template<unsigned nCards>
    std::array<uint8_t, nCards> getArr() const;

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