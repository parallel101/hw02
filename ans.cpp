#include <cstdio>
#include <memory>

// 节点的实现
template <typename T>
struct Node {
	std::unique_ptr<Node> next;
	Node* prev;
	T m_val;	

	// 构造函数
	explicit Node(T const& val)
		: m_val(val) {}

	void erase() {
		// 这里一定要注意，要先定义 next->prev = prev，否则会直接将 next 给 move 走
	    if(next)
			next->prev = prev;
		if(prev)
			prev->next = std::move(next);
		// if(next)
		//	next->prev = prev;
	}

	~Node() {
		printf("~Node()\n");
	}
};

template <typename T>
struct List {
public:
	using node_type = Node<T>;
	
	// 迭代器本质上还是指针，相当于泛化的指针
	class List_Iterator {
	public:
		using iterator_category = std::bidirectional_iterator_tag;
		using value_type = node_type;
		using difference_type = std::ptrdiff_t;
		using pointer = node_type*;
		using reference = node_type&;

		List_Iterator(pointer ptr)
			: m_ptr(ptr) {}
		// 解引用操作符
		reference operator*() { return *m_ptr; }
		pointer operator->() { return m_ptr; }
		// 前缀自增运算符
		List_Iterator& operator++() {
			m_ptr = m_ptr->next.get();
			return *this;
		}
		// 后缀自增运算符
		List_Iterator operator++(int) {
			List_Iterator tmp = *this;
			++ (*this);
			return tmp;
		}
		// 比较运算符
		bool operator==(List_Iterator const& other) { return m_ptr == other.m_ptr; }
		bool operator!=(List_Iterator const& other) { return m_ptr != other.m_ptr; }
	private:
		pointer m_ptr;
	};

public:
	std::unique_ptr<node_type> head;
	node_type* tail;
	
	List() = default;

	List(List const& other) {
		printf("List 被拷贝了！\n");
		head = std::make_unique<node_type>(other.begin()->m_val);
		tail = head.get();
		for(auto it = ++other.begin(); it != other.end(); ++ it) {
			push_back(it->m_val);
		}
	}

	List& operator=(List const&) = delete;

	List(List &&) = default;
	List& operator=(List &&) = default;
	
	// front
	node_type* front() const {
		return head.get();
	}
	// pop_front
	T pop_front() {
		T res = head->value;
		head = std::move(head->next);
		return res;					
	}
	// push_front
	void push_front(T const& val) {
		auto node = std::make_unique<node_type>(val);
		if(head)
			head->prev = node.get();
		else
			tail = node.get();
		node->next = std::move(head);
		head = std::move(node);
	}
	// push_back
	void push_back(T const& val) {
		auto node = std::make_unique<node_type>(val);
		node->prev = tail;
		tail->next = std::move(node);
		tail = tail->next.get();
	}
	// at
	node_type* at(size_t index) const {
		auto cur = front();
		for(size_t i = 0; i < index; i ++) {
			cur = cur->next.get();
		}
		return cur;
	}	
	List_Iterator begin() const { return List_Iterator(head.get()); }
	List_Iterator end() const { return List_Iterator(nullptr); }
};

template <class T>
void print(List<T> const& list) {
	printf("[");
	for(auto cur = list.front(); cur; cur = cur->next.get())
		printf(" %f", cur->m_val); 
	printf(" ]\n");
}

int main() {
	List<float> a;

	a.push_front(7.2f);
	a.push_front(5.5f);
	a.push_front(8.2f);
	a.push_front(2.1f);
	a.push_front(9.3f);
	a.push_front(4.5f);
	a.push_front(1.9f); 

	print(a);
	a.at(2)->erase();
	print(a);

	List<float> b = a;
	a.at(3)->erase();
	
	b = {};
	a = {};
	return 0;
}
