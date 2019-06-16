#include "autocomplete.h"
#include <stack>
#include <string>
#include <iostream>
#include <utility>
#include <unordered_set>

#define MAX_SUGGESTIONS_DEFAULT 5

Autocomplete::Autocomplete()
	: m_maxSuggestions(MAX_SUGGESTIONS_DEFAULT)
	, m_nodeCount(1)
{
	m_root = new Node;
}

Autocomplete::~Autocomplete()
{
	free(m_root);
}

void Autocomplete::insert(const wchar_t *str)
{
	Node *current = m_root;

	while (*str)
	{
		Node *next = getNextNode(current, *str);

		if (next == nullptr)
			break;
		else
			current = next;

		++str;
	}

	while (*str)
	{
		Node *new_node = new Node;
		current->children.push_back({ *str, new_node });
		current = new_node;
		++m_nodeCount;

		++str;
	}

	current->isEndOfWord = true;
}

void Autocomplete::suggest(const wchar_t *prefix)
{
	Node *current = m_root;
	const wchar_t *tmp = prefix;

	while (*tmp)
	{
		Node *next = getNextNode(current, *tmp);

		if (next == nullptr)
		{
			current = m_root;
			break;
		}
		else
			current = next;

		++tmp;
	}

	if (current == m_root)
		return;

	printSuggestions(current, prefix);
}

void Autocomplete::minimize()
{
	vector<MinimizeHelp> vec1(m_nodeCount);
	vector<MinimizeHelp> vec2(m_nodeCount);
	vector<MinimizeHelp *> eq_class_representatives;

	putTreeInVector(vec1);

	vec2 = vec1;

	vector<MinimizeHelp> &ref1 = vec1;
	vector<MinimizeHelp> &ref2 = vec2;
	bool has_changed;

	do
	{
		eq_class_representatives.clear();
		has_changed = false;

		for (size_t i = 0; i < m_nodeCount; ++i)
		{
			bool eq_class_found = false;

			for (size_t j = 0; j < eq_class_representatives.size(); ++j)
			{
				if (ref1[i].eq_class == eq_class_representatives[j]->eq_class)
				{
					if (areEquivalent(ref1[i].p_node, eq_class_representatives[j]->p_node, ref1))
					{
						ref2[i].eq_class = j;
						eq_class_found = true;
						break;
					}
					else
						has_changed = true;
				}
			}

			if (!eq_class_found)
			{
				ref2[i].eq_class = eq_class_representatives.size();
				eq_class_representatives.push_back(&ref1[i]);
			}
		}

		swap(ref1, ref2);
	} while (has_changed);

	buildNewTree(ref1, eq_class_representatives.size());

	m_nodeCount = eq_class_representatives.size();
}

void Autocomplete::setMaxSuggestions(size_t m)
{
	m_maxSuggestions = m;
}


size_t Autocomplete::getNodeCount() const
{
	return m_nodeCount;
}

Autocomplete::Node *Autocomplete::getNextNode(Node *current, wchar_t c)
{
	for (size_t i = 0; i < current->children.size(); ++i)
		if (current->children[i].c == c)
			return current->children[i].p_node;

	return nullptr;
}

void Autocomplete::printSuggestions(Node * root, const wchar_t * prefix)
{
	struct Tmp
	{
		Node *p_node;
		size_t idx;
	};

	size_t suggest_cnt = 0;
	wstring word = prefix;
	stack<Tmp> rec;

	rec.push({ root, 0 });

	while (!rec.empty() && suggest_cnt < m_maxSuggestions)
	{
		Tmp current = rec.top();

		if (current.idx == current.p_node->children.size())
		{
			rec.pop();
			word.pop_back();
		}
		else
		{
			++rec.top().idx;
			word.push_back(current.p_node->children[current.idx].c);
			rec.push({ current.p_node->children[current.idx].p_node, 0 });

			if (rec.top().p_node->isEndOfWord)
			{
				wcout << word << endl;
				++suggest_cnt;
			}
		}
	}
}

void Autocomplete::putTreeInVector(vector<MinimizeHelp> &vec)
{
	struct Tmp
	{
		Node *p_node;
		size_t idx;
		bool added;
	};

	stack<Tmp> rec;
	size_t counter = 0;
	rec.push({ m_root, 0, false });

	while (!rec.empty())
	{
		Tmp current = rec.top();

		if (current.added == false)
		{
			if (current.p_node->isEndOfWord)
				vec[counter++] = { current.p_node, 1 };
			else
				vec[counter++] = { current.p_node, 0 };

			rec.top().added = true;	
		}

		if (current.idx == current.p_node->children.size())
		{
			rec.pop();
		}
		else
		{
			++rec.top().idx;
			rec.push({ current.p_node->children[current.idx].p_node, 0 });
		}
	}
}

size_t Autocomplete::findInHelpVec(Node *what, vector<MinimizeHelp> &vec)
{
	for (size_t i = 0; i < vec.size(); ++i)
		if (vec[i].p_node == what)
			return i;

	//should not be able to reach this point
	return 0;
}

bool Autocomplete::areEquivalent(Node *a, Node *b, vector<MinimizeHelp> &vec)
{
	if (a->children.size() != b->children.size())
		return false;

	for (size_t i = 0; i < a->children.size(); ++i)
	{
		wchar_t curr_char = a->children[i].c;
		bool found = false;

		for (size_t j = 0; j < b->children.size(); ++j)
		{
			if (b->children[j].c == curr_char)
			{
				size_t idx_a = findInHelpVec(a->children[i].p_node, vec);
				size_t idx_b = findInHelpVec(b->children[j].p_node, vec);

				if (vec[idx_a].eq_class != vec[idx_b].eq_class)
					return false;

				found = true;
				break;
			}
		}

		if (!found)
			return false;
	}

	return true;
}

void Autocomplete::buildNewTree(vector<MinimizeHelp> &vec, size_t new_node_count)
{
	struct Tmp
	{
		Node * new_n;
		Node * old_n;
	};

	Node *new_root = nullptr;
	vector<Tmp> new_nodes(new_node_count);

	for (size_t i = 0; i < new_node_count; ++i)
		new_nodes[i].new_n = nullptr;

	for (size_t i = 0; i < vec.size(); ++i)
	{
		if (new_nodes[vec[i].eq_class].new_n == nullptr)
		{
			new_nodes[vec[i].eq_class].new_n = new Node;
			new_nodes[vec[i].eq_class].old_n = vec[i].p_node;
			new_nodes[vec[i].eq_class].new_n->isEndOfWord = vec[i].p_node->isEndOfWord;

			if (vec[i].p_node == m_root)
				new_root = new_nodes[vec[i].eq_class].new_n;
		}
	}

	for (size_t i = 0; i < new_node_count; ++i)
	{
		Node *tmp = new_nodes[i].old_n;

		for (size_t j = 0; j < tmp->children.size(); ++j)
		{
			wchar_t c = tmp->children[j].c;
			size_t idx = findInHelpVec(tmp->children[j].p_node, vec);
			Node *n = new_nodes[vec[idx].eq_class].new_n;

			new_nodes[i].new_n->children.push_back({ c, n });
		}
	}

	free(m_root);
	m_root = new_root;
}

void Autocomplete::free(Node *node)
{
	struct Tmp
	{
		Node *p_node;
		size_t idx;
	};

	stack<Tmp> rec;
	rec.push({ node, 0 });

	unordered_set<Node *> deleted;

	while (!rec.empty())
	{
		Tmp current = rec.top();

		if (deleted.find(current.p_node) != deleted.end())
		{
			rec.pop();
			continue;
		}

		if (current.idx == current.p_node->children.size())
		{
			if (deleted.find(current.p_node) == deleted.end())
			{
				delete current.p_node;
				deleted.insert(current.p_node);
			}

			rec.pop();
		}
		else
		{
			++rec.top().idx;
			rec.push({ current.p_node->children[current.idx].p_node, 0 });
		}
	}
}
