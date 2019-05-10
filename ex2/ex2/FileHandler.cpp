#include "FileHandler.h"
#include <algorithm>
FileHandler::~FileHandler()
{
	//if (m_manager != nullptr) delete m_manager;
	//m_mazeFile.close();
	//m_outputFile.close();
}


void FileHandler::createMatchVector(const string & path)
{
	FILE* dl;  // handle to read directory 
	string command_str = "ls " + path + "/*.maze";
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
		cout << "FH - after getting mazeVector. " << endl;
		ifstream *fin = new ifstream(filename.c_str());
		cout << "FH - after open the fin. " << filename << endl;
		if (!(*fin).is_open()) {
			exit(EXIT_FAILURE); // TODO: check how to exit
		}
		MatchManager * mm = parseInput(fin);
		if (mm == nullptr) {
			// TODO: deallocate all memory allocations
			return;
		}
		mm->createGameManagers();
		m_matchVector.push_back(mm);
	}
	pclose(dl);
}

void FileHandler::createAlgorithmVector(const string& path) {
	FILE* dl;  // handle to read directory 
	string command_str = "ls " + path + "/*.so";
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
			string algorithmName = filename.substr(index, filename.length() - 5);

			cout << "FH - checking for so:  "<< algorithmName << endl;

			dlib = dlopen(filename.c_str(), RTLD_LAZY);
			if (dlib == NULL) {
				cerr << dlerror() << endl;
				exit(EXIT_FAILURE);
			}
			dlVector.push_back(dlib);
			m_algorithmNameVector.push_back(algorithmName);
		}
	}
	pclose(dl);
}

void FileHandler::createOutput()
{
	unsigned int column_length = 15, num_of_mazes = m_matchVector.size();

	// seperation row
	for (unsigned int i = 0; i < (column_length + 1) * num_of_mazes; i++) {
		cout << "-";
	}
	cout << endl;

	// second line - titles

	// first column - algorithms title
	cout << "|";
	for (unsigned int i = 0; i < column_length; i++) {
		cout << " ";
	}
	
	// rest of the columns - mazes titles
	for (unsigned int j = 0; j < num_of_mazes; j++) {
		string & mazeName = m_matchVector[j]->getName();
		cout << "|" << mazeName;
		for (unsigned int i = 0; i < column_length - mazeName.length(); i++) {
			cout << " ";
		}
	}
	cout << "|" << endl;

	// information rows
	for (unsigned int i = 0; i < m_algorithmNameVector.size(); i++) {

		// seperation row
		for (unsigned int j = 0; j < (column_length + 1) * num_of_mazes; j++) {
			cout << "-";
		}
		cout << endl;

		// algorithm name
		string & algoName = m_algorithmNameVector[i];
		cout << "|" << algoName;
		for (unsigned int j = 0; j < column_length - algoName.length(); j++) {
			cout << " ";
		}


		// algorithm information for each maze
		for (unsigned int j = 0; j < m_matchVector.size(); j++) {
			vector<vector<char>> vec = m_matchVector[j]->getMoveListVector();
			cout << "|";
			if (vec[i][vec[i].size() - 1] == '!') {
				string str = "" + (vec[i].size() - 1);
				for (unsigned int k = 0; k < column_length - str.length(); k++) {
					cout << " ";
				}
				cout << str;
			}
			else {
				for (unsigned int k = 0; k < column_length - 2; k++) {
					cout << " ";
				}
				cout << "-1";
			}
			
			// output file handling
			if (m_outputPathExists) {
				string & mazeName = m_matchVector[j]->getName();
				ofstream fout = ofstream();
				fout.open(m_outputPath + "/" + mazeName + "_" + algoName + ".output");
				for (char c : vec[i]) fout << c << endl;
			}
		}
		cout << "|" << endl;
	}
	

	// seperation row
	for (unsigned int i = 0; i < (column_length + 1) * num_of_mazes; i++) {
		cout << "-";
	}
	cout << endl;

}

void FileHandler::initVectorsByCurrDirectory(const string & path) {
	if (!m_algorithmPathExists) createAlgorithmVector(path);
	if (!m_mazePathExists) createMatchVector(path);
	createOutput();
}

// TODO: Finish method
void FileHandler::parsePairOfArguments(char * type, char * path) {
	if (strcmp(type, "-maze_path") == 0) { // .maze folder path
		if (!m_mazePathExists) {
			m_mazePathExists = true;
			createMatchVector(path);
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
		initVectorsByCurrDirectory(".");
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

	cout << "FH - starting to parse Input. " << endl;
	string name = getName(fin, line);															// Collects maze parameters

	cout << "FH - after getting the name. " << endl;
	int maxSteps = getIntValue(fin, MAXSTEPS, ErrorType::MaxStepsError, line);
	int rowsNum = getIntValue(fin, ROWS, ErrorType::RowsError, line);
	int colsNum = getIntValue(fin, COLS, ErrorType::ColsError, line);
	checkErrors((void*)printHeaderErrorTitle);

	cout << "FH - before checking no_parsing_error. " << endl;
	if (m_errors.no_parsing_Errors) {														// No errors, lines 2-4 are valid.

		cout << "FH - after checking no_parsing_error. " << endl;
		Coordinate playerLocation, endLocation;
		MazeBoard board = getBoard(fin, rowsNum, colsNum, playerLocation, endLocation, line);

		cout << "FH - after getting board. " << endl;
		checkErrors((void*)printMazeErrorTitle);
		if (m_errors.no_parsing_Errors)							// No errors, maze file is valid - creates a Manager object
			return new MatchManager(name, maxSteps, rowsNum, colsNum,
				board, playerLocation, endLocation, m_algorithmNameVector);
	}

	cout << "FH - return nullptr in parseInput function. " << endl;
	return nullptr;
}

/* This function retrieves the name of the maze. */
string FileHandler::getName(ifstream * fin, string & line) {
	cout << "FH - before the if in get name." << endl;
	if (getline(*fin, line)) {

		cout << "FH - inside the if: " << line << endl;
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
	if (outputPathExists || !outputPathExists) {
		for (unsigned int i = 0; i < matchVector.size(); i++) {
			/*for (int j = 0; j < matchVector[i]->algorithmsCount(); j++) {

			}
			matchVector[i]->*/
		}
	}
}