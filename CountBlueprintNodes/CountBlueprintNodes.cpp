#include "../AbstractInfoset/TreeTraverser.h"
#include "../Blueprint/Constants.h"
#include "../LosslessAbstraction/hand_index.h"

int main()
{
	abc::TreeTraverser traverser(
		bp::MAX_PLAYERS, bp::ANTE, bp::BIG_BLIND, bp::INITIAL_STAKE, bp::BET_SIZES, true);

	std::vector<abc::TreeTraverser::seqs_t> actionSeqs;
	traverser.traverseTree(actionSeqs);
}