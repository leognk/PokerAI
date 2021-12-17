#include "../GameEngine/GameState.h"

namespace hdt {

static const std::string projectPath = "C:/Users/leota/Desktop/Info/Poker/My projets/Optimus";
static const std::string handDataRoot = projectPath + "/data/BulkHands-14489";
static const std::string compressedHandDataFile = projectPath + "/data/BulkHands-14489.txt";

#pragma warning(push)
#pragma warning(disable: 26495)
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
#pragma warning(pop)

std::string extractInfo(
    const std::string& text, const std::string& pattern,
    int matchIdx, bool findSecondMatch = false);

std::istream& operator>>(std::istream& is, HandHistory& hist);
std::ostream& operator<<(std::ostream& os, const HandHistory& hist);
std::ostream& writeCompressedData(std::ostream& os, const HandHistory& hist);
std::istream& readCompressedData(std::istream& is, HandHistory& hist);

} // hdt