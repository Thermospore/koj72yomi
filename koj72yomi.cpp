#include <iostream>
#include <fstream>
#include <queue>
#include <vector>
using namespace std;

int main()
{
	// (this is just used for debugging gaiji)
	int lastGaijiTest = -1;
	
	// Define loan word origins
	const int loanOriginSize = 46;
	const string loanOrigin [loanOriginSize] = {
		"ラテン",
		"梵",
		"ドイツ",
		"ヒンディー",
		"フランス",
		"ギリシア",
		"ペルシア",
		"アメリカ",
		"ベトナム",
		"アラビア",
		"イタリア",
		"ロシア",
		"イギリス",
		"ポルトガル",
		"スペイン",
		"デンマーク",
		"アフリカーンス",
		"オランダ",
		"インドネシア",
		"トルコ",
		"フラマン",
		"モンゴル",
		"エスペラント",
		"マレー",
		"スウェーデン",
		"カタルニア",
		"ヘブライ",
		"カンボジア",
		"アイスランド",
		"タイ",
		"バスク",
		"ノルウェー",
		"フィンランド",
		"ビルマ",
		"ネパール",
		"パーリ",
		"チベット",
		"ダリー",
		"クロアチア",
		"タングート",
		"ハンガリー",
		"タガログ",
		"ブリヤート",
		"ベラルーシ",
		"ハワイ",
		"ルーマニア"};
	
	// Define junk　string list (used for loan source word removal)
	const int junkSize = 98;
	const string junk [junkSize] = {
		"（", "）",
		"Ā", "Á", "Ä", "Å",
		"ā", "ä", "á", "à", "â", "ã", "ă", "ạ",
		"Ç", "Č",
		"ć", "ç", "č",
		"ḍ",
		"É", "Ē",
		"ē", "é", "è", "ê", "ę", "ė", "ë", "ě",
		"ğ",
		"Ḥ",
		"ḥ",
		"İ", "Ī",
		"î", "ī", "ï", "í", "ı",
		"Ł",
		"ł",
		"ṃ",
		"ń", "ñ", "ň", "ṅ", "ṇ",
		"Ö", "Ø", "Ō",
		"ō", "ö", "ó", "ø", "ŏ", "õ", "ô", "ò", "ő",
		"Ṛ",
		"ř", "ṛ",
		"Ś", "Š", "Ṣ",
		"ś", "š", "ş", "ș", "ṣ",
		"Ṭ",
		"ț", "ṭ",
		"Ú", "Ū", "Ü",
		"û", "ū", "ü", "ú", "ù",
		"ÿ", "ý",
		"Ż", "Ẓ",
		"ž", "ẓ", "ź",
		"œ",
		"α", "γ", "ß", "β",
		"‘", "’", "ʼ", "ʻ"};
	
	// Get seqNo range from user
	int startSeq = 0;
	int endSeq = 0;
	cout << "Due to file size limits (and copyright concerns), only have the first 300 entries exported when making commits."
		<< "\n\nstartSeq: ";
	cin >> startSeq;
	cout << "endSeq (0 for no cap): ";
	cin >> endSeq;
	cout << "\nExport position:\n";
	
	// Create and open output files
	ofstream termBank("term_bank_1.json");
	ofstream debugOutput("debugOutput.txt");
	
	// Read in icon map
	vector<string> iconFind;
	vector<string> iconReplace;
	ifstream iconMap("map_icon.txt");
	string line;
	int yaj2Pos = -1;
	int ALPHPos = -1;
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
		
		// Store some mapping indexes for easy reference later
		if (iconFind.back() == "yajirusi2.svg")
			yaj2Pos = iconReplace.size() - 1;
		if (iconFind.back() == "ALPH.svg")
			ALPHPos = iconReplace.size() - 1;
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
		string titleTag = "";
		string pos = "";
		int score = 0;
		
		// Extract Title and Html
		int separatorPos = line.find(",\"<rn></rn><a name=\"\"");
		title = line.substr(0, separatorPos);
		html = line.substr(separatorPos + 2, line.length() - separatorPos - 3);
		
		// Detect if phrase
		bool phraseEntry = (title.find("○") == -1) ? false : true;
		
		// Detect if ALPH entry
		bool alphEntry = (title.find("ALPH.svg") == -1) ? false : true;
		
		// Extract that（）tag thing that isn't included in the midashi
		// Watch out for:
		//		（ in【】ie アース‐カラー 【earth colo（u）r】
		//		stuff like（地名（京都府））
		//		titles in quotes ie "てならいこ【手習子】<sub class=""rubi"">‥ナラヒ‥</sub>（作品名）"
		int startPos = title.find("】") == -1 ?
				title.find("（") : title.find("（", title.find("】"));
		if (startPos != -1)
		{
			// Grab titleTag from end of title
			titleTag = title.substr(startPos);
			
			// Remove " from end, if needed
			if (titleTag.find("\"") != -1)
				titleTag = titleTag.substr(0, titleTag.length() - 1);
		}
		
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
		
		// Remove ‐s from title (not to be confused with -, jfc)
		int hyphPos = 0;
		while(1<2)
		{
			hyphPos = title.find("‐");
			if (hyphPos == -1) break;
			title.replace(hyphPos, 3, "");
		}
				
		// Extract reading and kanji from title
		// Check for 【】 brackets
		int brackPos = title.find("【");
		if (brackPos == -1) 
		{
			// No 【】 brackets; kana only
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
			// Yes 【】 brackets; kanji + reading
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
				fnOpenReplace = "\", {\"tag\": \"br\"}, \"";
			}
			else if (tagType == "div")
			{
				if (tagAttributes == "class=\\\"midashi\\\"")
				{
					// Encapsulates the headword. Dropping the structured div
					// (otherwise you can't have dict name and midashi on same line)
					// Insert the titleTag here, which we extracted from the title earlier
					// (ie（地名）in あいおい 【相生】)
					fnOpenReplace = "\"";
					fnCloseReplace = titleTag + "\", ";
					
					// Add ALPH.svg icon mapping for ALPH entries
					// (originally it was in the title, not the midashi, so you couldn't see it)
					if(alphEntry)
						fnOpenReplace += iconReplace[ALPHPos];
					
					// Add readings for phrase entries
					if (phraseEntry == true)
					{
						// Skip phrases that are kana only
						int readingPos = tagContents.find("<sub>");
						if (readingPos != -1)
						reading = tagContents.substr(readingPos + 5, tagContents.length() - readingPos - 11);
					}
					
					// Add readings for ALPH entries
					// NOTE: still need to handle WWOOF and CO-OP cases
					// Watch out for:
					//		WWOOF (ウオフ; ウーフ) // multiple readings. Only 5 entries with this
					//		CO-OP (コープ; coop) // alphabetical reading. Only occurs in this entry
					//		DoS (ドス攻撃) // leaving as is; yomichan doesn't care actually haha
					//		5W1H (ごダブリューいちエッチ) // some of them have hiragana too btw
					if (alphEntry == true)
					{
						int alphReadingPos = tagContents.find("(");
						if (alphReadingPos != -1)
							reading = tagContents.substr(alphReadingPos + 1, tagContents.length() - alphReadingPos - 2);
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
					// These are all indented by 1em
					fnOpenReplace = "\", {\"tag\": \"div\", \"style\": {\"marginLeft\": 1}, \"content\": [\"";
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
						html.find(iconReplace[yaj2Pos], openTagStart - iconReplace[yaj2Pos].length()) == openTagStart - iconReplace[yaj2Pos].length() ||
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
					// 6th ed in ebwin just bolds / different font eg 組合せ, 差集合
					// 7th ed logovista puts in「」but they don't really know shit
					// Looking at a physical copy of the 5th ed, they put everything in a math-y looking font?
					//		eg in うんどう‐エネルギー 【運動エネルギー】, イー 【E・e】, うら‐がね 【裏曲・裏矩】, えんすい‐めん 【円錐面】
					//		but that might just be how their latin letters normally look, idk
					// NOTE: I'm just going to put in「」for now
					fnOpenReplace = "「";
					fnCloseReplace =  "」";
				}
				else if (tagAttributes == "class=\\\"kente\\\"")
				{
					// 圏点. Had to go to the library to see what 1 vs 2 should look like lol
					// 6th edition in ebwin just bolds for this eg 沓冠; 序詞冠
					
					// Loop to add a 圏点 for each char
					string kente = "";
					for(int k = tagContents.length() / 3; k > 0; k--)
						kente += "●";
					
					fnOpenReplace = "\", {\"tag\": \"ruby\", \"content\": [\"";
					fnCloseReplace = "\", {\"tag\": \"rp\", \"content\": \"(\"}, {\"tag\": \"rt\", \"content\": \"" + kente + "\"}, {\"tag\": \"rp\", \"content\": \")\"}]}, \"";
				}
				else if (tagAttributes == "class=\\\"kenten2\\\"")
				{
					// This only exists in entry for 沓冠
					// 6th ed in ebwin doesn't show any sign of it
					fnOpenReplace = "\", {\"tag\": \"ruby\", \"content\": [\"";
					fnCloseReplace = "\", {\"tag\": \"rp\", \"content\": \"(\"}, {\"tag\": \"rt\", \"content\": \"○\"}, {\"tag\": \"rp\", \"content\": \")\"}]}, \"";
				}
				else if (tagAttributes == "class=\\\"bousen1\\\"")
				{
					// Gonna guess as in 傍線/棒線. Goes on top, not bottom
					fnOpenReplace = "\", {\"tag\": \"span\", \"style\": {\"textDecorationLine\": \"overline\"}, \"content\": [\"";
					fnCloseReplace = "\"]}, \"";
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
					fnOpenReplace = "\", {\"tag\": \"span\", \"style\": {\"fontSize\": \"x-small\", \"verticalAlign\": \"bottom\"}, \"content\": \"";
					fnCloseReplace = "\"}, \"";
				}
				else if (tagAttributes == "")
				{
					// Plain ol subscript. Technically making this `bottom`, not sub (to match rubi)
					fnOpenReplace = "\", {\"tag\": \"span\", \"style\": {\"fontSize\": \"x-small\", \"verticalAlign\": \"bottom\"}, \"content\": \"";
					fnCloseReplace = "\"}, \"";
				}
			}
			else if (tagType == "下")
			{
				// Seems to be used exactly the same as the sub with no attributes
				// Fun Fact: literally the only two entries that have this are LC50 and LD50
				fnOpenReplace = "\", {\"tag\": \"span\", \"style\": {\"fontSize\": \"x-small\", \"verticalAlign\": \"bottom\"}, \"content\": \"";
				fnCloseReplace = "\"}, \"";
			}
			else if (tagType == "sup")
			{
				// Superscript
				fnOpenReplace = "\", {\"tag\": \"span\", \"style\": {\"fontSize\": \"x-small\", \"verticalAlign\": \"super\"}, \"content\": \"";
				fnCloseReplace =  "\"}, \"";
			}
			else if (tagType == "i")
			{
				// Italic
				// Fun Fact: literally the only two entries that have this are NOx and SOx
				fnOpenReplace = "\", {\"tag\": \"span\", \"style\": {\"fontStyle\": \"italic\"}, \"content\": \"";
				fnCloseReplace = "\"}, \"";
			}
			else if (tagType == "strike")
			{
				// Tag I added myself for a single pair of gaiji lol (E535.svg and E536.svg)
				fnOpenReplace = "\", {\"tag\": \"span\", \"style\": {\"textDecorationLine\": \"line-through\"}, \"content\": [\"";
				fnCloseReplace = "\"]}, \"";
			}
			else if (tagType == "object")
			{
				if (tagAttributes.find("class=\\\"gaiji\\\" data=\\\"") != -1)
				{
					// Map from external gaiji text file
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
							else if (gaijiReplace[i].find(".svg") != -1)
							{
								fnOpenReplace = "\", {\"tag\": \"img\", \"path\": \"" + gaijiReplace[i];
								fnCloseReplace = "\", \"width\": 1, \"height\": 1, \"background\": false, \"appearance\": \"monochrome\", \"collapsible\": false, \"collapsed\": false, \"sizeUnits\": \"em\"}, \"";
							}
							// Unicode mapping
							else
							{
								fnAllReplace = gaijiReplace[i];
								// DEBUG: Uncomment this to generate a dict for checking gaiji mappings
								//		+ a list of example entries to search
								/*
								fnOpenReplace = "\", {\"tag\": \"img\", \"path\": \"gaiji/" + gaijiFind[i];
								fnCloseReplace = "\", \"width\": 1, \"height\": 1, \"background\": false, \"appearance\": \"monochrome\", \"collapsible\": false, \"collapsed\": false, \"sizeUnits\": \"em\"}, \"" + gaijiReplace[i];
								if (i > lastGaijiTest)
								{
									debugOutput << gaijiFind[i]<< " / " << gaijiReplace[i] << " / " << kanji << " / " << reading << endl;
									lastGaijiTest = i;
								}
								*/
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
					// NOTE: extract these from logovista 7th ed
					fnOpenReplace = "＜Photo not net extracted. ";
					fnCloseReplace = "＞";
				}
				else if (tagAttributes.find("class=\\\"FIGm\\\" src=\\\"") != -1)
				{
					// These all seem to be mathematical figures. tagContents empty
					// They get plopped inline, not in a div
					// Basically gaiji; look at 陰関数 in 6th ed to compare
					// NOTE: extract these from logovista 7th ed
					fnOpenReplace = "＜Mathematical figure not yet extracted.";
					fnCloseReplace = "＞";
				}
				else if (tagAttributes.find("class=\\\"FIGs\\\" src=\\\"") != -1)
				{
					// Diagrams. Empty tagContents
					// NOTE: extract these from logovista 7th ed
					fnOpenReplace = "＜Diagram not yet extracted.";
					fnCloseReplace = "＞";
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
		bool vzFlag = false;
		bool vkFlag = false;
		bool adjiFlag = false;
		
		// Extract PoS info
		// Referencing these:
		//		file:///C:/Program%20Files%20(x86)/LogoVista/LVEDBRSR/DIC/KOJIEN7/HANREI/contents/ryakugo.html
		//		https://discord.com/channels/841492951644373012/841497050058326046/868485931525623869
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
				
				// epi: "四段 are matched correctly when tagged as v5"
				// May as well include these
				posRaw == "四" ||
				posRaw == "自四" ||
				posRaw == "自他四" ||
				posRaw == "他四" ||
				
				// epi says these will match correctly as v5
				posRaw == "自ナ変")
			{
				v5Flag = true;
			}
			// vs: suru verb & vz: zuru verb
			else if (posRaw == "サ変" ||
				posRaw == "自サ変" ||
				posRaw == "自他サ変" ||
				posRaw == "他サ変")
			{
				// EG: 刊する (かんする)
				if (kanji.find("為る", kanji.length() - 6) == kanji.length() - 6 ||
					kanji.find("する", kanji.length() - 6) == kanji.length() - 6)
				{
					vsFlag = true;
				}
				// EG: 観ずる (かんずる)
				else if (kanji.find("ずる", kanji.length() - 6) == kanji.length() - 6)
				{
					vzFlag = true;
				}
				// EG: 燗す (かんす)
				else if (kanji.find("す", kanji.length() - 3) == kanji.length() - 3)
				{
					// epi: "most of those have become either v5 or vs: 愛す > 愛す 帰す > 帰する.
					//		It's just like 1, it will match when labelled as v5."
					v5Flag = true;
				}
				// EG: 願ず (がんず)
				else if (kanji.find("ず", kanji.length() - 3) == kanji.length() - 3)
				{
					// suppose v5 wouldn't do anything, so no PoS
				}
				// 為 (す) is the only remaining result
				else
				{
					// no PoS
				}
			}
			// vk: kuru verb
			else if (posRaw == "自カ変")
			{
				// EG: 行って来る (いってくる)
				if (kanji.find("来る", kanji.length() - 6) == kanji.length() - 6)
				{
					vkFlag = true;
				}
				// Remainder is some ancient stuff that ends in `来`
				else
				{
					// no PoS
				}
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
				posRaw == "助動" ||
				
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
				posRaw == "名" ||
				
				// epi: "二段 is a mixed bag when tagged as v5 (verbs in づ, ふ and ゆ are an issue)"
				// Suppose I'll just leave these out
				posRaw == "上二" ||
				posRaw == "自上二" ||
				posRaw == "自他上二" ||
				posRaw == "他上二" ||
				posRaw == "下二" ||
				posRaw == "自下二" ||
				posRaw == "自他下二" ||
				posRaw == "他下二")
			{
				// no PoS
			}
			// Otherwise there is some sort of error
			else
			{
				debugOutput << posRaw << "; " << kanji << " (" << reading << ")" << endl;
			}
		}
		
		// Blindly toss PoS tags onto phrase ○ entries with applicable endings
		//		Koj doesn't include PoS info for these
		//		unlikely to mismatch since it's a whole phrase
		if (phraseEntry == true)
		{
			if (kanji.find("る", kanji.length() - 3) == kanji.length() - 3)
				v1Flag = true;
			
			if (kanji.find("う", kanji.length() - 3) == kanji.length() - 3 ||
				kanji.find("く", kanji.length() - 3) == kanji.length() - 3 ||
				kanji.find("ぐ", kanji.length() - 3) == kanji.length() - 3 ||
				kanji.find("す", kanji.length() - 3) == kanji.length() - 3 ||
				kanji.find("つ", kanji.length() - 3) == kanji.length() - 3 ||
				// EG: 門を出づ(かどをいず)
				// Don't think yomichan even has handling for these atm,
				//		but koj has some v5 entries like this EG: 漬づ・沾づ(ひず)
				kanji.find("づ", kanji.length() - 3) == kanji.length() - 3 ||
				kanji.find("ぬ", kanji.length() - 3) == kanji.length() - 3 ||
				kanji.find("ふ", kanji.length() - 3) == kanji.length() - 3 ||
				kanji.find("ぶ", kanji.length() - 3) == kanji.length() - 3 ||
				kanji.find("む", kanji.length() - 3) == kanji.length() - 3 ||
				kanji.find("る", kanji.length() - 3) == kanji.length() - 3)
				v5Flag = true;
			
			if (kanji.find("する", kanji.length() - 6) == kanji.length() - 6)
				vsFlag = true; // There are no instances of 為る(する) btw
			
			if (kanji.find("ずる", kanji.length() - 6) == kanji.length() - 6)
				vzFlag = true;
			
			if (kanji.find("来る", kanji.length() - 6) == kanji.length() - 6 ||
				kanji.find("くる", kanji.length() - 6) == kanji.length() - 6)
				vkFlag = true;
			
			if (kanji.find("い", kanji.length() - 3) == kanji.length() - 3)
				adjiFlag = true;
		}
		
		// Print PoS flags to `pos` string
		if (v1Flag == true) pos += "v1 ";
		if (v5Flag == true) pos += "v5 ";
		if (vsFlag == true) pos += "vs ";
		if (vzFlag == true) pos += "vz ";
		if (vkFlag == true) pos += "vk ";
		if (adjiFlag == true) pos += "adj-i ";
		
		// Erase the trailing space at the end of pos string
		if (pos.length() > 0)
			pos.erase(pos.length() - 1, 1);
		
		// Remove multiple loan word source symbol `・ ` so they don't get folded out into kanjiQ
		// (the whole chunk (ie ` ポルトガルスペイン`) will get removed later)
		// Ex: サクラメント 【sacramento ポルトガル・ スペイン・ イタリア・sacrament イギリス】
		while(1<2)
		{
			int multiSourceLoc = kanji.find("・ ");
			if (multiSourceLoc != -1)
			{
				kanji.erase(multiSourceLoc, 4);
			}
			else
			{
				break;
			}
		}
			
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
		
		// Cycle through kanjiQ and remove loan origin (ie ドイツ)
		// (why did I use a queue lol)
		// Watch out for this: カタログ 【catalogue フランス・ イギリス・catalog アメリカ・型録】
		for(int i = 0; i < kanjiQ.size(); i++)
		{
			// Narrow down to kanjiQ listings that have a space in them
			if (kanjiQ.front().find(" ") != -1)
			{
				// Loop through loanWordOrigins list
				for (int j = 0; j < loanOriginSize; j++)
				{
					// If found, erase loan origin
					// (and don't check the rest of the loanOrigin list)
					int loanOriginPos = kanjiQ.front().find(" " + loanOrigin[j]);
					if (loanOriginPos != -1)
					{
						kanjiQ.front().erase(loanOriginPos);
						break;
					}
				}
			}
			
			// Cycle through the queue since I made the excellent decision of using a queue
			if (kanjiQ.size() > 1)
			{
				string temp = kanjiQ.front();
				kanjiQ.pop();
				kanjiQ.push(temp);
			}
		}
		
		// Remove loan source words (ie "class" as kanji for クラス)
		if (!alphEntry)
		{
			// Cycle through each kanji form in kanjiQ
			for(int i = 0; i < kanjiQ.size(); i++)
			{
				bool deleteCurKanji = false;
				
				// Only check for deletion if 
				if (
					// it's not a single char ie "r" or "V" etc
					kanjiQ.front().length() > 1
					// it's not a single char ie "ω" or "β" etc
					&& !(kanjiQ.front().length() == 2 &&
					!isalpha(kanjiQ.front()[0]) &&
					!isalpha(kanjiQ.front()[1]))
					)
				{
					// Shave off stuff we don't care about, to see if anything remains
					string SHAVEITUP = kanjiQ.front();
					
					// Start with single char stuff
					// Loop through each char in the string
					for(int j = 0; j < SHAVEITUP.length(); j++)
					{
						if (isalpha(SHAVEITUP[j]) ||
							isdigit(SHAVEITUP[j]) ||
							SHAVEITUP[j] == ' ' ||
							SHAVEITUP[j] == '.' ||
							SHAVEITUP[j] == '(' ||
							SHAVEITUP[j] == ')' ||
							SHAVEITUP[j] == '-' ||
							SHAVEITUP[j] == ',' ||
							SHAVEITUP[j] == '\'' ||
							SHAVEITUP[j] == '/' ||
							SHAVEITUP[j] == '!')
						{
							SHAVEITUP.erase(j, 1);
							j--;
						}
					}
					
					// Now multi char stuff
					// Loop until it is all gone
					while(1<2)
					{
						bool keepChecking = false;
						
						// Loop through junk string list
						for (int j = 0; j < junkSize; j++)
						{
							// If string found, erase it
							int junkPos = SHAVEITUP.find(junk[j]);
							if (junkPos != -1)
							{
								SHAVEITUP.erase(junkPos,junk[j].length());
								keepChecking = true;
							}
						}
						
						// Break if we didn't find anything that loop
						if (!keepChecking)
							break;
					}
					
					// If nothing remains, flag this kanji form for deletion
					deleteCurKanji = (SHAVEITUP.length() == 0) ? true : false;
					
					// BUT if it was all caps (and digits), we do actually want to keep it haha
					// EX: 4WD, CIF, 3A, WYSIWYG, or LGBT
					if (deleteCurKanji)
					{
						bool allCaps = true;
						for(int j = 0; j < kanjiQ.front().length(); j++)
						{
							if (!isupper(kanjiQ.front()[j]) &&
								!isdigit(kanjiQ.front()[j]))
							{
								allCaps = false;
								break;
							}
						}
						
						if (allCaps)
							deleteCurKanji = false;
					}
				}
				
				// Remove the kanji form, if flagged
				if (deleteCurKanji)
				{
					kanjiQ.pop();
					i--;
				}				
				// Cycle through the queue since I made the excellent decision of using a queue
				else if (kanjiQ.size() > 1)
				{
					string temp = kanjiQ.front();
					kanjiQ.pop();
					kanjiQ.push(temp);
				}
			}
			
			// If we removed all the kanji forms, move the reading to the kanji field
			if (kanjiQ.empty())
			{
				kanjiQ.push(reading);
				reading = "";
			}
		}
		
		// Scrape out empty ""s in html in an incredibly efficient and eloquent manner
		string garboFind = "";
		string garboReplace = "";
		int garboPos = -1;
		while(1<2)
		{
			garboFind = "[\"\", {";
			garboReplace = "[{";
			garboPos = html.find(garboFind);
			if (garboPos != -1)
			{
				html.replace(garboPos, garboFind.length(), garboReplace);
			}
			else
				break;
		}
		while(1<2)
		{
			garboFind = "}, \"\", {";
			garboReplace = "}, {";
			garboPos = html.find(garboFind);
			if (garboPos != -1)
			{
				html.replace(garboPos, garboFind.length(), garboReplace);
			}
			else
				break;
		}
		while(1<2)
		{
			garboFind = "}, \"\"]";
			garboReplace = "}]";
			garboPos = html.find(garboFind);
			if (garboPos != -1)
			{
				html.replace(garboPos, garboFind.length(), garboReplace);
			}
			else
				break;
		}
		
		// Loop to fold out a copy of the entry for each kanji alt
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
