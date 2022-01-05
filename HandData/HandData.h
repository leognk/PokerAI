#ifndef HANDDATA_H
#define HANDDATA_H

#include "../GameEngine/GameState.h"
#include <vector>

namespace hdt {

static const std::string projectPath = "C:/Users/leota/Desktop/Info/Poker/My projets/Optimus/";
static const std::string handDataRoot = projectPath + "data/HandData/BulkHands-14489";
static const std::string compressedHandDataFile = projectPath + "data/HandData/BulkHands-14489.txt";

#pragma warning(push)
#pragma warning(disable: 26495)
struct ForcedBets
{
    std::vector<egn::chips> antes;
    uint8_t sbPlayer, bbPlayer;
    egn::chips sb, bb;
};

struct Action
{
    uint8_t player;
    egn::Action action;
    egn::chips bet;
};

struct HandHistory
{
    uint8_t maxPlayers;
    egn::chips ante, sb, bb;
    uint8_t dealer;
    std::vector<egn::chips> initialStakes;

    egn::Hand boardCards;
    std::vector<egn::Hand> hands;

    ForcedBets forcedBets;
    std::vector<std::vector<Action>> actions;
    std::vector<egn::dchips> rewards;
    std::vector<bool> collectedPot;
    egn::chips rake;
};
#pragma warning(pop)

std::string extractInfo(
    const std::string& text, const std::string& pattern, int matchIdx);

std::istream& operator>>(std::istream& is, HandHistory& hist);
std::ostream& operator<<(std::ostream& os, const HandHistory& hist);
std::ostream& writeCompressedData(std::ostream& os, const HandHistory& hist);
std::istream& readCompressedData(std::istream& is, HandHistory& hist);

} // hdt

#endif // HANDDATA_H