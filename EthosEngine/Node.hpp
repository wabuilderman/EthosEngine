#pragma once
#include <string>
#include <vector>

struct NodeTree {
	struct Node {
		size_t self;
		size_t parent;
		std::string name;
		std::vector<size_t> children;
		Node(size_t parent, size_t self, const std::string& name);
	};

	std::vector<Node> nodes;

	const Node& getNode(size_t index) const;
	void addNode(size_t parent, const std::string& name);

	size_t merge(const NodeTree& other, size_t index = 0);

	bool matches(const NodeTree& other) const;

	//static bool nodesMatch(const Node& a, const Node& b);
	int nodeIndexOf(const Node& a, const Node& b) const;
	int nodeIndexOf(const std::string& name, size_t start = 0) const;
	bool nodeContains(const Node& a, const Node& b) const;

	// Returns if the other tree can fit within this tree
	bool isCompatible(const NodeTree& other) const;
};