#include <iostream>
#include <fstream>
using namespace std;

int main()
{
	// Define user range vars
	int startSeq = 0;
	int endSeq = 0;
	
	// Get seqNo range from user
	cout << "startSeq: ";
	cin >> startSeq;
	cout << "endSeq (0 for no cap): ";
	cin >> endSeq;
	cout << "\nExport position:\n";
	
	// Create and open files
	// NOTE: should just take term bank and debug offline, but keep a .old copy and use a local diff program
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
		
		// Break once end of csv is reached, or when endSeq is reached
		// WARNING: with no cap, you will exceed github file size limit...
		if (line == "," || (endSeq > 0 && seqNo > endSeq))
		break;
	
		// Continue if startSeq isn't reached
		if (seqNo < startSeq)
		continue;
		
		// Print progress
		if (seqNo % 10000 == 0)
		cout << seqNo/1000 << "k" << endl;
		
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
		// NOTE: print the stuff on the right side of honbun div to check
		honbun = html.substr(html.find("</div>") + 6);
		
		// Fix broken html in some ALPH entries
		if (alphEntry == true)
		{
			// (Pos is for Position, but that's a funny coincidence lol)
			int brokenPos = honbun.find("<div class=\\\"a_link");
			if (brokenPos != -1)
			honbun.insert(brokenPos, "</a>");
			// Fun fact: my internet is down rn and I just hoped and prayed that insert was a real function
		}
		
		// Handle tags in honbun from left to right until they are all gone
		// NOTE: don't forget the tag positions are no longer accurate after you replace stuff lol
		int loopNo = 0;
		while(1<2)
		{			
			// Define tag properties
			string tagType = "";
			string tagAttributes = "";
			string tagContents = "";
			int openTagStart = -1;
			int attributeSpacePos = -1;
			int openTagEnd = -1;
			int closeTagStart = -1;
			
			// Obtain tag properties
			openTagStart = honbun.find("<");
			// Special case for <br>
			if (honbun.find("<br>") != -1 && honbun.find("<br>") == openTagStart)
			{
				tagType = "br";
			}
			// Everything else goes here
			else if (openTagStart != -1)
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
				
				// Traverse heirarchy to find closeTagStart
				int depth = 1;
				int searchPos = openTagStart + 1;
				while(1<2)
				{
					// Find next <
					searchPos = honbun.find("<", searchPos) + 1;
					
					// If you reach a <br>, just ignore it
					if (honbun[searchPos] == 'b' &&
						honbun[searchPos + 1] == 'r' &&
						honbun[searchPos + 2] == '>')
					{
						continue;
					}
					// If it isn't followed by /, you have gone in a layer
					else if (honbun[searchPos] != '/')
					{
						depth++;
					}
					// If it is, you have come out a layer
					else if (honbun[searchPos] == '/')
					{
						depth--;
						
						// Break once you reach closeTagStart (and verify it's the right tag, for safety)
						if (depth == 0 && honbun.find(tagType, searchPos) - searchPos == 1)
						{
							closeTagStart = searchPos - 1;
							break;
						}
					}
				}
				
				tagContents = honbun.substr(openTagEnd + 1, closeTagStart - openTagEnd - 1);
				
				// NOTE: verify all the tag positions + lengths add up at the end (checksum)?
			}
			// Break loop once all tags handled
			else
			{
				break;
			}
			
			// Define tag function flags
			// NOTE: Add a fn to replace open/close tags with specified strings?
			bool fnDelete = false;
			bool fnNeutralize = false;
			
			// Set tag function flag
			// NOTE: check for all tagAttributes varieties etc
			if (tagType == "rn")
			{
				// Not sure what this does, but I don't think it's useful to us
				fnDelete = true;
			}
			else if (tagType == "br")
			{
				// Used for newlines in xref sections, image sections, etc
				// NOTE: move this to replacement function
				honbun.replace(openTagStart, 4, "\\n");
			}
			else if (tagType == "div")
			{
				if (tagAttributes == "class=\\\"honbun\\\"")
				{
					// Encapsulate the whole entry (except midashi)
					// Fun Fact: the only entries with content outside the honbun
					//           div are those broken a_link ALPH entries
					// NOTE: do you even need this outer div at all? just delete?
					// NOTE: maybe check how that works in those broken ALPH entries though
					fnNeutralize = true;				
				}
				else if (tagAttributes == "style=\\\"margin-left:1em;\\\"")
				{
					// Encapsulate senses etc in entry (this is the juicy stuff rh, mwah～)
					fnNeutralize = true;
				}
				else if (tagAttributes == "class=\\\"media\\\"")
				{
					// Exclusively contains FIGc and FIGs class <img> tags
					fnNeutralize = true;
				}
				else if (tagAttributes == "class=\\\"a_link\\\"")
				{
					// Exclusively contain the <a> href tags in those broken a_link ALPH entries
					fnNeutralize = true;
				}
				else if (tagAttributes == "class=\\\"oyko_link\\\"")
				{
					// Encapsulate those long xref lists that use yajirusi2.svg
					fnNeutralize = true;
				}
			}
			else if (tagType == "a")
			{
				if (tagAttributes.find("href=\\\"lved.dataid:") != -1)
				{
					// NOTE: these are xrefs; underline and make blue?
					// NOTE: all hrefs have contents and all non-hrefs have no contents
					// NOTE: check how all the varieties look (check contents)
					fnNeutralize = true;
				}
				else if (tagAttributes.find("name=\\\"0") != -1)
				{
					// There's one of these before every sense etc
					// Maybe they are internal links? Not useful to us, so delete
					fnDelete = true;
				}
				else if (tagAttributes.find("name=\\\"y") != -1)
				{
					// One of these before each example sentence
					fnDelete = true;
				}
			}
			else if (tagType == "q")
			{
				fnNeutralize = true;
			}
			else if (tagType == "sub")
			{
				fnNeutralize = true;
			}
			else if (tagType == "下")
			{
				// NOTE: p sure you can treat this just like <sub>
				fnNeutralize = true;
			}
			else if (tagType == "sup")
			{
				fnNeutralize = true;
			}
			else if (tagType == "object")
			{
				fnNeutralize = true;
			}
			else if (tagType == "img")
			{
				fnNeutralize = true;
			}
			
			// Perform tag functions
			// deletes the tag itself, while leaving contents
			if (fnDelete == true)
			{
				// Do the closing tag first, so the indexes for the other one don't get shifted...
				honbun.erase(closeTagStart, tagType.length() + 3);
				honbun.erase(openTagStart, openTagEnd - openTagStart + 1);
			}
			// TEMP: replaces <> with ＜＞ (for tags that aren't handled yet)
			else if (fnNeutralize == true)
			{
				// Again, go back to front to not shift indexes
				honbun.replace(closeTagStart + tagType.length() + 2, 1, "＞");
				honbun.replace(closeTagStart, 1, "＜");
				honbun.replace(openTagEnd, 1, "＞");
				honbun.replace(openTagStart, 1, "＜");
			}
			/*
			// TEMP: print tag data to verify
			debugOutput << tagType << ", "
			<< tagAttributes << ", "
			<< tagContents << ", "
			<< openTagStart << ", "
			<< attributeSpacePos << ", "
			<< openTagEnd << ", "
			<< closeTagStart << endl;*/
		}
		/*
		// TEMP: nl after each entry's tag data
		debugOutput << endl;*/
		
		// Print entry to term bank
		// NOTE: probably faster to concat everything and minimize file writes?
		if (seqNo != 1 && seqNo != startSeq) termBank << ",\n";
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
	
	cout << endl << "Done!!";
	
	return 0;
}
