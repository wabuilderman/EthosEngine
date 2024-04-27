#include "Node.hpp"


NodeTree::Node::Node(size_t parent, size_t self, const std::string& name) :
	parent(parent), self(self), name(name) {

	// Nothing to initialize ... yet

}

void NodeTree::addNode(size_t parent, const std::string& name) {
	size_t index = nodes.size();
	nodes[parent].children.push_back(index);
	nodes.push_back(Node(parent, index, name));
}

const NodeTree::Node& NodeTree::getNode(size_t index) const {
	return nodes[index];
}

// Adds all nodes from the given tree that don't already exist in this one
// Returns number of nodes added
size_t NodeTree::merge(const NodeTree& other, size_t index) {

	bool exists = (nodeIndexOf(other.nodes[index].name) != -1);
	size_t added = 0;
	if (!exists) {
		size_t parent = nodeIndexOf(other.nodes[other.nodes[index].parent].name);
		addNode(parent, other.nodes[index].name);
		added++;
	}

	for (size_t child : other.nodes[index].children) {
		added += merge(other, child);
	}

	return added;
}

bool NodeTree::matches(const NodeTree& other) const {

	if (nodes.size() != other.nodes.size()) {
		return false;
	}
		
	for (size_t i = 0; i < nodes.size(); ++i) {
		if (nodes[i].name.compare(other.nodes[i].name) != 0) {
			return false;
		}
	}

	return true;
}

int NodeTree::nodeIndexOf(const Node& a, const Node& b) const {
	// If this matches the node
	if (a.name.compare(b.name) == 0) {
		// Check if this node contains all it's children
		for (size_t i : b.children) {
			if (!nodeContains(a, getNode(i))) {
				return -1;
			}
		}
		// This is the index of the node
		return (int)a.self;
	}

	// Check if the node exists somewhere within this node's children
	for (size_t i = 0; i < a.children.size(); ++i) {
		int index = nodeIndexOf(getNode(a.children[i]), b);
		if (index != -1) {
			return index;
		}
	}

	// That node doesn't exist as a decendent of this node.
	return -1;
}

int NodeTree::nodeIndexOf(const std::string& name, size_t start) const {

	// If the node tree is empty, it can't contain the node.
	if (nodes.size() == 0) {
		return -1;
	}

	// If this matches the node
	if (name.compare(nodes[start].name) == 0) {
		return (int)start;
	}

	// Check if the node exists somewhere within this node's children
	for (size_t i = 0; i < getNode(start).children.size(); ++i) {
		int index = nodeIndexOf(name, getNode(start).children[i]);
		if (index != -1) {
			return index;
		}
	}

	// That node doesn't exist as a decendent of this node.
	return -1;
}

bool NodeTree::isCompatible(const NodeTree& other) const {
	for (size_t i = 0; i < other.nodes.size(); ++i) {
		if (nodeIndexOf(other.nodes[i].name) == -1) {
			return false;
		}
	}
	return true;
}

bool NodeTree::nodeContains(const Node& a, const Node& b) const {
	if (a.name.compare(b.name) != 0) {
		return false;
	}

	if (a.children.size() < b.children.size()) {
		return false;
	}

	for (size_t i = 0; i < b.children.size(); ++i) {
		bool exists = false;
		for (size_t j = 0; j < a.children.size(); ++j) {
			if (!nodeContains(getNode(a.children[j]), getNode(b.children[i]))) {
				exists = true;
				break;
			}
		}
		if (!exists) {
			return false;
		}
	}
	return true;
}