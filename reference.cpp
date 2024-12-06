#include <iostream>
#include <vector>

template <typename T>
class MyRandomAccessIterator {
public:
	using iterator_category = std::bidirectional_iterator_tag;
	using value_type = T;
	using difference_type = std::ptrdiff_t;
	using pointer = T*;
	using reference = T&;

	MyRandomAccessIterator(pointer ptr) 
		: m_ptr(ptr) {}

	// 解引用运算符
	reference operator*() const { return *m_ptr; }
	pointer operator->() const { return m_ptr; }

	// 前缀自增运算符
	MyRandomAccessIterator& operator++() {
		++ m_ptr;
		return *this;
	}
	
	// 后缀自增运算符
	MyRandomAccessIterator operator++(int) {
		MyRandomAccessIterator tmp = *this;
		++(*this);
		return tmp;
	}	

	// 随机访问运算符
	reference operator[](size_t index) const { return *(m_ptr + index); }
	
	// 赋值运算符
	bool operator==(MyRandomAccessIterator const& other) const { return m_ptr == other.m_ptr; }
	bool operator!=(MyRandomAccessIterator const& other) const { return m_ptr != other.m_ptr; }	
private:
	pointer m_ptr;
};

template <typename T>
class MyContainer {
public:
	using iterator = MyRandomAccessIterator<T>;
	
	void add(T const& val) {
		m_data.push_back(val);
	}

	iterator begin() { return iterator(m_data.data());}
	iterator end() { return iterator(m_data.data() + m_data.size());}
private:
	std::vector<T> m_data;
}; 

int main() {
	MyContainer<float> container;

	container.add(1);
	container.add(2);
	container.add(3);

	for(auto it = container.begin(); it != container.end(); it ++) 
		std::cout << *it << ' ';
	std::cout << '\n';

	// 这里在迭代器中定义的 [] 索引符号
	auto it = container.begin();
	std::cout << it[2] << '\n';
	// std::cout << container[2] << '\n';  非法
	return 0;
}
