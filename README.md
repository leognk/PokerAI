# PokerAI

This personal project aims to develop an AI for multiplayer no-limit Texas hold'em poker, inspired by [Pluribus](https://en.wikipedia.org/wiki/Pluribus_(poker_bot)). Pluribus is the first AI to beat professional players in no-limit Texas hold'em poker with 6 players.

The project's objective is to enable gameplay against strong AIs and provide insights into near optimal strategies in various scenarios, enhancing understanding of this intricate game.

## What has been done so far
- **Fast poker game engine**: Implements the game rules.
- **Information abstraction**: Groups together similar poker hands to reduce the search tree.
- **Action abstraction**: Groups together similar actions to reduce the search tree.
- **MCCFR for computing the blueprint strategy**: An algorithm using Monte Carlo simulations to explore the search tree and stores the regrets of each action (the opposite of rewards) in a file.
- Handling different initial stakes, ante, and big blind for actual play.

## To-Do
- Real-time search.
- GUI for playing against AIs.

## Testing and Results
So far, I have tested the blueprint strategy against five random AI players. Each test iteration involves giving each player 100 Big Blinds (BB) and playing rounds until only one player remains.

In these tests, the blueprint strategy achieved an average gain of 2.50 BB/game (1 game = 1 round) with a standard deviation of 26.6 BB/game. The number of games played totaled 84.1 million. Further testing against more sophisticated opponents would be nice.