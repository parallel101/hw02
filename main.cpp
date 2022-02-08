/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <exception>
#include <memory>
#include <stdexcept>
#include <utility>
#include <type_traits>
#include <iterator>
#include <cstddef>

template <typename T>
struct Node {
    // 这两个指针会造成什么问题？请修复
    // 对于双向链表，相邻项之间存在循环引用，引用计数无法归零释放空间。
    // 对于双向链表，可以看作前一项“拥有”后一项。
    // 而后一项保留前一项的Node*指针
    std::unique_ptr<Node> next;
    Node* prev;
    // 如果能改成 unique_ptr 就更好了!

    T value;

    // 这个构造函数有什么可以改进的？:value直接根据val构造而不是默认构造后赋值。
    Node(const T& val): value(val), prev(nullptr) {}
    Node(const Node&) = default;
    Node& operator=(Node&&) = default;

    // insert会导致无法使用unique_ptr，因为会破环上面假设的“前一项拥有后一项”的前提
    /*
              +--- O ---+
        O ---x           x--- nextO
              +--- O ---+

        会变成上面这样, 双向链表中用不到该操作所以直接注掉了。
    */
    // void insert(int val) {
    //     auto node = std::make_unique<Node>(val);
    //     node->next = next;
    //     node->prev = prev;
    //     if (prev)
    //         prev->next = node;
    //     if (next)
    //         next->prev = node;
    // }

    void erase() {
        if (next)
            next->prev = prev;
        if (prev)
            prev->next = std::move(next);
    }

    ~Node() {
        printf("~Node()\n");   // 应输出多少次？为什么少了？因为循环引用
    }
};

template<typename T>
struct List {

    class iterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = Node<T>;
        using pointer           = value_type*;  // or also value_type*
        using reference         = value_type&;  // or also value_type&

        iterator(pointer ptr): m_ptr(ptr) {}
        reference operator*() { return *m_ptr; }
        pointer operator->() { return m_ptr; }

        // Prefix increment
        iterator& operator++() 
        { m_ptr = m_ptr->next.get(); return *this; }  

        // Postfix increment
        iterator operator++(int) 
        { iterator tmp = *this; ++(*this); return tmp; }

        friend bool operator== (const iterator& a, const iterator& b) 
        { return a.m_ptr == b.m_ptr; };
        friend bool operator!= (const iterator& a, const iterator& b) 
        { return a.m_ptr != b.m_ptr; };     

    private:
        Node<T>* m_ptr;
    };

    std::unique_ptr<Node<T>> head;
    Node<T>* back = nullptr;

    List() = default;

    List(List &other) {
        printf("List 被拷贝！\n");
        for (auto it = other.begin(); it != other.end(); it++) {
            push_back(it->value);
        }
        // 请实现拷贝构造函数为 **深拷贝**
    }

    List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？
    // 此处拷贝赋值 = 拷贝构造出右值+移动赋值

    List(List &&) = default;
    List &operator=(List &&) = default;

    Node<T> *front() const {
        return head.get();
    }

    T pop_front() {
        if (begin() == end()) {
            throw std::out_of_range("pop_front()");
        }
        T ret = head->value;
        if (head.get() == back)
            back = nullptr;
        head = std::move(head->next);
        return ret;
    }

    void push_front(const T& value) {
        auto node = std::make_unique<Node<T>>(value);
        if (head)
            head->prev = node.get();
        else
            back = node.get();
        node->next = std::move(head);
        head = std::move(node);
    }

    void push_back(const T& value) {
        auto node = std::make_unique<Node<T>>(value);
        if (back) {
            node->prev = back;
            back->next = std::move(node);
            back = back->next.get();
        } else {
            head = std::move(node);
            back = head.get();
        }
    }

    Node<T> *at(size_t index) const {
        auto curr = front();
        for (size_t i = 0; i < index; i++) {
            curr = curr->next.get();
        }
        return curr;
    }

    iterator begin() { return iterator(head.get()); }
    iterator end() { return iterator(nullptr); }
};

void print(List<int> &lst) {  // 有什么值得改进的？: 传入const引用，避免拷贝
    printf("[");
    for (auto &v: lst) {
        printf(" %d", v.value);
    }
    printf(" ]\n");
}

int main() {
    List<int> a;

    a.push_front(7);
    a.push_front(5);
    a.push_front(8);
    a.push_front(2);
    a.push_front(9);
    a.push_front(4);
    a.push_front(1);

    print(a);   // [ 1 4 9 2 8 5 7 ]

    a.at(2)->erase();

    print(a);   // [ 1 4 2 8 5 7 ]

    List<int> b = a;

    a.at(3)->erase();

    print(a);   // [ 1 4 2 5 7 ]
    print(b);   // [ 1 4 2 8 5 7 ]

    b = {};
    a = {};

    b.push_back(1);
    b.push_back(2);
    b.push_back(3);
    print(b);   // [ 1 2 3 ]
    return 0;
}