#include <fstream>
#include <iostream>
#include <ctime>
#include "autocomplete.h"

#define MAX_WORD_SIZE 100
#define MINIMIZE

int main()
{
	const char *file_name = "450000.txt";
	Autocomplete A;
	wifstream fin(file_name);
	wchar_t current[MAX_WORD_SIZE];

	if (!fin)
	{
		cout << "Could not open " << file_name << " for reading." << endl;
		return 1;
	}

	clock_t time = clock();

	while (!fin.eof())
	{
		fin >> current;
		A.insert(current);
	}

	fin.close();

	time = clock() - time;
	cout << "Time to construct prefix tree: " << (double)time / CLOCKS_PER_SEC << " secs" << endl;

#ifdef MINIMIZE
	cout << "Nodes before minimization: " << A.getNodeCount() << endl;
	time = clock();

	A.minimize();

	time = clock() - time;
	cout << "Time to minimize prefix tree: " << (double)time / CLOCKS_PER_SEC << " secs" << endl;
	cout << "Nodes after minimization: " << A.getNodeCount() << endl;
#endif

	while (wcscmp(current, L"_quit"))
	{
		wcin.getline(current, MAX_WORD_SIZE);
		system("CLS");

		if (wcscmp(current, L"_set_K") == 0)
		{
			size_t K;
			wcin >> K;
			wcin.ignore();
			A.setMaxSuggestions(K);
		}
		else
		{
			A.suggest(current);
			wcout << current << endl;
		}
	}

	return 0;
}