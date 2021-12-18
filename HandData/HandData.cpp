#include <boost/regex/icu.hpp>
#include <json/json.h>
#include "HandData.h"

namespace hdt {

std::string extractInfo(
    const std::string& text, const std::string& pattern,
    int matchIdx, bool findSecondMatch)
{
    boost::u32regex pat = boost::make_u32regex(pattern);
    boost::smatch matches;
    if (boost::u32regex_search(text, matches, pat)) {
        if (!findSecondMatch)
            return matches.str(matchIdx);
        return extractInfo(matches.suffix().str(),
            pattern, matchIdx);
    }
    else
        return "";
}

#pragma warning(push)
#pragma warning(disable: 4244)
std::istream& operator>>(std::istream& is, HandHistory& hist)
{
    // Go to the beginning of the next hand history.
    std::string line;
    while (std::getline(is, line)
        && !extractInfo(line, R"(PokerStars Hand #\d+:  Hold'em No Limit)", 0).size())
        line = "";

    if (is.eof())
        return is;

    // Read currency.
    std::string currency = "\\" + extractInfo(line, R"(\((.)\d)", 1);

    hist.ante = 0;

    // Read small blind.
    std::string sb = extractInfo(
        line, currency + R"(((\d|\.)+)\/)", 1);
    hist.sb = std::round(100 * std::stod(sb));

    // Read big blind.
    std::string bb = extractInfo(
        line, R"(\/)" + currency + R"(((\d|\.)+))", 1);
    hist.bb = std::round(100 * std::stod(bb));

    line = "";
    std::getline(is, line);

    // Read maxPlayers.
    std::string maxPlayers = extractInfo(line, R"((\d+)-max)", 1);
    hist.maxPlayers = std::stoi(maxPlayers);

    // Read dealer.
    std::string dealer = extractInfo(line, R"(Seat #(\d+) is the button)", 1);
    hist.dealer = std::stoi(dealer) - 1;

    line = "";
    std::getline(is, line);

    // Read players names and stakes.
    std::unordered_map<std::string, uint8_t> nameToId;
    hist.initialStakes = std::vector<egn::chips>(hist.maxPlayers, 0);
    while (extractInfo(line, R"(Seat \d+: )", 0).size()) {

        // Read seat number.
        uint8_t id = std::stoul(extractInfo(line, R"(Seat (\d+): )", 1)) - 1;

        // Read player's name.
        std::string playerName = extractInfo(line, R"(Seat \d+: (.+) \()", 1);
        nameToId[playerName] = id;

        // Read player's stake.
        std::string stake = extractInfo(line, currency + R"(((\d|\.)+))", 1);
        hist.initialStakes[id] = std::round(100 * std::stod(stake));

        line = "";
        std::getline(is, line);
    }

    std::vector<egn::chips> stakes(hist.initialStakes);
    hist.forcedBets.antes = std::vector<egn::chips>(hist.maxPlayers, 0);

    // Preflop starts.
    hist.actions.push_back({});
    std::vector<egn::chips> roundBets(hist.maxPlayers, 0);

    // Read forced bets.
    uint8_t nPostedBlinds = 0;
    while (line != "*** HOLE CARDS ***") {

        // Read sb.
        if (extractInfo(line, ": posts small blind", 0).size()) {
            // Read sb player.
            std::string sbPlayer = extractInfo(line, R"(([^:]+):)", 1);
            hist.forcedBets.sbPlayer = nameToId[sbPlayer];
            // Read sb.
            std::string postedSb = extractInfo(line, currency + R"(((\d|\.)+))", 1);
            hist.forcedBets.sb = std::round(100 * std::stod(postedSb));
            roundBets[hist.forcedBets.sbPlayer] += hist.forcedBets.sb;
            stakes[hist.forcedBets.sbPlayer] -= hist.forcedBets.sb;
            ++nPostedBlinds;
        }

        // Read bb.
        else if (extractInfo(line, ": posts big blind", 0).size()) {
            // Read bb player.
            std::string bbPlayer = extractInfo(line, R"(([^:]+):)", 1);
            hist.forcedBets.bbPlayer = nameToId[bbPlayer];
            // Read bb.
            std::string postedBb = extractInfo(line, currency + R"(((\d|\.)+))", 1);
            hist.forcedBets.bb = std::round(100 * std::stod(postedBb));
            roundBets[hist.forcedBets.bbPlayer] += hist.forcedBets.bb;
            stakes[hist.forcedBets.bbPlayer] -= hist.forcedBets.bb;
            ++nPostedBlinds;
        }

        // Bizarre case of a player who posts both sb & bb (I haven't heard of it).
        else if (extractInfo(line, " posts small & big blinds", 0).size()) {
            is.clear(std::ios_base::failbit);
            return is;
        }

        else if (!extractInfo(line, ": is sitting out", 0).size()
            && !extractInfo(line, ": sits out", 0).size()
            && !extractInfo(line, " has timed out", 0).size()
            && !extractInfo(line, " leaves the table", 0).size()
            && !extractInfo(line, " joins the table ", 0).size()
            && !extractInfo(line, " said, ", 0).size()
            && !extractInfo(line, " is connected", 0).size()
            && !extractInfo(line, " is disconnected", 0).size()
            && !extractInfo(line, " will be allowed to play after the button", 0).size()
            && !extractInfo(line, " was removed from the table for failing to post", 0).size())
            throw std::runtime_error("Found unknown case while reading forced bets.");

        // Bizarre case of multiple big blinds posted (I haven't heard of it).
        if (nPostedBlinds > 2) {
            is.clear(std::ios_base::failbit);
            return is;
        }

        line = "";
        std::getline(is, line);
    }

    hist.hands = std::vector(hist.maxPlayers, egn::Hand(egn::Hand::empty()));
    hist.collectedPot = std::vector(hist.maxPlayers, false);

    // Read actions.
    while (line != "*** SUMMARY ***") {

        // A new round begins.
        if (extractInfo(line,
            R"(\*\*\* (FLOP|TURN|RIVER) \*\*\*)", 0).size()) {
            hist.actions.push_back({});
            roundBets = std::vector<egn::chips>(hist.maxPlayers, 0);
        }

        // Read action.
        else if (extractInfo(line,
            R"(: (folds|checks|calls|bets|raises))", 0).size()) {

            Action action;

            // Read player.
            std::string playerName = extractInfo(line, R"(([^:]+):)", 1);
            action.player = nameToId[playerName];

            // Read action type.
            std::string actionStr = extractInfo(line, R"(: (\S+))", 1);
            // All-in
            if (extractInfo(line, " is all-in", 0).size())
                action.action = egn::Action::allin;
            // Raise
            else if (actionStr == "bets" || actionStr == "raises")
                action.action = egn::Action::raise;
            // Fold
            else if (actionStr == "folds")
                action.action = egn::Action::fold;
            // Call
            else if (actionStr == "checks" || actionStr == "calls")
                action.action = egn::Action::call;
            else
                throw std::runtime_error("Found unknown action type: " + actionStr);

            // Read bet value.
            // Null bet.
            if (actionStr == "folds" || actionStr == "checks")
                action.bet = 0;
            // Non-zero bet.
            else {
                std::string bet = extractInfo(
                    line, currency + R"(((\d|\.)+))", 1,
                    actionStr == "raises");
                action.bet = std::round(100 * std::stod(bet));
                // Get the number of chips added to the pot for a raise.
                if (actionStr == "raises")
                    action.bet -= roundBets[action.player];
            }

            roundBets[action.player] += action.bet;
            stakes[action.player] -= action.bet;

            hist.actions.back().emplace_back(action);
        }

        // Read uncalled bet.
        else if (extractInfo(line, "returned to ", 0).size()) {
            std::string player = extractInfo(line, R"(returned to (.+))", 1);
            std::string cash = extractInfo(
                line, R"(\()" + currency + R"(((\d|\.)+)\))", 1);
            stakes[nameToId[player]] += std::round(100 * std::stod(cash));
        }

        // Read reward.
        else if (extractInfo(line, " collected", 0).size()) {
            std::string player = extractInfo(line, R"((.+) collected)", 1);
            std::string reward = extractInfo(line, currency + R"(((\d|\.)+))", 1);
            stakes[nameToId[player]] += std::round(100 * std::stod(reward));
            hist.collectedPot[nameToId[player]] = true;
        }

        // Read hole cards.
        else if (extractInfo(line, R"(: shows \[.. ..\])", 0).size()) {
            std::string player_name = extractInfo(line, R"(([^:]+): shows)", 1);
            std::string cards = extractInfo(line, R"(: shows \[(.. ..)\])", 1);
            hist.hands[nameToId[player_name]] = egn::Hand(cards);
        }

        // Bizarre case of two hands dealt (I haven't heard of it).
        else if (extractInfo(line, R"(\*\*\* FIRST (FLOP|TURN|RIVER) \*\*\*)", 0).size()) {
            is.clear(std::ios_base::failbit);
            return is;
        }

        else if (line != "*** HOLE CARDS ***"
            && line != "*** SHOW DOWN ***"
            && !extractInfo(line, ": doesn't show hand", 0).size()
            && !extractInfo(line, " joins the table", 0).size()
            && !extractInfo(line, ": mucks hand", 0).size()
            && !extractInfo(line, " leaves the table", 0).size()
            && !extractInfo(line, " has timed out", 0).size()
            && !extractInfo(line, " was removed from the table for failing to post", 0).size()
            && !extractInfo(line, " cashed out the hand for ", 0).size()
            && !extractInfo(line, " is disconnected", 0).size()
            && !extractInfo(line, " said, ", 0).size()
            && !extractInfo(line, " is connected", 0).size()
            && !extractInfo(line, R"(: shows \[..\])", 0).size())
            throw std::runtime_error("Found unknown case while reading action.");

        line = "";
        std::getline(is, line);
    }

    line = "";
    std::getline(is, line);

    // Read rake.
    std::string rake = extractInfo(line, "Rake " + currency + R"(((\d|\.)+))", 1);
    hist.rake = std::round(100 * std::stod(rake));

    line = "";
    std::getline(is, line);

    // Read board cards.
    hist.boardCards = egn::Hand::empty();
    if (extractInfo(line, "Board ", 0).size()) {
        std::string cards = extractInfo(line, R"(Board \[(.+)\])", 1);
        hist.boardCards += egn::Hand(cards);
    }

    // Compute rewards.
    hist.rewards = std::vector<egn::dchips>(hist.maxPlayers, 0);
    for (auto& it : nameToId)
        hist.rewards[it.second] = (egn::dchips(stakes[it.second])
            - egn::dchips(hist.initialStakes[it.second]));

    return is;
}
#pragma warning(pop)

std::ostream& operator<<(std::ostream& os, const HandHistory& hist)
{
    // Print general info.
    os << "Blinds: " << hist.sb << "/" << hist.bb << "\n"
        << "Max players: " << int(hist.maxPlayers) << "\n"
        << "Dealer: " << std::to_string(hist.dealer) << "\n\n";

    // Print player names and stakes.
    os << "Stakes:\n\n";
    for (uint8_t i = 0; i < hist.maxPlayers; ++i) {
        if (hist.initialStakes[i])
            os << std::to_string(i) << ": " << hist.initialStakes[i] << "\n";
    }
    os << "\n";

    // Print posted forced bets.
    os << "SB: " << std::to_string(hist.forcedBets.sbPlayer)
        << " posted " << hist.forcedBets.sb << "\n";
    os << "BB: " << std::to_string(hist.forcedBets.bbPlayer)
        << " posted " << hist.forcedBets.bb << "\n";

    // Print actions.
    for (size_t i = 0; i < hist.actions.size(); ++i) {
        os << "\n*** " << egn::Round(i) << " ***\n\n";
        for (size_t j = 0; j < hist.actions[i].size(); ++j) {
            Action a = hist.actions[i][j];
            os << std::to_string(a.player) << ": " << a.action;
            if (a.action != egn::Action::fold)
                os << " " << a.bet;
            os << "\n";
        }
    }

    // Print cards.
    if (hist.boardCards != egn::Hand::empty())
        os << "\nBoard cards: " << hist.boardCards << "\n";
    for (uint8_t i = 0; i < hist.maxPlayers; ++i) {
        if (hist.hands[i] != egn::Hand::empty())
            os << std::to_string(i) << ": " << hist.hands[i] << "\n";
    }

    // Print rewards.
    os << "\nRewards:\n\n";
    for (uint8_t i = 0; i < hist.maxPlayers; ++i) {
        if (hist.initialStakes[i]) {
            os << std::to_string(i) << ": " << hist.rewards[i];
            if (hist.collectedPot[i])
                os << " (won)";
            os << "\n";
        }
    }

    // Print rake.
    os << "\nRake: " << hist.rake << "\n";

    return os;
}

std::ostream& writeCompressedData(std::ostream& os, const HandHistory& hist)
{
    Json::Value histJs;

    // Write general info.
    histJs["mxP"] = hist.maxPlayers;
    histJs["at"] = hist.ante;
    histJs["sb"] = hist.sb;
    histJs["bb"] = hist.bb;
    histJs["dl"] = hist.dealer;

    // Write initialStakes.
    Json::Value initialStakesJs(Json::arrayValue);
    for (egn::chips x : hist.initialStakes)
        initialStakesJs.append(Json::Value(x));
    histJs["inS"] = initialStakesJs;

    // Write cards.
    histJs["bd"] = hist.boardCards.getStr();
    Json::Value handsJs(Json::arrayValue);
    for (const egn::Hand& h : hist.hands)
        handsJs.append(Json::Value(h.getStr()));
    histJs["hd"] = handsJs;

    // Write forcedBets.
    Json::Value forcedBetsJs;
    Json::Value antesJs(Json::arrayValue);
    for (egn::chips x : hist.forcedBets.antes)
        antesJs.append(Json::Value(x));
    forcedBetsJs["at"] = antesJs;
    forcedBetsJs["sbP"] = hist.forcedBets.sbPlayer;
    forcedBetsJs["bbP"] = hist.forcedBets.bbPlayer;
    forcedBetsJs["sb"] = hist.forcedBets.sb;
    forcedBetsJs["bb"] = hist.forcedBets.bb;
    histJs["frB"] = forcedBetsJs;

    // Write actions.
    Json::Value actionsJs(Json::arrayValue);
    for (const std::vector<Action>& roundActions : hist.actions) {
        Json::Value roundActionsJs(Json::arrayValue);
        for (const Action& a : roundActions) {
            Json::Value aJs;
            aJs["p"] = a.player;
            aJs["a"] = uint8_t(a.action);
            aJs["b"] = a.bet;
            roundActionsJs.append(aJs);
        }
        actionsJs.append(roundActionsJs);
    }
    histJs["a"] = actionsJs;

    // Write rewards.
    Json::Value rewardsJs(Json::arrayValue);
    for (egn::dchips r : hist.rewards)
        rewardsJs.append(Json::Value(r));
    histJs["rw"] = rewardsJs;

    // Write collectedPot.
    Json::Value collectJs(Json::arrayValue);
    for (uint8_t x : hist.collectedPot)
        collectJs.append(Json::Value(x));
    histJs["co"] = collectJs;

    histJs["rk"] = hist.rake;

    Json::FastWriter writer;
    return os << writer.write(histJs);
}

std::istream& readCompressedData(std::istream& is, HandHistory& hist)
{
    std::string line;
    std::getline(is, line);

    Json::Value histJs;
    Json::Reader reader;
    reader.parse(line, histJs, false);

    // Read general info.
    hist.maxPlayers = histJs["mxP"].asUInt();
    hist.ante = histJs["at"].asUInt();
    hist.sb = histJs["sb"].asUInt();
    hist.bb = histJs["bb"].asUInt();
    hist.dealer = histJs["dl"].asUInt();

    // Read initialStakes.
    for (const Json::Value& x : histJs["inS"])
        hist.initialStakes.push_back(x.asUInt());

    // Read cards.
    if (histJs["bd"].asString().size())
        hist.boardCards = egn::Hand(histJs["bd"].asString());
    else
        hist.boardCards = egn::Hand::empty();
    for (const Json::Value& h : histJs["hd"]) {
        if (h.asString().size())
            hist.hands.push_back(
                egn::Hand::empty() + egn::Hand(h.asString()));
        else
            hist.hands.push_back(egn::Hand::empty());
    }

    // Read forcedBets.
    for (const Json::Value& x : histJs["frB"]["at"])
        hist.forcedBets.antes.push_back(x.asUInt());
    hist.forcedBets.sbPlayer = histJs["frB"]["sbP"].asUInt();
    hist.forcedBets.bbPlayer = histJs["frB"]["bbP"].asUInt();
    hist.forcedBets.sb = histJs["frB"]["sb"].asUInt();
    hist.forcedBets.bb = histJs["frB"]["bb"].asUInt();

    // Read actions.
    for (const Json::Value& roundActionsJs : histJs["a"]) {
        hist.actions.push_back({});
        for (const Json::Value& aJs : roundActionsJs) {
            Action a;
            a.player = aJs["p"].asUInt();
            a.action = egn::Action(aJs["a"].asUInt());
            a.bet = aJs["b"].asUInt();
            hist.actions.back().emplace_back(a);
        }
    }

    // Read rewards.
    for (const Json::Value& r : histJs["rw"])
        hist.rewards.push_back(r.asInt());

    // Read collectedPot.
    for (const Json::Value& x : histJs["co"])
        hist.collectedPot.push_back(x.asUInt());

    hist.rake = histJs["rk"].asUInt();

    return is;
}

} // hdt