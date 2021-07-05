#include <iostream>
#include <fstream>
using namespace std;

int main()
{
	// Create and open files
	ofstream termBank("term_bank_1.json");
	ofstream debugOutput("debugOutput.txt");
	
	// Write opening bracket
	termBank << "[";
	
	// Init sequence number
	int seqNo = 0;
	
	// Read dict line by line
	ifstream file("KOUJIEN7.csv");
	string line;
	getline(file, line); // skip initial "Title,Html" line
	while (1<2)
	{
		// Grab lines until end of csv
		getline(file, line);
		if (line == ",") break;
		
		seqNo++;
		
		// TEMP: break once limit reached
		// WARNING: with no cap, you will exceed github file size limit...
		if (seqNo > 200) break;
		
		// Init dict csv components
		string title = "";
		string html = "";
		
		// Extract Title and Html
		int separatorPos = line.find(",\"<rn></rn><a name=\"\"");
		title = line.substr(0, separatorPos);
		html = line.substr(separatorPos + 2, line.length() - separatorPos - 3);
		
		// TEMP: print html
		//debugOutput << html << endl;
		
		// Remove ‐s from Title
		// NOTE: count entry with largest number of these
		int hyphPos = 0;
		while(1<2)
		{
			hyphPos = title.find("‐");
			if (hyphPos == -1) break;
			title.replace(hyphPos, 3, "");
		}
		
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
		string midashi = "";
		string honbun = "";
				
		// Extract reading and kanji
		// Check for 【】 brackets
		int brackPos = title.find(" 【");
		if (brackPos == -1) 
		{
			// No 【】 brackets; Kana only
			// NOTE: export a bunch of these to see how they look
			// Check for （） brackets
			brackPos = title.find("（");
			if (brackPos == -1) 
			{
				// No （） brackets to worry about
				kanji = title;
			}
			else
			{
				// Ignore （） bracket stuff at end
				// NOTE: maybe add these in tags? some show up in honbun and some don't
				kanji = title.substr(0, brackPos);
			}
		}
		else 
		{
			// Yes 【】 brackets; Kanji + reading
			// NOTE: export a bunch of stuff outside of 】 to see how it looks
			reading = title.substr(0, brackPos);
			kanji = title.substr(brackPos + 4, title.find("】") - brackPos - 4);
		}
		
		// TEMP: dumping raw html into honbun
		honbun = html;
		
		// potential strategy:
		// Get rid of junk at start
		// extract midashi
		// extract honbun
		// see if anything is left outside
		
		// Go through html tag by tag
		// Identify which tag it is
		// extract tag data
		// proccess said data
		// remove tag from html
		// might need to rethink / get recursive if there are too many tags inside of tags
		// maybe go count the size of the largest div nest
		
		// better error/exception checking...
		// check for multiple csv commas
		
		// Print entry to term bank
		// Note: probably faster to concat everything and minimize file writes?
		if (seqNo != 1) termBank << ",\n";
		termBank << "[\"" 
			<< kanji << "\",\""
			<< reading << "\",\"\",\""
			<< pos << "\","
			<< score << ",[\""
			<< midashi << "\\n"
			<< honbun << "\"],"
			<< seqNo << ",\"\"]";
	}
	
	// Write closing bracket
	termBank << "]";
	
	// Close files
	termBank.close();
	debugOutput.close();
	
	return 0;
}
