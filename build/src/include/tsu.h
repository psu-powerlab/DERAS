// Author: Tylor Slay
// Description:
// 		This namespace was primary used to create the config.ini parser, but
// 		also holds complicated functions that may be used by lots of other
// 		classes such as FileToMatrix and ToString.

#ifndef TSU_H_INCLUDED
#define TSU_H_INCLUDED

// INCLUDES
#include <iostream> // cin, cout
#include <fstream>
#include <sstream>
#include <string> // getline, stoi, stod
#include <vector>
#include <regex>
#include <map>


// Tylor Slay Utilities is used to map config.ini files as well store files as
// either vectors or matricies. 
namespace tsu {

// map <section, map <property, value>>
// this is only for the INI file methods because it is huge
typedef std::map<std::string, std::map<std::string, std::string>> config_map;

// vector <vector <string>>
// this is used to create a matrix of strings
typedef std::vector <std::vector <std::string>> string_matrix;


// To String
// - the to_string () function in STL doesn't seem to work for all cases so I 
// - made this function as a hack to be uniform
template <typename T>
static std::string ToString (T t_value) {
	std::ostringstream ss;
	ss << t_value;
	return ss.str();
} // end To String

// File To String
// - this method reads the entire file into a single string.
// - primarily used by FileToVector or FileToMatrix, but can be stand-alone
static std::string FileToString (const std::string& kFilename) {

	// open file at end of file
	if (std::ifstream file{kFilename, std::ios::binary | std::ios::ate}) {
		auto buffer = file.tellg(); 	// what size is the file
		std::string str(buffer, '\0'); 	// construct string of buffer size
		file.seekg(0);					// return to start of file

		try {
			file.read(&str[0], buffer);
			return str;
		} catch (const std::exception& e) {
			std::cout << "[ERROR]...reading file: " << e.what () << std::endl;
			return NULL;
		}
	} else {
		std::cout << "[ERROR]...file not found!\n";
		return NULL;
 	}
} // end File To String

// Map Config File
// - this method uses regular expressions to create a config map to
// initialize class members or define program settings.
// - https://en.wikipedia.org/wiki/Regular_expression
static config_map MapConfigFile (const std::string& kFilename) {
	// read file into a string
	config_map file_map;
	std::string file_string, line, section, property, value;
	file_string = FileToString(kFilename);

	// create section and property regular expressions
	std::regex section_regex("^\\[(.*)\\]");
	std::regex property_regex("^(\\w+)=([^\\+]+(?!\\+{3}))");
	std::smatch match_regex;

	// split long string by each new line
	std::stringstream ss(file_string);
	while (std::getline(ss, line)) {
		// first look for section then property
		if (std::regex_search(line, match_regex, section_regex)) {
			section = match_regex[1];
		} else if (std::regex_search(line, match_regex, property_regex)){
			property = match_regex[1];
			value = match_regex[2];
			file_map[section][property] = value;
		}
	}
	return file_map;
} // end Map Config File

// Count Delimiter
// - count number of delimiters within string to make creating vectors and
// - matrices more efficient
static double CountDelimiter (const std::string& kString,
							  const char kDelimiter) {
	std::string line, item;
	double ctr = 0;
	std::stringstream ss(kString);

	// slit string by each new line character
	while (std::getline(ss, line)) {
		std::stringstream ss2(line);

		// split string by each delimiter and count
		while (std::getline(ss2, item, kDelimiter)) {
			ctr++;
		}
	}
	return ctr;
} // end Count Delimiter

// Split String
// - split string given delimiter
static std::vector<std::string> SplitString (const std::string& kString,
											 const char& kDelimiter) {
	std::vector<std::string> split_string;
	std::string line, item;
	double items = tsu::CountDelimiter(kString, kDelimiter);
	split_string.reserve(items);
	std::stringstream ss(kString);
	while (std::getline(ss, line)) {
		std::stringstream ss2(line);
		while (std::getline(ss2, item, kDelimiter)) {
			split_string.emplace_back(item);
		}
	}
	return split_string;
} // end Split String

// File To Vector
// - parse string for delimiter and create vector for each delimiter
static std::vector<std::string> FileToVector (const std::string& kFilename,
	const char& kDelimiter) {
	std::string whole_file = FileToString(kFilename);
	return SplitString(whole_file, kDelimiter);
} // end File To Vector

// File To Matrix
// - convert file to a vector of rows and then convert to vectors of columns.
static string_matrix FileToMatrix (const std::string &kFilename,
								   char kDelimiter,
								   unsigned int columns) {
	std::vector <std::string> file_vector;
	file_vector = FileToVector (kFilename, kDelimiter);

	// determine rows in matrix
	unsigned int count = file_vector.size();
	unsigned int rows = count/columns;

	// initialize matrix for rows/cols with empty cells
	string_matrix matrix (rows, std::vector <std::string> (columns, ""));

	// iterate through file_vector to populate matrix
	unsigned int cnt = 0;
	for (auto &row : matrix) {
		for (auto &col : row) {
			col = file_vector[cnt];
			cnt++;
		}
	}
	return matrix;
}

};  // end tsu


#endif // TSU_H_INCLUDED