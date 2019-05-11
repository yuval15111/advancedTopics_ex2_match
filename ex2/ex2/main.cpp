#include "FileHandler.h"

int main(int argc, char* argv[]) {
	// MAZEPATH_INDEX = 0, ALGOPATH_INDEX = 1, OUTPUTPATH_INDEX = 2
	vector<string> paths = { ".", ".", "" };
	bool validArgs = initPaths(argc, argv, paths);
	if (!validArgs) {
		printWrongArgumentsFormatError();
		return -1;
	}
	FileHandler fh(paths);
	fh.getAlgorithms();
	fh.getMatchesAndPlay();
	fh.createOutput();
	return 0;
}