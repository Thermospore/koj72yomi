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
		if (seqNo > 200) break;
		
		// Init dict csv components
		string title = "";
		string html = "";
		
		// Extract Title and Html
		int commaPos = line.find(",");
		title = line.substr(0, commaPos);
		html = line.substr(commaPos + 1);
		
		// Remove ‐s from Title
		int hyphPos = 0;
		while(1<2)
		{
			hyphPos = title.find("‐");
			if (hyphPos == -1) break;
			title.replace(hyphPos, 3, "");
		}
		
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
				
		// Extract reading and kanji
		// Check for 【】 brackets
		int brackPos = title.find(" 【");
		if (brackPos == -1) 
		{
			// No 【】 brackets; Kana only
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
				kanji = title.substr(0, brackPos);
			}
		}
		else 
		{
			// Yes 【】 brackets; Kanji + reading
			reading = title.substr(0, brackPos);
			kanji = title.substr(brackPos + 4, title.find("】") - brackPos - 4);
		}
		
		// Temp: dumping raw html into gloss
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
