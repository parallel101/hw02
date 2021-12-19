#pragma once
#include <memory>
#include <utility>

template <typename T>
struct Node {
	// 这两个指针会造成什么问题？请修复
	std::unique_ptr<Node> next = nullptr;
	Node* prev = nullptr;
	// 如果能改成 unique_ptr 就更好了!

	T value;

	template<typename TArg>
	Node(TArg&& arg) : value(T(std::forward<TArg>(arg))) {}    // 有什么可以改进的？

	template<typename TArg>
	void insertAsSucc(TArg&& firstArg) {
		auto node = std::make_unique<Node<T>>(T(std::forward<TArg>(firstArg)));
		if (next) {
			next->prev = node.get();
			node->next = std::move(next);
		}
		node->prev = this;
		this->next = std::move(node);
	}

	template<typename FirstTArg, typename ...RestTArgs>
	void insertAsSucc(FirstTArg&& firstArg, RestTArgs&& ...arg) {
		insertAsSucc(std::forward<FirstTArg>(firstArg));
		insertAsSucc(std::forward<RestTArgs>(arg)...);
	}

	void erase() {
		if (next)
			next->prev = prev;
		if (prev)
			prev->next = std::move(next);
	}

	~Node() {
		printf("~Node()\n");   // 应输出多少次？为什么少了？
	}
};


template <typename T>
struct List {
	using NodePtr = std::unique_ptr<Node<T>>;
	NodePtr head = nullptr;

	List() = default;

	List(List const& other) {
		printf("List 被拷贝！\n");
		// 这是浅拷贝！
		// 请实现拷贝构造函数为 **深拷贝**

		head = std::make_unique<Node<T>>(other.head->value);
		for (auto curr = front(), currOther = other.head->next.get(); currOther; curr = curr->next.get(), currOther = currOther->next.get()) {
			curr->insertAsSucc(currOther->value);
		}
	}

	List& operator=(List const&) = delete;  // 为什么删除拷贝赋值函数也不出错？

	List(List&&) = default;
	List& operator=(List&&) = default;

	Node<T>* front() const {
		return head.get();
	}

	int pop_front() {
		int ret = head->value;
		head = std::move(head->next);
		return ret;
	}

	void push_front(int value) {
		auto node = std::make_unique<Node<T>>(value);
		if (head) {
			head->prev = node.get();
			node->next = std::move(head);
		}
		head = std::move(node);
	}

	Node<T>* at(size_t index) const {
		auto curr = front();
		for (size_t i = 0; i < index; i++) {
			curr = curr->next.get();
		}
		return curr;
	}
};