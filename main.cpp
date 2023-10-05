/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>

template <class value_type>
struct Node
{
	std::unique_ptr<Node> next;
	Node* prev;
	// 如果能改成 unique_ptr 就更好了!

	value_type value;

	// 这个构造函数有什么可以改进的？：防止单参数隐式构造，初始列构造减少一次赋值
	explicit Node(value_type val) : value(val)
	{}

	void erase() {
		if (next)
			next->prev = prev;
		if (prev)
			prev->next = std::move(next);
	}

	~Node() {
		printf("~Node() : %f\n", value); // 应输出多少次？为什么少了？：14次，share_ptr循环引用导致内存无法释放
	}
};


template <class T>
struct List
{
	using node_type = Node<T>;

	class list_iterator
	{
	public:
		using iterator_catagory = std::bidirectional_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using value_type = node_type;
		using pointer = node_type*;
		using reference = node_type&;

		list_iterator(pointer ptr) : m_ptr(ptr)
		{}

		reference operator*() { return *m_ptr; }
		pointer operator->() { return m_ptr; }

		list_iterator operator++() {
			m_ptr = m_ptr->next.get();
			return *this;
		}

		list_iterator operator++(int) {
			return ++(*this);
		}

		friend bool operator==(const list_iterator a, const list_iterator b) {
			return a.m_ptr == b.m_ptr;
		}

		friend bool operator!=(const list_iterator a, const list_iterator b) {
			return a.m_ptr != b.m_ptr;
		}

	private:
		node_type* m_ptr;
	};

public:
	std::unique_ptr<node_type> head;
	node_type* back;
	List() = default;

	List(List const& other) {
		printf("List 被拷贝！\n");

		// 请实现拷贝构造函数为 **深拷贝**
		head = std::make_unique<node_type>(other.cbegin()->value);
		back = head.get();
		for (auto it = ++other.cbegin(); it != other.cend(); ++it) {
			push_back(it->value);
		}
	}

	List& operator=(List const&) = delete; // 为什么删除拷贝赋值函数也不出错？：编译器自动创建一个纯右值，再用移动构造将所有权转给等号左边的变量

	List(List&&) = default;
	List& operator=(List&&) = default;

	node_type* front() const {
		return head.get();
	}

	T pop_front() {
		T ret = head->value;
		head = std::move(head->next);
		return ret;
	}

	void push_front(const T& value) {
		auto node = std::make_unique<node_type>(value);
		if (head)
			head->prev = node.get();
		else
			back = node.get();
		node->next = std::move(head);
		head = std::move(node);
	}

	void push_back(const T& value) {
		auto node = std::make_unique<node_type>(value);
		node->prev = back;
		back->next = std::move(node);
		back = back->next.get();
	}

	node_type* at(size_t index) const {
		auto curr = front();
		for (size_t i = 0; i < index; i++) {
			curr = curr->next.get();
		}
		return curr;
	}

	list_iterator cbegin() const { return list_iterator(head.get()); }

	list_iterator cend() const { return list_iterator(nullptr); }

	//TODO:非const的begin(),end()
	//TODO:shared_ptr改unique_ptr
	//TODO:end(),back实现修改，返回值依然是nullptr，但可以--
};

template <class T>
void print(List<T> const& lst) {
	// 有什么值得改进的？：常量引用传递避免拷贝
	printf("[");
	for (auto curr = lst.front(); curr; curr = curr->next.get()) {
		printf(" %f", curr->value);
	}
	printf(" ]\n");
}

int main() {
	List<double> a;

	a.push_front(7.2f);
	a.push_front(5.5f);
	a.push_front(8.2f);
	a.push_front(2.1f);
	a.push_front(9.3f);
	a.push_front(4.5f);
	a.push_front(1.9f);

	print(a); // [ 1 4 9 2 8 5 7 ]

	a.at(2)->erase();

	print(a); // [ 1 4 2 8 5 7 ]

	List<double> b = a;

	a.at(3)->erase();

	print(a); // [ 1 4 2 5 7 ]
	print(b); // [ 1 4 2 8 5 7 ]

	b = {};
	a = {};

	return 0;
}
