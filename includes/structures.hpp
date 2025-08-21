#pragma once

#include <iostream>

#define BINARY_TREE_TEMPLATE template <typename T>

enum BinaryTreeSide { Right, Left };

BINARY_TREE_TEMPLATE
class BinaryTree
{
	private:
		bool root = true;

		T value;
		BinaryTree* right = nullptr;
		BinaryTree* left = nullptr;

	public:
		BinaryTree(T value);
		~BinaryTree();

		void Destroy();

		const bool& IsRoot() const;
		const T& GetValue() const;

		const bool HasSide(BinaryTreeSide side) const;
		BinaryTree<T>* GetSide(BinaryTreeSide side) const;

		BinaryTree<T>* Expand(T newValue, BinaryTreeSide side);
};

BINARY_TREE_TEMPLATE
std::ostream& operator<<(std::ostream& out, const BinaryTree<T>& tree);

#include "structures.tpp"