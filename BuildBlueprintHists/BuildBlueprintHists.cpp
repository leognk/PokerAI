#include "../Blueprint/Blueprint.h"
#include "../Utils/Histogram.h"

int main()
{
	bp::Blueprint blueprint(bp::BLUEPRINT_GAME_NAME, bp::BLUEPRINT_BUILD_NAME);
	blueprint.loadRegrets();
}