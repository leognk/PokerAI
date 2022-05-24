# PokerAI

This is an ongoing personal project that aims to develop an AI for multiplayer no-limit Texas hold'em poker based on the AI Pluribus.

Pluribus is the first AI that has beaten professional players in no-limit Texas hold'em poker with 6 players on the table.
Concretely, the goal of this project is to be able to play poker against strong AIs and to get insights on what the best moves are in a given situation to try to better understand the complex game of poker.

What has been done so far:
- fast poker game engine.
- information abstraction.
- action abstraction.
- MCCFR for computing the blueprint strategy.
- dealing with different initial stakes, ante, big blind for actual play.

To-do:
- Real-time search.
- GUI to play poker against AIs.

For the moment, I just tested the blueprint strategy against 5 random AIs. Each iteration proceeds as follows:
- give each player 100 BB.
- play the rounds until only one player remains.
In this setting, the blueprint strategy achieves an average gain of 2.50 BB/game (1 game = 1 round) with a standard deviation of 26.6 BB/game (number of games: 84.1 million). The blueprint strategy needs to be further tested against stronger opponents.
