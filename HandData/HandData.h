#include "../GameEngine/GameState.h"

static const std::string handDataRoot = "../data/BulkHands-14489";
static const std::string compressedHandDataFile = "../data/BulkHands-14489.txt";

struct HandHistory
{
    uint8_t maxPlayers;
    uint32_t ante, sb, bb;
    uint8_t dealer;
    std::vector<uint32_t> initialStakes;

    egn::Hand boardCards;
    std::vector<egn::Hand> hands;

    ForcedBets forcedBets;
    std::vector<std::vector<Action>> actions;
    std::vector<int64_t> rewards;
    uint32_t rake;
};

struct ForcedBets
{
    std::vector<uint32_t> antes;
    uint8_t sbPlayer, bbPlayer;
    uint32_t sb, bb;
};

struct Action
{
    uint8_t player;
    egn::Action action;
    uint32_t bet;
};

std::string extractInfo(
    const std::string& text, const std::string& pattern,
    int matchIdx, bool findSecondMatch = false);

std::istream& operator>>(std::istream& is, HandHistory& hist);
std::ostream& operator<<(std::ostream& os, const HandHistory& hist);
std::ostream& writeCompressedData(std::ostream& os, const HandHistory& hist);
std::istream& readCompressedData(std::istream& is, HandHistory& hist);