#include <iostream>
#include <fstream>
using namespace std;

int main()
{
	// Create and open term bank file
	ofstream termBank("term_bank_1.json");
	
	// Write opening bracket
	termBank << "[";
	
	// Init term bank components
	string kanji = "";
	string reading = "";
	string pos = "";
	int score = 0;
	string gloss = "";
	int seqNo = 0;
	
	// Read dict line by line
	ifstream file("KOUJIEN7.csv");
	string line;
	while (getline(file, line))
	{
		// Temp: break once limit reached
		seqNo++;
		if (seqNo > 20) break;
		
		// Print entry to term bank
		termBank << "[\"" 
			<< kanji << "\",\""
			<< reading << "\",\"\",\""
			<< pos << "\","
			<< score << ",[\""
			<< gloss << "\"],"
			<< seqNo << ",\"\"]\n";
	}
	
	// Write closing bracket
	termBank << "]";
	
	// Close the file
	termBank.close();
	
	return 0;
}
