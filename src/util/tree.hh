#pragma once

#include "assert.hh"

// todo: try a different container for storing child nodes
#include <deque>
//#include <list>
#include <memory>

namespace util
{
	/**
	 * @brief Fundamental unit of a which trees are formed.
	 *  Each non-root node has a parent and owns zero or more child nodes.
	 */
	template <class T>
	class TreeNode
	{
	private:
		T *_parent;
		std::deque<std::unique_ptr<T>> _children;

	public:
		TreeNode(T *parent)
			: _parent(parent), _children()
		{
		}

		TreeNode(const TreeNode<T> &) = delete;
		TreeNode<T> &operator=(const TreeNode<T> &) = delete;

		TreeNode(TreeNode<T> &&) = delete;
		TreeNode<T> &operator=(TreeNode<T> &&) = delete;

		void clear()
		{
			_children.clear();
		}

		bool empty() const
		{
			return _children.empty();
		}

		std::size_t size() const
		{
			return _children.size();
		}

		T *front()
		{
			return _children.front().get();
		}

		const T *front() const
		{
			return _children.front().get();
		}

		T *back()
		{
			return _children.back().get();
		}

		const T *back() const
		{
			return _children.back().get();
		}

		void pop_front()
		{
			ASSERT(!empty());
			_children.pop_front();
		}

		void pop_back()
		{
			ASSERT(!empty());
			_children.pop_back();
		}

		T *child(std::size_t i)
		{
			ASSERT(i < size());
			return _children[i].get();
		}

		const T *child(std::size_t i) const
		{
			ASSERT(i < size());
			return _children[i].get();
		}

		template <typename ...Args>
		void emplace(std::size_t i, Args &&...args)
		{
			insert(i, std::make_unique<T>(nullptr, std::forward<Args>(args)...));
		}

		template <typename ...Args>
		void emplace_back(Args &&...args)
		{
			push_back(std::make_unique<T>(nullptr, std::forward<Args>(args)...));
		}

		template <typename ...Args>
		void emplace_front(Args &&...args)
		{
			push_front(std::make_unique<T>(nullptr, std::forward<Args>(args)...));
		}

		void insert(std::size_t i, std::unique_ptr<T> node)
		{
			ASSERT(node->is_dangling());

			node->_parent = static_cast<T *>(this);
			_children.insert(_children.begin() + i, std::move(node));
		}

		void push_back(std::unique_ptr<T> node)
		{
			ASSERT(node->is_dangling());

			node->_parent = static_cast<T *>(this);
			_children.push_back(std::move(node));
		}

		void push_front(std::unique_ptr<T> node)
		{
			ASSERT(node->is_dangling());

			node->_parent = static_cast<T *>(this);
			_children.push_front(std::move(node));
		}

		void remove(std::size_t i)
		{
			ASSERT(i < size());
			erase(_children.begin() + i);
		}

		void swap(std::size_t i, std::size_t j)
		{
			ASSERT(i < size());
			ASSERT(j < size());

			_children[i].swap(_children[j]);
		}

		T *parent()
		{
			return _parent;
		}

		const T *parent() const
		{
			return _parent;
		}

		bool is_root() const
		{
			return parent() == nullptr;
		}

		bool is_dangling() const
		{
			return parent() == nullptr;
		}

		bool is_leaf() const
		{
			return empty();
		}

		T *root()
		{
			T *node = static_cast<T *>(this);

			while (!node->is_root())
				node = node->parent();

			return node;
		}

		const T *root() const
		{
			const T *node = static_cast<const T *>(this);

			while (!node->is_root())
				node = node->parent();

			return node;
		}
	};
}
