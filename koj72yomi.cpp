#include <iostream>
#include <fstream>
using namespace std;

int main()
{
	// Create and open term bank file
	ofstream termBank("term_bank_1.json");
	
	// Write opening bracket
	termBank << "[";
	
	// Init sequence number
	int seqNo = 0;
	
	// Read dict line by line
	ifstream file("KOUJIEN7.csv");
	string line;
	getline(file, line); // skip initial "Title,Html" line
	while (getline(file, line))
	{
		seqNo++;
		
		// Temp: break once limit reached
		if (seqNo > 20) break;
		
		// Init dict csv components
		string title = "";
		string html = "";
		
		// Extract Title and Html
		int commaPos = line.find(",");
		title = line.substr(0, commaPos);
		html = line.substr(commaPos + 1);
		
		// Remove "s from edges of html
		html = html.substr(1, html.length() - 2);
		
		// Replace "" with \" in html
		int doubleQuotePos = 0;
		while(1<2)
		{
			doubleQuotePos = html.find("\"\"");
			if (doubleQuotePos == -1) break;
			html.replace(doubleQuotePos, 1, "\\");
		}
		
		// Init term bank components
		string kanji = "";
		string reading = "";
		string pos = "";
		int score = 0;
		string gloss = "";
		
		kanji = title;
		gloss = html;
		
		// Print entry to term bank
		if (seqNo != 1) termBank << ",\n";
		termBank << "[\"" 
			<< kanji << "\",\""
			<< reading << "\",\"\",\""
			<< pos << "\","
			<< score << ",[\""
			<< gloss << "\"],"
			<< seqNo << ",\"\"]";
	}
	
	// Write closing bracket
	termBank << "]";
	
	// Close the file
	termBank.close();
	
	return 0;
}
