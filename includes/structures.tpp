#include "structures.hpp"

BINARY_TREE_TEMPLATE
BinaryTree<T>::BinaryTree(T value) : value(value)
{

}

BINARY_TREE_TEMPLATE
BinaryTree<T>::~BinaryTree()
{
	if (root) Destroy();
}

BINARY_TREE_TEMPLATE
void BinaryTree<T>::Destroy()
{
	if (right)
	{
		right->Destroy();
		delete(right);
		right = nullptr;
	}
	if (left)
	{
		left->Destroy();
		delete(left);
		left = nullptr;
	}
}

BINARY_TREE_TEMPLATE
const bool& BinaryTree<T>::IsRoot() const
{
	return (root);
}

BINARY_TREE_TEMPLATE
const T& BinaryTree<T>::GetValue() const
{
	return (value);
}

BINARY_TREE_TEMPLATE
const bool BinaryTree<T>::HasSide(BinaryTreeSide side) const
{
	return (GetSide(side) != nullptr);
}

BINARY_TREE_TEMPLATE
BinaryTree<T>* BinaryTree<T>::GetSide(BinaryTreeSide side) const
{
	if (side == Right) return (right);
	else if (side == Left) return (left);
	else return (nullptr);
}

BINARY_TREE_TEMPLATE
BinaryTree<T>* BinaryTree<T>::Expand(T newValue, BinaryTreeSide side)
{
	if (GetSide(side) != nullptr) return (nullptr);

	BinaryTree<T>* newTree = new BinaryTree<T>(newValue);
	newTree->root = false;

	if (side == Right) right = newTree;
	else if (side == Left) left = newTree;

	return (newTree);
}

BINARY_TREE_TEMPLATE
std::ostream& operator<<(std::ostream& out, const BinaryTree<T>& tree)
{
	out << tree.GetValue() << std::endl;

	if (tree.HasSide(Right)) out << *tree.GetSide(Right);
	if (tree.HasSide(Left)) out << *tree.GetSide(Left);

	return (out);
}