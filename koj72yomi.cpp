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
		// Get next line and bump seqNo
		getline(file, line);
		seqNo++;
		
		// Break once end of csv is reached
		// TEMP: or once seqNo limit is reached
		// WARNING: with no cap, you will exceed github file size limit...
		if (line == "," /**/|| seqNo > 200/**/) break;
		
		// Detect if ALPH entry
		bool alphEntry;
		if (line.find("ALPH.svg") == -1)
		{
			alphEntry = false;
		}
		else
		{
			alphEntry = true;
		}
		
		// Init dict csv components
		string title = "";
		string html = "";
		
		// Init term bank components
		string kanji = "";
		string reading = "";
		string pos = "";
		int score = 0;
		string midashi = "";
		string honbun = "";
		
		// Extract Title and Html
		// NOTE: should print all the titles out to check them
		int separatorPos = line.find(",\"<rn></rn><a name=\"\"");
		title = line.substr(0, separatorPos);
		html = line.substr(separatorPos + 2, line.length() - separatorPos - 3);
		
		// Handle Titles in quotation marks
		if (title[0] == '\"')
		{
			// remove external quotes
			title = title.substr(1, title.length() - 2);
			
			// replace "" with \"
			int doubleQuotePos = 0;
			while(1<2)
			{
				doubleQuotePos = title.find("\"\"");
				if (doubleQuotePos == -1) break;
				title.replace(doubleQuotePos, 1, "\\");
			}
			
			// Remove rubi objects
			
			// Replace gaiji objects (can be multiple per line)
			
			//debugOutput << title << endl;
		}
		
		// NOTE: when to handle ○s?
		
		// NOTE: remove instances of `<sub>` `</sub>` `<sup>` `</sup>` from title
		
		// NOTE: remove leading and trailing junk for ALPH
		
		// NOTE: remove ・ outside of brackets?
		
		// Remove ‐s from Title
		// NOTE: count entry with largest number of these
		// NOTE: check for entries that don't have the same number in title and midashi
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
				
		// Extract reading and kanji from title
		// NOTE: how to handle ALPH entries at end?
		// NOTE: titles with gaiji or subtext?
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
		
		// Extract midashi from html
		// NOTE: need to handle the <sub>‥イフ</sub> things
		midashi = html.substr(61, html.find("</div>") - 61); // They all start at 61...
		
		// Extract honbun from html
		// NOTE: ya doing it wrong. some of the honbuns don't go all the way to the end, like 012110600001
		//int honbunStart = html.find("honbun") + 9;
		//honbun = html.substr(honbunStart, html.length() - honbunStart - 6);
				
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
