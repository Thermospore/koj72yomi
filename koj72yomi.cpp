#include <iostream>
#include <fstream>
using namespace std;

int main() {
  // Create and open term bank file
  ofstream termBank("term_bank_1.json");

  // Test write
  termBank << "here is where the yomi dict is gonna go (Wow!!)";

  // Close the file
  termBank.close();

	return 0;
}
