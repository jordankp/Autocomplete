#ifndef AUTOCOMPLETE_HEADER
#define AUTOCOMPLETE_HEADER

#include <vector>

using namespace std;

class Autocomplete
{
	struct Node
	{
		struct Pair
		{
			wchar_t c;
			Node *p_node;
		};

		vector<Pair> children;
		bool isEndOfWord;

		Node()
			: isEndOfWord(false)
		{}
	};

	struct MinimizeHelp
	{
		Node *p_node;
		size_t eq_class;
	};

	Node *m_root;
	size_t m_maxSuggestions;
	size_t m_nodeCount;

public:
	Autocomplete();
	Autocomplete(const Autocomplete&) = delete;
	Autocomplete& operator=(const Autocomplete&) = delete;
	~Autocomplete();

	void insert(const wchar_t *str);
	void suggest(const wchar_t *prefix);
	void minimize();
	void setMaxSuggestions(size_t m);
	size_t getNodeCount() const;
private:
	void free(Node *root);
	Node *getNextNode(Node *current, wchar_t c);
	void printSuggestions(Node *root, const wchar_t *prefix);
	void putTreeInVector(vector<MinimizeHelp> &vec);
	size_t findInHelpVec(Node *what, vector<MinimizeHelp> &vec);
	bool areEquivalent(Node *a, Node *b, vector<MinimizeHelp> &vec);
	void buildNewTree(vector<MinimizeHelp> &vec, size_t new_node_count);
};


#endif
