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
		// NOTE: should just take term bank and debug offline, but keep a .old copy and use a local diff program
		// NOTE: maybe ask for cap (or even a range) in console
		if (line == "," /**/|| seqNo > 300/**/) break;
		
		// Detect if ALPH entry
		bool alphEntry = (line.find("ALPH.svg") == -1) ? false : true;
		
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
		
		// Handle titles in quotation marks
		if (title[0] == '\"')
		{
			// Remove external quotes
			title = title.substr(1, title.length() - 2);
			
			// Replace "" with \"
			int doubleQuotePos = 0;
			while(1<2)
			{
				doubleQuotePos = title.find("\"\"");
				if (doubleQuotePos == -1) break;
				title.replace(doubleQuotePos, 1, "\\");
			}
			
			// Remove rubi object
			int rubiPos = title.find("<sub class=\\\"rubi\\\">");
			if (rubiPos != -1)
			{
				title.erase(rubiPos, title.find("</sub>") - rubiPos + 6);
			}
			
			// Replace gaiji objects (can be multiple per line)
			// NOTE: still need to insert mapped gaiji from table
			while(1<2)
			{
				int gaijiPos = title.find("<object class=\\\"gaiji\\\"");
				if (gaijiPos == -1) break;
				title.replace(gaijiPos, 51, "�");
			}
		}
		
		// Remove inconsistent space before `【` in title
		int randoSpacePos = title.find(" 【");
		if (randoSpacePos != -1)
		title.erase(randoSpacePos, 1);
		
		// NOTE: when to handle ○s?
		
		// Remove inflection point thingies ・ in title reading ie `あざ・る`
		int inflecPos = title.find("・");
		if (inflecPos != -1)
		{
			// Don't remove ・ inside of （） or 【】 !!!
			int parenthPos = title.find("（");
			int bracPos = title.find("【");
			if (!(parenthPos != -1 && parenthPos < inflecPos) &&
				!(bracPos != -1 && bracPos < inflecPos))
			title.erase(inflecPos, 3);
		}
		
		// Clean up ALPH title
		if (alphEntry == true)
		{
			// Remove starting junk
			title = title.substr(50);
			
			// Remove ending numbers
			if (title.find("①") != -1 ||
				title.find("①") != -1 ||
				title.find("②") != -1 ||
				title.find("③") != -1 ||
				title.find("④") != -1 ||
				title.find("⑤") != -1 ||
				title.find("⑥") != -1 ||
				title.find("⑦") != -1 ||
				title.find("⑧") != -1 ||
				title.find("⑨") != -1 ||
				title.find("⑩") != -1)
			{
				title = title.substr(0, title.length() - 3);
			}
		}
		
		// Lazily and dangerously remove remaining tags from title
		// (should only be <sub>, <sup>, and <i>)
		while(1<2)
		{
			int tagPos = title.find("<");
			if (tagPos == -1) break;
			title.erase(tagPos, title.find(">") - tagPos + 1);
		}
		
		// Remove ‐s from title
		// NOTE: count entry with largest number of these
		// NOTE: check for entries that don't have the same number in title and midashi
		int hyphPos = 0;
		while(1<2)
		{
			hyphPos = title.find("‐");
			if (hyphPos == -1) break;
			title.replace(hyphPos, 3, "");
		}
				
		// Extract reading and kanji from title
		// NOTE: make case for ALPH entries
		// Check for 【】 brackets
		int brackPos = title.find("【");
		if (brackPos == -1) 
		{
			// No 【】 brackets; kana only
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
			// Yes 【】 brackets; kanji + reading
			// NOTE: export a bunch of stuff outside of 】 to see how it looks
			reading = title.substr(0, brackPos);
			kanji = title.substr(brackPos + 3, title.find("】") - brackPos - 3);
		}
		
		// Replace "" with \" in html
		int doubleQuotePos = 0;
		while(1<2)
		{
			doubleQuotePos = html.find("\"\"");
			if (doubleQuotePos == -1) break;
			html.replace(doubleQuotePos, 1, "\\");
		}
		
		// Extract midashi from html
		// NOTE: also handle gaiji
		midashi = html.substr(61, html.find("</div>") - 61); // They all start at 61...
		
		// Handle <sub> in midashi using yomichan structured content
		int subTagPos = 0;
		while(1<2)
		{
			subTagPos = midashi.find("<sub>");
			if (subTagPos == -1) break;
			midashi.replace(subTagPos, 5, "\", {\"tag\": \"span\", \"style\": {\"fontSize\": \"small\", \"verticalAlign\": \"sub\"}, \"content\": \"");
			midashi.replace(midashi.find("</sub>"), 6, "\"}, \"");
		}
		
		// Handle <sup> in midashi
		int supTagPos = 0;
		while(1<2)
		{
			supTagPos = midashi.find("<sup>");
			if (supTagPos == -1) break;
			midashi.replace(supTagPos, 5, "\", {\"tag\": \"span\", \"style\": {\"fontSize\": \"x-small\", \"verticalAlign\": \"super\"}, \"content\": \"");
			midashi.replace(midashi.find("</sup>"), 6, "\"}, \"");
		}
		
		// Handle <i> in midashi
		int iTagPos = 0;
		while(1<2)
		{
			iTagPos = midashi.find("<i>");
			if (iTagPos == -1) break;
			midashi.replace(iTagPos, 3, "\", {\"tag\": \"span\", \"style\": {\"fontStyle\": \"italic\"}, \"content\": \"");
			midashi.replace(midashi.find("</i>"), 4, "\"}, \"");
		}
		
		// Extract honbun from html
		// NOTE: find end of actual honbun div and print the stuff on the right side to check
		//honbun = html.substr(html.find("</div>") + 6);
		// TEMP: using midashi for testing
		honbun = html;
		
		// Handle tags in honbun from left to right until they are all gone
		// NOTE: don't forget the tag positions are no longer accurate after you replace stuff lol
		// TEMP: break after 3 loops, instead of infinitely
		int loopNo = 0;
		while(loopNo<3)
		{
			loopNo++;
			
			// Define tag properties
			string tagType = "";
			string tagAttributes = "";
			string tagContents = "";
			int openTagStart = -1;
			int attributeSpacePos = -1;
			int openTagEnd = -1;
			int closeTagStart = -1;
			
			// Obtain info about tag
			openTagStart = honbun.find("<");
			if (openTagStart != -1)
			{
				openTagEnd = honbun.find(">");
				attributeSpacePos = honbun.find(" ", openTagStart);
				if (attributeSpacePos != -1 && attributeSpacePos < openTagEnd)
				{
					// Has attribute(s)
					tagType = honbun.substr(openTagStart + 1, attributeSpacePos - openTagStart - 1);
					tagAttributes = honbun.substr(attributeSpacePos + 1, openTagEnd - attributeSpacePos - 1);
				}
				else
				{
					// No attribute(s)
					tagType = honbun.substr(openTagStart + 1, openTagEnd - openTagStart - 1);
					attributeSpacePos = -1;
				}
				
				// NOTE: this doesn't handle nested tags...
				closeTagStart = honbun.find("</" + tagType);
				tagContents = honbun.substr(openTagEnd + 1, closeTagStart - openTagEnd - 1);
				
				// NOTE: verify all the tag positions + lengths add up at the end (checksum)
			}
			// Break loop once all tags handled
			else
			{
				break;
			}
			
			// Define tag function flags
			bool fnDelete = false; // deletes the tag itself, while leaving contents
			bool fnNeutralize = false; // TEMP: replaces < with ＜ (for tags that aren't handled yet)
			
			// Set tag function flags & perform operations
			if (tagType == "rn")
			{
				// This tag can always be ignored
				fnDelete = true;
			}
			else if (tagType == "etc")
			{
				
			}
			else
			{
				// TEMP: delete everything to test delete function
				fnDelete = true;
			}
			
			// Perform tag functions
			if (fnDelete == true)
			{
				// Erase closing tag
				honbun.erase(closeTagStart, tagType.length() + 3);
				
				// Erase opening tag
				honbun.erase(openTagStart, openTagEnd - openTagStart + 1);
			}
			
			// TEMP: print tag data to verify
			debugOutput << tagType << ", "
			<< tagAttributes << ", "
			<< tagContents << ", "
			<< openTagStart << ", "
			<< attributeSpacePos << ", "
			<< openTagEnd << ", "
			<< closeTagStart << endl;
		}
		
		// TEMP: nl after each entry's tag data
		debugOutput << endl;
		
		// Print entry to term bank
		// NOTE: probably faster to concat everything and minimize file writes?
		if (seqNo != 1) termBank << ",\n";
		termBank << "[\"" 
			<< kanji << "\",\""
			<< reading << "\",\"\",\""
			<< pos << "\","
			<< score << ",[{\"type\": \"structured-content\", \"content\": [\""
			<< midashi << "\\n"
			<< honbun << "\"]}],"
			<< seqNo << ",\"\"]";
	}
	
	// Write closing bracket
	termBank << "]";
	
	// Close files
	termBank.close();
	debugOutput.close();
	
	return 0;
}
