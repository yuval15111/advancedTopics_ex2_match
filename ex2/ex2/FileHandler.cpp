#include "FileHandler.h"
#include <algorithm>
FileHandler::~FileHandler()
{
	//if (m_manager != nullptr) delete m_manager;
	//m_mazeFile.close();
	//m_outputFile.close();
}


void FileHandler::createMazeVector(const string & path)
{
	FILE* dl;  // handle to read directory 
	string command_str = "ls " + path + "/*maze";
	char in_buf[BUF_SIZE];
	// get the names of all the .maze  files in the current dir 
	dl = popen(command_str.c_str(), "r");
	if (!dl) {
		cout << "Error: error in popen command. " << strerror(errno) << endl;
		exit(EXIT_FAILURE); //TODO: check how to exit
	}
	
	while (fgets(in_buf, BUF_SIZE, dl)) {
		// trim off the whitespace 
		char* ws = strpbrk(in_buf, " \t\n");
		if (ws) *ws = '\0';
		string filename(in_buf);
		size_t index = filename.find("_308243351_");
		if ((index != string::npos) && (endsWith(filename, ".maze"))) {
			string mazeName = filename.substr(index, filename.length() - 5);
			ifstream fin;
			fin.open(filename.c_str());
			if (!fin.is_open()) {
				exit(EXIT_FAILURE); // TODO: check how to exit
			}
			m_mazeVector.push_back(make_pair(mazeName, &fin));
		}
	}
	pclose(dl);
}

void FileHandler::createAlgorithmVector(const string& path) {
	FILE* dl;  // handle to read directory 
	string command_str = "ls " + path + "/*so";
	char in_buf[BUF_SIZE]; // input buffer for lib names 
	// get the names of all the dynamic libs (.so  files) in the current dir 
	dl = popen(command_str.c_str(), "r");
	if (!dl) {
		cout << "Error: error in popen command. " << strerror(errno) << endl;
		exit(EXIT_FAILURE); //TODO: check if exitting is legal
	}
	void* dlib;
	while (fgets(in_buf, BUF_SIZE, dl)) {
		// trim off the whitespace 
		char* ws = strpbrk(in_buf, " \t\n");
		if (ws) *ws = '\0';
		string filename(in_buf);
		size_t index = filename.find("_308243351_");
		if ((index != string::npos) && (endsWith(filename, ".so"))) {
			string algorithmName = filename.substr(index, filename.length() - 3);
			dlib = dlopen(filename.c_str(), RTLD_LAZY);
			if (dlib == NULL) {
				cerr << dlerror() << endl;
				exit(EXIT_FAILURE);
			}
			dlVector.push_back(dlib);
			//m_algorithmVector.push_back(make_pair(algorithmName, dlib));
			m_algorithmNameVector.push_back(algorithmName);
		}
	}
	pclose(dl);
}

void FileHandler::createOutputVector()
{
	vector<MazePair>::iterator mazeIt;
	for (mazeIt = m_mazeVector.begin(); mazeIt != m_mazeVector.end(); ++mazeIt) {
		for (string & algoName : m_algorithmNameVector) {
			m_outputVector.push_back(ofstream(m_outputPath + "/" + mazeIt->first + "_" + algoName + ".output"));
		}
	}
}

void FileHandler::initVectorsByCurrDirectory(const string & path) {
	if (!m_mazePathExists) createMazeVector(path);
	if (!m_algorithmPathExists) createAlgorithmVector(path);
	if (m_outputPathExists) createOutputVector();
}

// TODO: Finish method
void FileHandler::parsePairOfArguments(char * type, char * path) {
	if (strcmp(type, "-maze_path") == 0) { // .maze folder path
		if (!m_mazePathExists) {
			m_mazePathExists = true;
			createMazeVector(path);
		}
		else m_invalidArguments = true;
	}
	else if (strcmp(type, "-algorithm_path") == 0) { // .so folder path
		if (!m_algorithmPathExists) {
			m_algorithmPathExists = true;
			createAlgorithmVector(path);
		}
		else m_invalidArguments = true;
	}
	else if (strcmp(type, "-output") == 0) { // .output folder path
		if (!m_outputPathExists) {
			m_outputPathExists = true;
			m_outputPath = path;
		}
		else m_invalidArguments = true;
	}
	else m_invalidArguments = true;
}

/*	In the constructor we initialize maze, algorithm output vectors.
	We also check here validity of the program arguments. */
FileHandler::FileHandler(int argc, char * argv[]) {
	switch (argc) {
	case 7:
		parsePairOfArguments(argv[5], argv[6]);
	case 5:
		parsePairOfArguments(argv[3], argv[4]);
	case 3:
		parsePairOfArguments(argv[1], argv[2]);
	case 1:
		initVectorsByCurrDirectory(argv[0]);
		break;
	default:
		m_invalidArguments = true;
		break;
	}
}

/*	This function checks if there are errors. If so: updates m_errors.noErrors field to false and prints the errors */
void FileHandler::checkErrors(void*(titleFunc)) {
	if (m_errors.list.size() == 0) return;
	if (titleFunc != nullptr) { // parsing errors
		FuncNoArgs f = (FuncNoArgs)titleFunc;
		f();

		m_errors.no_parsing_Errors = false;
	}
	for (ErrorList::iterator it = m_errors.list.begin(); it != m_errors.list.end(); ++it) {
		Func f = m_errors.fmap[it->first];
		string str = it->second;
		f(str);
	}
	m_errors.list.clear();
}

/* This function parses the input file and creates the manager object. */
MatchManager * FileHandler::parseInput(ifstream * fin) {
	string line;

	string name = getName(fin, line);															// Collects maze parameters
	int maxSteps = getIntValue(fin, MAXSTEPS, ErrorType::MaxStepsError, line);
	int rowsNum = getIntValue(fin, ROWS, ErrorType::RowsError, line);
	int colsNum = getIntValue(fin, COLS, ErrorType::ColsError, line);
	checkErrors((void*)printHeaderErrorTitle);
	if (m_errors.no_parsing_Errors) {														// No errors, lines 2-4 are valid.
		Coordinate playerLocation, endLocation;
		MazeBoard board = getBoard(fin, rowsNum, colsNum, playerLocation, endLocation, line);
		checkErrors((void*)printMazeErrorTitle);
		if (m_errors.no_parsing_Errors)							// No errors, maze file is valid - creates a Manager object
			return new MatchManager(name, maxSteps, rowsNum, colsNum,
				board, playerLocation, endLocation, m_algorithmNameVector);
	}
	return nullptr;
}

/* This function retrieves the name of the maze. */
string FileHandler::getName(ifstream * fin, string & line) {
	if (getline(*fin, line)) {
		return line;
	}
	return nullptr;
}

/* This function retrieves the integer value for lines 2-4. */
int FileHandler::getIntValue(ifstream * fin, const string & input, const ErrorType error, string & line) {
	const regex reg("\\s*" + input + "\\s*=\\s*[1-9][0-9]*\\s*$");

	const regex numReg("[1-9][0-9]*");
	smatch match;
	if (getline(*fin, line)) {
		if (!regex_match(line, reg)) {
			pushError(error, line);
			return -1;
		}
		regex_search(line, match, numReg);
		return stoi(match[0]);
	}
	pushError(error, line);
	return -1;
}

/*	params: rows, col - parsed from maze file; references to playerLocation and endLocation that will be filled in this function;
			refernce to line string which we fill with lines from the input and parse the file with.
	return: A maze board object (two-dimensional character vector) */
MazeBoard FileHandler::getBoard(ifstream * fin, const int rows, const int cols, Coordinate & playerLocation, Coordinate & endLocation, string & line) {
	MazeBoard board;
	bool seenPlayerChar = false, seenEndChar = false;
	for (int i = 0; i < rows; i++) {
		MazeRow row;
		if (getline(*fin, line)) {											// Succeeded reading a line - fills MazeRow according to line
			for (int j = 0; j < min(cols, (int)line.length()); j++) {
				if (line[j] == PLAYER_CHAR)
					handleSpecialChar(PLAYER_CHAR, playerLocation, i, j, seenPlayerChar, line, ErrorType::MoreThanOnePlayerChar);
				else if (line[j] == END_CHAR)
					handleSpecialChar(END_CHAR, endLocation, i, j, seenEndChar, line, ErrorType::MoreThanOneEndChar);
				else if (line[j] == '\r') line[j] = SPACE_CHAR;
				else if (line[j] != SPACE_CHAR && line[j] != WALL_CHAR) // other chars are invalid
					handleInvalidChar(line[j], i, j);
				row.push_back(line[j]);
			}
			for (int j = (int)line.length(); j < cols; j++)					// Fill with SPACE_CHAR when cols > line.length()
				row.push_back(SPACE_CHAR);
		}
		else																// No more lines in maze file - fills the row with SPACE_CHARs
			for (int j = 0; j < cols; j++)
				row.push_back(SPACE_CHAR);
		board.push_back(row);
	}
	if (!seenPlayerChar) pushError(ErrorType::MissingPlayerChar, string());
	if (!seenEndChar) pushError(ErrorType::MissingEndChar, string());
	return board;
}

/*	A helper function for getBoard().
	Params: char c, location reference, (i, j) new coordinate indices, other helping parameters.
	The function updates the location coordinate by (i, j) values or pushes errors to the Errors vector if needed. */
void FileHandler::handleSpecialChar(const char c, Coordinate & location, const int i, const int j, bool & seenChar, string & line, const ErrorType e) {
	if (!seenChar) {
		updateCoordinate(location, i, j);
		seenChar = true;
		if (c == PLAYER_CHAR) line[j] = SPACE_CHAR; // not necessary as '@' character
	}
	else { // only one player char allowed!
		pushError(e, string());
	}
}

/*	A helper function for getBoard().
	Params: invalid char c, (i, j) error indices.
	The function pushes invalid char error to Error vector. */
void FileHandler::handleInvalidChar(const char c, const int i, const int j) {
	string str = "000";
	str[0] = c;
	str[1] = (char)i;
	str[2] = (char)j;
	pushError(ErrorType::WrongChar, str);
}

/*	params: vector of game actions.
	This function pushes the actions vector into the output file. */
void FileHandler::pushActionsToOutputFile(ofstream & fout, vector<char> actions) {
	for (const char & c : actions)
		fout << c << endl;
}

void FileHandler::pushLogsToOutputFiles(vector<MatchManager*> matchVector, bool outputPathExists)
{
	outputPathExists = false;
	for (unsigned int i = 0; i < matchVector.size(); i++) {
		/*for (int j = 0; j < matchVector[i]->algorithmsCount(); j++) {

		}
		matchVector[i]->*/
	}
}