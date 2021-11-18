#include <iostream>
#include <random>
#include <chrono>
#include <vector>
#include <omp/HandEvaluator.h>
#include <omp/Random.h>

using namespace std::chrono;
using namespace omp;
using std::vector;

int main()
{
    XoroShiro128Plus rng{ std::random_device{}() };
    FastUniformIntDistribution<unsigned> rnd(0, CARD_COUNT - 1);
    unsigned card;
    HandEvaluator eval;

    vector<Hand> table(10000000);
    for (Hand& hand : table) {
        // Generate a random 7-card hand.
        hand = Hand::empty();
        uint64_t usedCardMask = 0;
        for (int i = 0; i < 7; ++i) {
            uint64_t cardMask;
            do {
                card = rnd(rng);
                cardMask = 1ull << card;
            } while (usedCardMask & cardMask);
            usedCardMask |= cardMask;
            hand += card;
        }
    }

    auto t0 = std::chrono::high_resolution_clock::now();

    auto nevals = 0ull;
    for (int i = 0; i < 100; ++i) {
        for (Hand& hand : table) {
            eval.evaluate(hand);
            ++nevals;
        }
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    double duration = 1e-9 * std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
    std::cout << "time spent:" << duration << "s" << std::endl;
    std::cout << "number evals:" << nevals << std::endl;
    std::cout << "Mevals/sec:" << (double)nevals / duration * 1e-6 << std::endl;
}