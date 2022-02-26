#include "ActionSeqSize.h"

namespace abc {

ActionSeqSize::ActionSeqSize(const std::string& indexerName)
{
	auto file = std::fstream(ActionSeqIndexer::getSizesPath(indexerName), std::ios::in | std::ios::binary);
	opt::loadVar(preflopSize, file);
	opt::loadVar(flopSize, file);
	opt::loadVar(turnSize, file);
	opt::loadVar(riverSize, file);
	file.close();
}

} // abc