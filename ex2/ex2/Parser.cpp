#include "FileHandler.h"

Parser::~Parser()
{
	//if (m_manager != nullptr) delete m_manager;
	//m_mazeFile.close();
	//m_outputFile.close();
}


// TODO: Finish method
void Parser::createMazeList(const string & path)
{
}

// TODO: Finish method
void Parser::createAlgorithmList(const string& path) {
	/*
	FILE* dl;   // handle to read directory 
	string cmd_str = "ls " + path + "/*.so";
	const char *command_str = cmd_str.c_str();  // command string to get dynamic lib names
	char in_buf[BUF_SIZE];
	char curDLName[BUF_SIZE]; //name of the current dl
	void *dlib;
	dl = _popen(command_str, "r"); // TODO: in linux, delete underscore!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	if (!dl) {
		_pclose(dl);
		return false;
	}
	while (fgets(in_buf, BUF_SIZE, dl)) {
		// trim off the whitespace 
		char *ws = strpbrk(in_buf, " \t\n");
		if (ws) *ws = '\0';
		// append ./ to the front of the lib name
		sprintf(curDLName, "%s", in_buf);
		dlib = dlopen(curDLName, RTLD_NOW);
		if (dlib == NULL) {
			cerr << dlerror() << endl;
			_pclose(dl);
			exit(-1);
		}
		// add the handle to our list
		m_algorithmDL.push_back(dlib);
	}
	if (m_mazeFiles.size() == 0)
	{
		cout << "no *.so files found." << endl;
		_pclose(dl);
		return false;
	}
	_pclose(dl);
	*/
}

void Parser::createOutputList()
{
	list<MazePair>::iterator mazeIt;
	list<AlgorithmPair>::iterator algoIt;
	for (mazeIt = m_mazeList.begin(); mazeIt != m_mazeList.end(); ++mazeIt) {
		for (algoIt = m_algorithmList.begin(); algoIt != m_algorithmList.end(); ++algoIt) {
			// TODO: check correctness
			m_outputList.push_back(ofstream(m_outputPath + "/" + mazeIt->first + "_" + algoIt->first + ".output"));
		}
	}
}

void Parser::initVectorsByCurrDirectory(const string & path) {
	if (!mazePathExists) createMazeList(path);
	if (!algorithmPathExists) createAlgorithmList(path);
	if (mazePathExists) createOutputList();
}

// TODO: Finish method
void Parser::parsePairOfArguments(char * type, char * path) {
	if (strcmp(type, "-maze_path") == 0) { // .maze folder path
		if (!mazePathExists) {
			mazePathExists = true;
			createMazeList(path);
		}
		else invalidArguments = true;
	}
	else if (strcmp(type, "-algorithm_path") == 0) { // .so folder path
		if (!algorithmPathExists) {
			algorithmPathExists = true;
			createAlgorithmList(path);
		}
		else invalidArguments = true;
	}
	else if (strcmp(type, "-output") == 0) { // .output folder path
		if (!outputPathExists) {
			outputPathExists = true;
			m_outputPath = path;
		}
		else invalidArguments = true;
	}
	else invalidArguments = true;
}

/*	In the constructor we initialize maze, algorithm output vectors.
	We also check here validity of the program arguments. */
Parser::Parser(int argc, char * argv[]) {
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
		invalidArguments = true;
		break;
	}
}

/*	This function checks if there are errors. If so: updates m_errors.noErrors field to false and prints the errors */
void Parser::checkErrors(void*(titleFunc)) {
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
void Parser::parseInput() {
	string line;

	string name = getName(line);															// Collects maze parameters
	int maxSteps = getIntValue(MAXSTEPS, ErrorType::MaxStepsError, line);
	int rowsNum = getIntValue(ROWS, ErrorType::RowsError, line);
	int colsNum = getIntValue(COLS, ErrorType::ColsError, line);
	checkErrors((void*)printHeaderErrorTitle);
	if (m_errors.no_parsing_Errors) {														// No errors, lines 2-4 are valid.
		Coordinate playerLocation, endLocation;
		MazeBoard board = getBoard(rowsNum, colsNum, playerLocation, endLocation, line);
		checkErrors((void*)printMazeErrorTitle);
		if (m_errors.no_parsing_Errors)							// No errors, maze file is valid - creates a Manager object
			m_manager = new GameManager(name, maxSteps, rowsNum, colsNum,
				board, playerLocation, endLocation);
	}
}

/* This function retrieves the name of the maze. */
string Parser::getName(string & line) {
	if (getline(m_mazeFile, line)) {
		return line;
	}
	return nullptr;
}

/* This function retrieves the integer value for lines 2-4. */
int Parser::getIntValue(const string & input, const ErrorType error, string & line) {
	const regex reg("\\s*" + input + "\\s*=\\s*[1-9][0-9]*\\s*$");

	const regex numReg("[1-9][0-9]*");
	smatch match;
	if (getline(m_mazeFile, line)) {
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
MazeBoard Parser::getBoard(const int rows, const int cols, Coordinate & playerLocation, Coordinate & endLocation, string & line) {
	MazeBoard board;
	bool seenPlayerChar = false, seenEndChar = false;
	for (int i = 0; i < rows; i++) {
		MazeRow row;
		if (getline(m_mazeFile, line)) {											// Succeeded reading a line - fills MazeRow according to line
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
void Parser::handleSpecialChar(const char c, Coordinate & location, const int i, const int j, bool & seenChar, string & line, const ErrorType e) {
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
void Parser::handleInvalidChar(const char c, const int i, const int j) {
	string str = "000";
	str[0] = c;
	str[1] = (char)i;
	str[2] = (char)j;
	pushError(ErrorType::WrongChar, str);
}

/*	params: vector of game actions.
	This function pushes the actions vector into the output file. */
void Parser::pushActionsToOutputFile(vector<char> actions) {
	for (const char & c : actions)
		m_outputFile << c << endl;
}