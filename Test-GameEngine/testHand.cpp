#include "pch.h"

#include "../GameEngine/Hand.h"
#include <vector>
#include <string>

using namespace egn;

class HandTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		for (unsigned i = 0; i < strCards.size(); ++i) {
			singleHandsStr.push_back(Hand(strCards[i]));
			singleHandsInt.push_back(Hand(i));
		}
	}

	// Build all cards of deck in string format.
	static std::vector<std::string>& getStrCards()
	{
		static std::vector<std::string> cards;
		std::vector<char> ranks{ '2', '3', '4', '5', '6', '7', '8', '9', 't', 'j', 'q', 'k', 'a' };
		std::vector<char> suits{ 's', 'h', 'c', 'd' };
		std::string card;
		for (char r : ranks) {
			for (char s : suits) {
				card = "";
				card += r;
				card += s;
				cards.push_back(card);
			}
		}
		return cards;
	}

	static std::vector<std::string> strCards;

	// Single hands (one card for each hand) built from string.
	std::vector<Hand> singleHandsStr;
	// Single hands built from int.
	std::vector<Hand> singleHandsInt;
};

std::vector<std::string> HandTest::strCards(HandTest::getStrCards());

TEST_F(HandTest, StrInitWorks)
{
	for (size_t i = 0; i < singleHandsStr.size(); ++i)
		EXPECT_EQ(singleHandsStr[i], singleHandsInt[i]);
}

TEST_F(HandTest, GetStrWorks)
{
	// Test empty hand.
	std::ostringstream os;
	os << Hand::empty();
	EXPECT_EQ(os.str(), "");

	// Test single hands.
	for (size_t i = 0; i < singleHandsStr.size(); ++i) {
		std::ostringstream os;
		os << singleHandsStr[i];
		EXPECT_EQ(os.str(), strCards[i]);
	}
}