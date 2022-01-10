#include "pch.h"
#include <numeric>
#include "../LossyAbstraction/EquityCalculator.h"
#include "../GameEngine/Hand.h"
#include "../Utils/ioArray.h"

static const std::string hsHistExDir = "../data/AbstractionSaves/HSHistExamples/";

class EquityCalculatorTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		static bool initVar = (staticInit(), true);
	}
	static void staticInit()
	{
		eqt.loadRivHSLUT();
	}
	static abc::EquityCalculator eqt;
};

TEST_F(EquityCalculatorTest, EquityInRange)
{
  for (uint16_t hs : eqt.RIV_HS_LUT)
	  EXPECT_LE(hs, abc::MAX_RIV_HS);
}

TEST_F(EquityCalculatorTest, VerifySomeRivHS)
{
	EXPECT_EQ(eqt.RIV_HS_LUT[0], 1974);
	EXPECT_EQ(eqt.RIV_HS_LUT[5], 1972);
	EXPECT_EQ(eqt.RIV_HS_LUT[1000004], 1980);
	EXPECT_EQ(eqt.RIV_HS_LUT[1000006], 8);
	EXPECT_EQ(eqt.RIV_HS_LUT[50000002], 1964);
	EXPECT_EQ(eqt.RIV_HS_LUT[60000953], 1964);
	EXPECT_EQ(eqt.RIV_HS_LUT[123156245], 1980);
	EXPECT_EQ(eqt.RIV_HS_LUT[123156253], 1980);
}

TEST_F(EquityCalculatorTest, VerifySomeTurnHSHist)
{
	const std::vector<std::string> fileNames = {
		"Turn equity distribution - 2h5h-3hKhAsQc.bin",
		"Turn equity distribution - AsAc-3hKhKsQc.bin"
	};
	const std::vector<std::string> handStrings = {
		"2h 5h 3h Kh As Qc",
		"As Ac 3h Kh Ks Qc"
	};
	for (uint8_t i = 0; i < fileNames.size(); ++i) {
		std::array<uint8_t, 50> hsHistRef;
		opt::loadArray(hsHistRef, hsHistExDir + fileNames[i]);
		auto hand = egn::Hand::stringToArray<7>(handStrings[i]);
		auto hsHist = eqt.buildTurnHSHist(hand.data());
		for (uint8_t j = 0; j < hsHist.size(); ++j)
			EXPECT_EQ(hsHist[j], hsHistRef[j]);
		uint8_t sum = std::accumulate(hsHist.begin(), hsHist.end(), uint8_t(0));
		EXPECT_EQ(sum, 46); // 52 - 6
	}
}

TEST_F(EquityCalculatorTest, VerifySomeFlopHSHist)
{
	const std::vector<std::string> fileNames = {
		"Flop equity distribution - TcQd-7h9hQh.bin",
		"Flop equity distribution - 5c9d-3d5d7d.bin"
	};
	const std::vector<std::string> handStrings = {
		"Tc Qd 7h 9h Qh",
		"5c 9d 3d 5d 7d"
	};
	for (uint8_t i = 0; i < fileNames.size(); ++i) {
		std::array<uint16_t, 50> hsHistRef;
		opt::loadArray(hsHistRef, hsHistExDir + fileNames[i]);
		auto hand = egn::Hand::stringToArray<7>(handStrings[i]);
		auto hsHist = eqt.buildFlopHSHist(hand.data());
		for (uint8_t j = 0; j < hsHist.size(); ++j)
			EXPECT_EQ(hsHist[j], hsHistRef[j]);
		uint16_t sum = std::accumulate(hsHist.begin(), hsHist.end(), uint16_t(0));
		EXPECT_EQ(sum, 1081); // binom(52 - 5, 2) = 47 * 23
	}
}

TEST_F(EquityCalculatorTest, VerifySomePreflopHSHist)
{
	const std::vector<std::string> fileNames = {
		"Preflop equity distribution - 4s4h.bin",
		"Preflop equity distribution - 6s6h.bin",
		"Preflop equity distribution - QsKs.bin",
		"Preflop equity distribution - TsJs.bin"
	};
	const std::vector<std::string> handStrings = {
		"4s 4h",
		"6s 6h",
		"Qs Ks",
		"Ts Js"
	};
	for (uint8_t i = 0; i < fileNames.size(); ++i) {
		std::array<uint32_t, 50> hsHistRef;
		opt::loadArray(hsHistRef, hsHistExDir + fileNames[i]);
		auto hand = egn::Hand::stringToArray<7>(handStrings[i]);
		auto hsHist = eqt.buildPreflopHSHist(hand.data());
		for (uint32_t j = 0; j < hsHist.size(); ++j)
			EXPECT_EQ(hsHist[j], hsHistRef[j]);
		uint32_t sum = std::accumulate(hsHist.begin(), hsHist.end(), uint32_t(0));
		EXPECT_EQ(sum, 2118760); // binom(52 - 2, 5)
	}
}