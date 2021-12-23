#pragma once
#include <memory>
#include <utility>

template <typename T>
struct Node {
	using NodeUPtr = std::unique_ptr<Node<T>>;
	using NodePtr = Node<T>*;

	// 这两个指针会造成什么问题？请修复
	NodeUPtr next = nullptr;
	NodePtr prev = nullptr;
	// 如果能改成 unique_ptr 就更好了!

	T value;

	Node() {}

	template<typename TArg>
	Node(TArg&& arg, NodePtr p = nullptr, NodeUPtr n = nullptr) : value(T(std::forward<TArg>(arg))), prev(p), next(std::move(n)) {}    // 有什么可以改进的？

	template<typename TArg>
	void insertAsPrev(TArg&& arg) {
		auto node = std::make_unique<Node<T>>(std::forward<TArg>(arg), prev, std::move(prev->next));
		prev->next = std::move(node);  //设置正向链接
		prev = (prev->next).get();    //设置逆向链接
	}

	template<typename ...TArgs>
	void insertAsPrev(TArgs&& ...args) {  //variadic template，多个值在前面插入
		struct Dummy {} dummy;
		((insertAsPrev(std::forward<TArgs>(args)), dummy) = ...);
	}

	template<typename ...TArgs>
	void insertAsPrevInverted(TArgs&& ...args) {  //多个值逆序在前面插入
		static_assert(sizeof...(args) > 0);
		(insertAsPrev(std::forward<TArgs>(args)), ...);
	}

	template<typename TArg>
	void insertAsNext(TArg&& arg) {
		auto node = std::make_unique<Node<T>>(std::forward<TArg>(arg), this, std::move(next));
		node->next->prev = node.get();  //设置逆向链接
		next = std::move(node);  //设置正向链接
	}

	template<typename ...TArgs>
	void insertAsNext(TArgs&& ...args) { //多个值顺序在后面插入
		(insertAsNext(std::forward<TArgs>(args)), ...);
	}

	template<typename ...TArgs>
	void insertAsNextInverted(TArgs&& ...args) { //多个值顺序在后面插入
		static_assert(sizeof...(args) > 0);
		struct Dummy {} dummy;
		((insertAsNext(std::forward<TArgs>(args)), dummy) = ...);
	}

	//c++的函数模板不支持偏特化，只支持重载，这些相似的代码有没有简化的写法？workaround: 偏特化functor类模板? tag dispatch?

	void erase() {
		next->prev = prev;
		prev->next = std::move(next);
	}

	~Node() {
		printf("~Node()\n");   // 应输出多少次？为什么少了？
	}
};


template <typename T>
struct List {
	using NodeUPtr = std::unique_ptr<Node<T>>;
	using NodePtr = Node<T>*;

	//设置头尾哨兵，保证前向后向指针始终有效
	NodeUPtr head = nullptr; //头哨兵
	NodePtr tail = nullptr;  //尾哨兵

	class ListIterator  //a minimal iterator 
	{
	public:
		ListIterator(NodePtr curr) : current(curr) { }

		bool operator!= (const ListIterator& other) const {
			return current != other.current;
		}

		const T& operator* () const {
			return current->value;
		}

		const ListIterator& operator++ () {
			current = (current->next).get();
			return *this;
		}

	private:
		NodePtr current;
	};

	constexpr NodePtr first() const {//首节点
		return head->next.get();
	}

	constexpr NodePtr last() const { //末节点
		return tail->prev;
	}

	auto begin() const {  //支持 Range-based for循环
		return ListIterator{ first() };
	}

	auto end() const { //支持 Range-based for循环
		return ListIterator{ tail };
	}

	void init() {
		head = std::make_unique<Node<T>>();  //创建头哨兵节点
		tail = (head->next = std::make_unique<Node<T>>()).get();  //创建尾哨兵节点，设置正向链接
		head->next->prev = head.get();  //设置逆向链接
	}

	List() {
		init();
	}

	List(List const& other) {
		printf("List 被拷贝！\n");
		// 这是浅拷贝！
		// 请实现拷贝构造函数为 **深拷贝**
		init();
		for (auto&& value : other) {
			push_end(value);
		}
	}

	List& operator=(List const&) = delete;  // 为什么删除拷贝赋值函数也不出错？

	List(List&&) = default;
	List& operator=(List&&) = default;

	T& pop_front() {
		auto ret = first()->value;
		first()->erase();
		return ret;
	}

	template<typename FirstTArg, typename ...RestTArgs>
	void push_end(FirstTArg arg, RestTArgs&& ...args) { //末节点插入
		tail->insertAsPrevInverted(std::forward<FirstTArg>(arg), std::forward<RestTArgs>(args)...);
	}

	template<typename FirstTArg, typename ...RestTArgs>
	void push_front(FirstTArg arg, RestTArgs&& ...args) {
		head->insertAsNextInverted(std::forward<FirstTArg>(arg), std::forward<RestTArgs>(args)...);
	}

	NodePtr at(size_t index) const {
		auto curr = first();
		for (size_t i = 0; i < index; i++) {
			curr = curr->next.get();
		}
		return curr;
	}
};