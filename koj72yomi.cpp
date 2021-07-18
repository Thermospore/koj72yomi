#include <iostream>
#include <fstream>
#include <queue>
#include <vector>
using namespace std;

int main()
{
	// Get seqNo range from user
	int startSeq = 0;
	int endSeq = 0;
	cout << "startSeq: ";
	cin >> startSeq;
	cout << "endSeq (0 for no cap): ";
	cin >> endSeq;
	cout << "\nExport position:\n";
	
	// Create and open output files
	// NOTE: should just take term bank and debug offline, but keep a .old copy and use a local diff program
	ofstream termBank("term_bank_1.json");
	ofstream debugOutput("debugOutput.txt");
	
	// Read in icon map
	vector<string> iconFind;
	vector<string> iconReplace;
	ifstream iconMap("map_icon.txt");
	string line;
	while (getline(iconMap, line))
	{
		// Skip comment lines
		if (line[0] == '#')
			continue;
		
		// Find tab separator
		int tabPos = line.find("\t");
		
		// Store contents
		iconFind.push_back(line.substr(0,tabPos));
		iconReplace.push_back(line.substr(tabPos + 1));
	}
	iconMap.close();
	
	// Read in gaiji map
	vector<string> gaijiFind;
	vector<string> gaijiReplace;
	ifstream gaijiMap("map_gaiji.txt");
	line = "";
	while (getline(gaijiMap, line))
	{
		// Skip comment lines
		if (line[0] == '#')
			continue;
		
		// Find tab separator
		int tabPos = line.find("\t");
		
		// Store contents
		gaijiFind.push_back(line.substr(0,tabPos));
		gaijiReplace.push_back(line.substr(tabPos + 1));
	}
	gaijiMap.close();
	
	// Write opening bracket
	termBank << "[";
	
	// Init sequence number
	int seqNo = 0;
	
	// Read dict line by line
	ifstream koj("KOUJIEN7.csv");
	line = "";
	getline(koj, line); // skip initial "Title,Html" line
	while (1<2)
	{
		// Get next line and bump seqNo
		getline(koj, line);
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
		
		// Init dict csv components
		string title = "";
		string html = "";
		
		// Init term bank components
		string kanji = "";
		string reading = "";
		string pos = "";
		int score = 0;
		
		// Extract Title and Html
		// NOTE: should print all the titles out to check them
		int separatorPos = line.find(",\"<rn></rn><a name=\"\"");
		title = line.substr(0, separatorPos);
		html = line.substr(separatorPos + 2, line.length() - separatorPos - 3);
		
		// Detect if phrase
		bool phraseEntry = (title.find("○") == -1) ? false : true;
		
		// Detect if ALPH entry
		bool alphEntry = (title.find("ALPH.svg") == -1) ? false : true;
		
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
			
			// Handle gaiji objects (can be multiple per line)
			while(1<2)
			{
				// Break if all gaiji handled
				int gaijiPos = title.find("<object class=\\\"gaiji\\\"");
				if (gaijiPos == -1) break;
				
				// Get filename from <object> attributes
				string gaijiFilename = title.substr(gaijiPos + 31, 8);
					
				// Loop until you find it in the list
				int i = 0;
				while(1<2)
				{
					if (gaijiFind[i] == gaijiFilename)
					{
						// SVG mapping. Can't do this for yomichan headword, so replace with �
						if (gaijiReplace[i].find(".svg") != -1)
						{
							title.replace(gaijiPos, 51, "�");
						}
						// Unicode mapping
						else
						{
							title.replace(gaijiPos, 51, gaijiReplace[i]);
						}
						
						break;
					}
					
					i++;
				}
			}
		}
		
		// Remove inconsistent space before `【` in title
		int randoSpacePos = title.find(" 【");
		if (randoSpacePos != -1)
		title.erase(randoSpacePos, 1);
		
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
		
		// Remove ○ from phrase title
		if (phraseEntry == true)
		title.erase(0,3);
		
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
		
		// Fix broken html in some ALPH entries
		if (alphEntry == true)
		{
			// (Pos is for Position, but that's a funny coincidence lol)
			int brokenPos = html.find("<div class=\\\"a_link");
			if (brokenPos != -1)
			html.insert(brokenPos, "</a>");
			// Fun fact: my internet is down rn and I just hoped and prayed that insert was a real function
		}
		
		// Handle tags in html from left to right until they are all gone
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
			openTagStart = html.find("<");
			// Special case for <br>
			if (html.find("<br>") != -1 && html.find("<br>") == openTagStart)
			{
				tagType = "br";
				openTagEnd = openTagStart + 3;
			}
			// Everything else goes here
			else if (openTagStart != -1)
			{
				openTagEnd = html.find(">");
				attributeSpacePos = html.find(" ", openTagStart);
				if (attributeSpacePos != -1 && attributeSpacePos < openTagEnd)
				{
					// Has attribute(s)
					tagType = html.substr(openTagStart + 1, attributeSpacePos - openTagStart - 1);
					tagAttributes = html.substr(attributeSpacePos + 1, openTagEnd - attributeSpacePos - 1);
				}
				else
				{
					// No attribute(s)
					tagType = html.substr(openTagStart + 1, openTagEnd - openTagStart - 1);
					attributeSpacePos = -1;
				}
				
				// Traverse heirarchy to find closeTagStart
				int depth = 1;
				int searchPos = openTagStart + 1;
				while(1<2)
				{
					// Find next <
					searchPos = html.find("<", searchPos) + 1;
					
					// If you reach a <br>, just ignore it
					if (html[searchPos] == 'b' &&
						html[searchPos + 1] == 'r' &&
						html[searchPos + 2] == '>')
					{
						continue;
					}
					// If it isn't followed by /, you have gone in a layer
					else if (html[searchPos] != '/')
					{
						depth++;
					}
					// If it is, you have come out a layer
					else if (html[searchPos] == '/')
					{
						depth--;
						
						// Break once you reach closeTagStart (and verify it's the right tag, for safety)
						if (depth == 0 && html.find(tagType, searchPos) - searchPos == 1)
						{
							closeTagStart = searchPos - 1;
							break;
						}
					}
				}
				
				tagContents = html.substr(openTagEnd + 1, closeTagStart - openTagEnd - 1);
			}
			// Break loop once all tags handled
			else
			{
				break;
			}
			
			// Define tag function flags
			bool fnDelete = false;
			bool fnNeutralize = false;
			string fnOpenReplace = "";
			string fnCloseReplace = "";
			string fnAllReplace = "";
			
			// Set tag function flag
			if (tagType == "rn")
			{
				// Not sure what this does, but I don't think it's useful to us
				fnDelete = true;
			}
			else if (tagType == "br")
			{
				// Used for newlines in xref sections, image sections, etc
				// NOTE: what <br>s exist outside of oyko_link div?
				// NOTE: wait, yomi has actual <br>s dumbo
				fnOpenReplace = "\\n";
			}
			else if (tagType == "div")
			{
				if (tagAttributes == "class=\\\"midashi\\\"")
				{
					// Encapsulates the headword. Dropping the structured div
					// (otherwise you can't have dict name and midashi on same line)
					// NOTE: underline / bold the midashi?
					fnOpenReplace = "\"";
					fnCloseReplace = "\", ";
					
					// Add readings for phrase entries
					if (phraseEntry == true)
					{
						// Skip phrases that are kana only
						int readingPos = tagContents.find("<sub>");
						if (readingPos != -1)
						reading = tagContents.substr(readingPos + 5, tagContents.length() - readingPos - 11);
					}
				}
				if (tagAttributes == "class=\\\"honbun\\\"")
				{
					// Encapsulates the body of the entry
					// Fun Fact: the only entries with content outside the honbun
					//           div are those broken a_link ALPH entries
					fnOpenReplace = "{\"tag\": \"div\", \"content\": [\"";
					fnCloseReplace = "\"]}";
				}
				else if (tagAttributes == "class=\\\"a_link\\\"")
				{
					// Exclusively contain the <a> href tags in those broken a_link ALPH entries
					fnOpenReplace = ", {\"tag\": \"div\", \"content\": [\"";
					fnCloseReplace = "\"]}";
				}
				else if (tagAttributes == "style=\\\"margin-left:1em;\\\"")
				{
					// Encapsulates senses etc in entry (this is the juicy stuff rh, mwah～)
					fnOpenReplace = "\", {\"tag\": \"div\", \"content\": [\"";
					fnCloseReplace = "\"]}, \"";
				}
				else if (tagAttributes == "class=\\\"media\\\"")
				{
					// Exclusively contains FIGc and FIGs class <img> tags
					fnOpenReplace = "\", {\"tag\": \"div\", \"content\": [\"";
					fnCloseReplace = "\"]}, \"";
				}
				else if (tagAttributes == "class=\\\"oyko_link\\\"")
				{
					// Encapsulate those long xref lists that use yajirusi2.svg
					fnOpenReplace = "\", {\"tag\": \"div\", \"style\": {\"fontSize\": \"x-small\"}, \"content\": [\"";
					fnCloseReplace = "\"]}, \"";
					
					// Strip <br> tags (because look at shit like まくら 【枕】or even worse にほん 【日本】)
					// ...while carefully adjusting closeTagStart position lol
					while(1<2)
					{
						int brPos = html.find("<br>", openTagEnd);
						if (brPos != -1 && brPos < closeTagStart)
						{
							html.replace(brPos, 4, " ");
							closeTagStart -= 3;
						}
						else
						{
							break;
						}
					}
				}
			}
			else if (tagType == "a")
			{
				if (tagAttributes.find("href=\\\"lved.dataid:") != -1)
				{
					// Hyperlinks/xrefs. Drop formatting if there is already an arrow indicator
					// (everyone already knows it's an xref, so may as well save the import time)
					if (tagContents.find("→") == 0 ||
						tagContents.find("<sup>(→)</sup>") == 0 ||
						tagContents.find("<object class=\\\"icon\\\" data=\\\"yajirusi1.svg\\\"></object>") == 0 ||
						// NOTE: kinda dangerous; should ref the icon list instead of hardcoding lol
						html.find("➡", openTagStart - 3) == openTagStart - 3 ||
						// These are exclusively the "参照" refs (with a single exception in こ‐じっかり 【小確】)
						(tagContents.find("「") == 0 && tagContents.find("」") == tagContents.length() - 3))
					{
						fnDelete = true;
					}
					// Bold + underline for all other xrefs
					else
					{
						fnOpenReplace = "\", {\"tag\": \"span\", \"style\": {\"fontWeight\": \"bold\", \"textDecorationLine\": \"underline\"}, \"content\": [\"";
						fnCloseReplace =  "\"]}, \"";
					}
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
				if (tagAttributes == "class=\\\"siki\\\"")
				{
					// Gonna guess as in 式
					// How to format for yomichan?? italics? simply print `s on outside?
					// 6th ed in ebwin just bolds / different font? 組合せ, 差集合
					fnNeutralize = true;
				}
				else if (tagAttributes == "class=\\\"kente\\\"")
				{
					// ...圏点?? btw typing that in ime brings some up
					// internet too slow to research 
					// 6th edition in ebwin just bolds for this: 沓冠; 序詞冠
					fnNeutralize = true;
				}
				else if (tagAttributes == "class=\\\"kenten2\\\"")
				{
					// Gonna guess as in 圏点
					// This only exists in entry for 沓冠
					// 6th ed in ebwin doesn't show any sign of it
					fnNeutralize = true;
				}
				else if (tagAttributes == "class=\\\"bousen1\\\"")
				{
					// Gonna guess as in 傍線/棒線. Goes on top, not bottom
					fnOpenReplace = "\", {\"tag\": \"span\", \"style\": {\"textDecorationLine\": \"overline\"}, \"content\": [\"";
					fnCloseReplace =  "\"]}, \"";
				}
			}
			else if (tagType == "sub")
			{
				if (tagAttributes == "class=\\\"rubi\\\"")
				{
					// Furigana
					// There is no way to detect what kanji the furigana should go over, so doing this instead
					//		See: わそう‐コート 【和装コート】
					// Example where having rubi formatting is important アートマン 【ātman 梵】
					// NOTE: how does this look bold? super script?
					fnOpenReplace = "\", {\"tag\": \"span\", \"style\": {\"fontSize\": \"x-small\", \"verticalAlign\": \"bottom\"}, \"content\": \"";
					fnCloseReplace =  "\"}, \"";
				}
				else if (tagAttributes == "")
				{
					// Plain ol subscript. Technically making this `bottom`, not sub (to match rubi)
					fnOpenReplace = "\", {\"tag\": \"span\", \"style\": {\"fontSize\": \"x-small\", \"verticalAlign\": \"bottom\"}, \"content\": \"";
					fnCloseReplace =  "\"}, \"";
				}
			}
			else if (tagType == "下")
			{
				// Seems to be used exactly the same as the sub with no attributes
				// Fun Fact: literally the only two entries that have this are LC50 and LD50
				fnOpenReplace = "\", {\"tag\": \"span\", \"style\": {\"fontSize\": \"x-small\", \"verticalAlign\": \"bottom\"}, \"content\": \"";
				fnCloseReplace =  "\"}, \"";
			}
			else if (tagType == "sup")
			{
				// Superscript
				// NOTE: have a single source for the sup style for this and midashi
				fnOpenReplace = "\", {\"tag\": \"span\", \"style\": {\"fontSize\": \"x-small\", \"verticalAlign\": \"super\"}, \"content\": \"";
				fnCloseReplace =  "\"}, \"";
			}
			else if (tagType == "i")
			{
				// Italic
				// Fun Fact: literally the only two entries that have this are NOx and SOx
				fnOpenReplace = "\", {\"tag\": \"span\", \"style\": {\"fontStyle\": \"italic\"}, \"content\": \"";
				fnCloseReplace =  "\"}, \"";
			}
			else if (tagType == "strike")
			{
				// Tag I added myself for a single pair of gaiji lol (E535.svg and E536.svg)
				fnOpenReplace = "\", {\"tag\": \"span\", \"style\": {\"textDecorationLine\": \"line-through\"}, \"content\": [\"";
				fnCloseReplace =  "\"]}, \"";
			}
			else if (tagType == "object")
			{
				if (tagAttributes.find("class=\\\"gaiji\\\" data=\\\"") != -1)
				{
					// Map from external gaiji text file
					// NOTE: change (i) and (ii) to {i} and {ii} so they stand apart a bit more?
					// NOTE: make E565.svg	＊ <sup>? can just add html right into the gaiji file like with <strike>
					// NOTE: change those cap leters to these?
					//		https://en.wikipedia.org/wiki/Mathematical_Alphanumeric_Symbols
					// NOTE: is A16A really supposed to be F? why did they gaiji it?
					// NOTE: match these to icons file? A428.svg	🈔
					// NOTE: what's up with E1E7; is it supposed to be null?
					// NOTE: AD56.svg	𬮆 doesn't show up for me, might not have good font support?
					//		same with B258.svg	𭸻
					//		same with AE46.svg	𬝟
					//		same with AE6E.svg	𫫠
					// NOTE: not sure how I feel about B925.svg	（縦線二本）＄
					// Get filename from <object> attributes
					string gaijiFilename = tagAttributes.substr(23, tagAttributes.length() - 25);
					
					// Loop until you find it in the list
					int i = 0;
					while(1<2)
					{
						if (gaijiFind[i] == gaijiFilename)
						{
							// Ignore null
							if (gaijiReplace[i] == "null")
							{
								fnDelete = true;
							}
							// SVG mapping. Insert structured content
							// NOTE: try how different imageRendering settings look, since these are vector graphics not images
							else if (gaijiReplace[i].find(".svg") != -1)
							{
								fnOpenReplace = "\", {\"tag\": \"img\", \"path\": \"" + gaijiReplace[i];
								fnCloseReplace = "\", \"width\": 1, \"height\": 1, \"imageRendering\": \"crisp-edges\", \"background\": false, \"appearance\": \"monochrome\", \"collapsible\": false, \"collapsed\": false, \"sizeUnits\": \"em\"}, \"";
							}
							// Unicode mapping
							else
							{
								fnAllReplace = gaijiReplace[i];
							}
							
							break;
						}
						
						i++;
					}
				}
				else if (tagAttributes.find("class=\\\"icon\\\" data=\\\"") != -1)
				{
					// Map from external icons text file
					// Get filename from <object> attributes
					string iconFilename = tagAttributes.substr(22, tagAttributes.length() - 24);
					
					// Loop until you find it in the list
					int i = 0;
					while(1<2)
					{
						if (iconFind[i] == iconFilename)
						{
							fnAllReplace = iconReplace[i];
							break;
						}
						i++;
					}
				}
			}
			else if (tagType == "img")
			{
				if (tagAttributes.find("class=\\\"FIGc\\\" src=\\\"") != -1)
				{
					// These all contain 撮影/提供 credits in tagContents
					// NOTE: look through some entries to see what these are
					fnNeutralize = true;
				}
				else if (tagAttributes.find("class=\\\"FIGm\\\" src=\\\"") != -1)
				{
					// These all seem to be mathematical figures. tagContents empty
					// They get plopped inline, not in a div
					// Basically gaiji; look at 陰関数 in 6th ed to compare
					// NOTE: sure we can't get these?? only 38 of them
					fnNeutralize = true;
				}
				else if (tagAttributes.find("class=\\\"FIGs\\\" src=\\\"") != -1)
				{
					// Empty tagContents
					// NOTE: look through some entries to see what these are
					fnNeutralize = true;
				}
				else if (tagAttributes.find("class=\\\"icon\\\" src=\\\"") != -1)
				{
					// Entirely consists of bungo.png. Empty tagContents
					string iconFilename = "bungo.png";
					
					// Loop until you find it in the list
					int i = 0;
					while(1<2)
					{
						if (iconFind[i] == iconFilename)
						{
							fnAllReplace = iconReplace[i];
							break;
						}
						i++;
					}
				}
			}
			
			// Perform tag functions
			// deletes the tag itself, while leaving contents
			if (fnDelete == true)
			{
				// Do the closing tag first, so the indexes for the other one don't get shifted...
				html.erase(closeTagStart, tagType.length() + 3);
				html.erase(openTagStart, openTagEnd - openTagStart + 1);
			}
			// TEMP: replaces <> with ＜＞ (for tags that aren't handled yet)
			else if (fnNeutralize == true)
			{
				// Again, go back to front to not shift indexes
				html.replace(closeTagStart + tagType.length() + 2, 1, "＞");
				html.replace(closeTagStart, 1, "＜");
				html.replace(openTagEnd, 1, "＞");
				html.replace(openTagStart, 1, "＜");
			}
			// replaces opening and or closing tag with specified string
			else if (fnOpenReplace != "" || fnCloseReplace != "")
			{
				if (fnCloseReplace != "")
				html.replace(closeTagStart, tagType.length() + 3, fnCloseReplace);
				if (fnOpenReplace != "")
				html.replace(openTagStart, openTagEnd - openTagStart + 1, fnOpenReplace);
			}
			// replaces whole tag with specified string
			else if (fnAllReplace != "")
			{
				// Ensure there are no contents
				if (tagContents == "")
				{
					html.erase(closeTagStart, tagType.length() + 3);
					html.replace(openTagStart, openTagEnd - openTagStart + 1, fnAllReplace);
				}
			}
		}
		
		// Define PoS flags
		bool v1Flag = false;
		bool v5Flag = false;
		bool vsFlag = false;
		bool vkFlag = false;
		bool adjiFlag = false;
		
		// Extract PoS info
		// NOTE: Blindly toss verb tags onto phrase ◯ entries with applicable endings?
		//		since you are unlikely to mismatch?
		//		Koj doesn't include PoS info for those
		// Referencing these:
		//		file:///C:/Program%20Files%20(x86)/LogoVista/LVEDBRSR/DIC/KOJIEN7/HANREI/contents/ryakugo.html
		//		https://github.com/FooSoft/yomichan-import/blob/master/koujien.go
		//		https://github.com/FooSoft/yomichan-import/blob/master/edict.go
		// Fun Fact: あ・く 【明く・開く・空く】 and おのれ 【己】 have the most〘PoS〙tags at 4
		int curPos = 0;
		while(1<2)
		{
			// Find start of PoS tag (or break if we've reached the end)
			int opBracPos = html.find("〘", curPos);
			if (opBracPos == -1)
				break;
			
			// Find end of PoS tag and update position
			int closBracPos = html.find("〙", curPos);
			curPos = closBracPos + 3;
			
			// Extract contents
			string posRaw = html.substr(opBracPos + 3, closBracPos - opBracPos - 3);
			
			// Set PoS flags based on contents
			// v1: ichidan verb
			if (posRaw == "自上一" ||
				posRaw == "自他上一" ||
				posRaw == "他上一" ||
				
				posRaw == "自下一" ||
				posRaw == "自他下一" ||
				posRaw == "他下一")
			{
				v1Flag = true;
			}
			// v5: godan verb
			else if (posRaw == "五" ||
				posRaw == "自五" ||
				posRaw == "自他五" ||
				posRaw == "他五" ||
				
				// NOTE: I think these can go here?
				posRaw == "四" ||
				posRaw == "自四" ||
				posRaw == "自他四" ||
				posRaw == "他四" ||
				
				// NOTE: I think these can go here?
				posRaw == "上二" ||
				posRaw == "自上二" ||
				posRaw == "自他上二" ||
				posRaw == "他上二" ||
				
				// NOTE: I think these can go here?
				posRaw == "下二" ||
				posRaw == "自下二" ||
				posRaw == "自他下二" ||
				posRaw == "他下二" ||
				
				// NOTE: Can maybe just throw these here too?
				// 		Full list:
				//		往ぬ・去ぬ (いぬ)
				//		酔ひ死ぬ (えいしぬ)
				//		思ひ死ぬ (おもいしぬ)
				//		恋ひ死ぬ (こいしぬ)
				//		乾死ぬ・干死ぬ (ひしぬ)
				posRaw == "自ナ変")
			{
				v5Flag = true;
			}
			// vs: suru verb
			// NOTE: looks like yomi import limits to `(strings.HasSuffix(term.Expression, "する") || strings.HasSuffix(term.Expression, "為る")`?
			// NOTE: wait does yomichan have a vz tag too?
			//		https://github.com/FooSoft/yomichan-import/blob/35175a5a1ef618847f940767fb94b8ce82c728d0/edict.go#L36
			// TEMP: Some examples of these:
			//		願ず (がんず)
			//		観ずる (かんずる)
			//		燗す (かんす)
			//		刊する (かんする)
			else if (posRaw == "サ変" ||
				posRaw == "自サ変" ||
				posRaw == "自他サ変" ||
				posRaw == "他サ変")
			{
				vsFlag = true;
			}
			// vk: kuru verb
			// NOTE: looks like yomi import koj6 limits to only `term.Expression == "来る"`?
			//		Full list:
			//		在り来 (ありく)
			//		行って来る (いってくる)
			//		出で来 (いでく)
			//		来 (く)
			//		来る (くる)
			//		漕ぎ来る (こぎくる)
			//		立ち来 (たちく)
			//		出来 (でく)
			//		尋め来 (とめく)
			//		参来 (まいく)
			//		参出来 (まいでく)
			//		参り来 (まいりく)
			//		詣で来 (までく)
			//		惑ひ来 (まどいく)
			//		参来 (もうく)
			//		詣で来 (もうでく)
			//		遣って来る (やってくる)
			//		行き来・往き来 (ゆきく)
			//		寄せ来る (よせくる)
			//		寄り来る (よりくる)
			else if (posRaw == "自カ変")
			{
				vkFlag = true;
			}
			// adj-i: i-adjective
			else if (posRaw == "形")
			{
				adjiFlag = true;
			}
			// Leave PoS blank
			else if (posRaw == "形ク" ||
				posRaw == "形シク" ||
				
				posRaw == "自ラ変" ||
				posRaw == "他ラ変" ||
				
				posRaw == "助詞" ||
				posRaw == "助動" || // I think?
				
				posRaw == "接続" ||
				posRaw == "接頭" ||
				posRaw == "接尾" ||
				
				posRaw == "連体" ||
				
				// Interesting: こ・ず 【掘ず】〘他〙（用例は連用形のみ、活用は上二段か四段か不明）
				posRaw == "他" ||
				
				posRaw == "感" ||
				posRaw == "代" ||
				posRaw == "副" ||
				posRaw == "枕" ||
				posRaw == "名")
			{
				// Do absolutely nothing
			}
			// Otherwise there is some sort of error
			else
			{
				debugOutput << posRaw << "; " << kanji << " (" << reading << ")" << endl;
			}
		}
		
		// NOTE: print PoS flags to `pos` string here
		
		// Extract all kanji variants. Has to account for ; and ・
		// ie ツァーリズム 【tsarizm ロシア・czarism; tsarism イギリス】
		queue<string> kanjiQ;
		while(1<2)
		{
			// Find separators
			int jpSepPos = kanji.find("・");
			int alphSepPos = kanji.find(";");
			
			// Extract frontmost variant and place it in the queue (until `kanji` is empty)
			if (jpSepPos != -1 &&
				(alphSepPos == -1 || jpSepPos < alphSepPos))
			{
				kanjiQ.push(kanji.substr(0, jpSepPos));
				kanji.erase(0, jpSepPos + 3);
			}
			else if (alphSepPos != -1 &&
				(jpSepPos == -1 || alphSepPos < jpSepPos))
			{
				kanjiQ.push(kanji.substr(0, alphSepPos));
				kanji.erase(0, alphSepPos + 2);
			}
			else
			{
				kanjiQ.push(kanji);
				kanji.erase(0);
				break;
			}
		}
		
		// NOTE: Remove loan origin here? ie ドイツ
		
		// NOTE: Skip alphabetical kanji forms here (except from ALPH.svg entries)?
		//		Could just delete from the queue if
		//			whole thing matches isalpha()
		//			has more than one or two or three chars?
		//			don't forget the katakana needs to be moved to the kanji for yomichan tho
		
		// Loop to fold out a copy of the entry for each kanji alt
		// NOTE: avoid leaving all those excessive empty ""s between divs etc in structured-content?
		//		maybe just be lazy and loop to remove all ` "",` in html lol
		while(kanjiQ.empty() == false)
		{
			// Close previous line/entry (if applicable)
			if (seqNo != 1 && seqNo != startSeq) termBank << ",\n";
			
			// Print entry to term bank
			termBank << "[\"" 
				<< kanjiQ.front() << "\",\""
				<< reading << "\",\"\",\""
				<< pos << "\","
				<< score << ",[{\"type\": \"structured-content\", \"content\": ["
				<< html << "]}],"
				<< seqNo << ",\"\"]";
			
			// Remove kanji form we've already printed
			kanjiQ.pop();
		}
	}
	
	// Write closing bracket
	termBank << "]";
	
	// Close files
	termBank.close();
	debugOutput.close();
	koj.close();
	
	cout << endl << "Done!!";
	
	return 0;
}
