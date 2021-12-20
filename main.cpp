/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>
#include <vector>

// Node 只维护指针连接关系
template<typename T>
struct Node {
    // 这两个指针会造成什么问题？请修复
    Node* next;
    Node* prev;
    // 如果能改成 unique_ptr 就更好了!

    T value;

    // 这个构造函数有什么可以改进的？
    Node(T val = 0) 
        : value(val) 
        , next(nullptr)
        , prev(nullptr)
    {
    }

    void insert(Node* node) {
        node->next = next;
        node->prev = prev;
        if (prev)
            prev->next = node;
        if (next)
            next->prev = node;
    }

    void erase() {
        if (prev)
            prev->next = next;
        if (next)
            next->prev = prev;
    }

    ~Node() {
        printf("~Node()\n");   // 应输出多少次？为什么少了？
    }
};

template<typename T>
struct List {
    using Node_t = Node<T>;

    List() = default;

    List(List const &other) {
        printf("List 被拷贝！\n");
        auto head = other.front();  // 这是浅拷贝！
        // 请实现拷贝构造函数为 **深拷贝**
        while (head->next) {
            head = head->next;
        }
        while (head) {
            this->push_front(head->value);
            head = head->prev;
        }
    }

    List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？

    List(List &&) = default;
    List &operator=(List &&) = default;

    Node_t*front() const {
        return m_head;
    }

    void pop_front() {
        if (!m_head) return; // if list is empty, pop_front should do nothing ? no value can be returned ?
        m_head = m_head->next;
    }

    void push_front(int value) {
        auto node = std::make_unique<Node_t>(value);
        node->next = m_head;
        if (m_head)
            m_head->prev = node.get();
        m_head = node.get();
        m_nodeList.emplace_back(std::move(node));
    }

    Node_t*at(size_t index) const {
        auto curr = front();
        for (size_t i = 0; i < index; i++) {
            if (!curr) return nullptr; // index >= list.size();
            curr = curr->next;
        }
        return curr;
    }

private:
    // in heap, will not run out of stack space when a long list is being destructed
    std::vector<std::unique_ptr<Node_t>> m_nodeList;
    Node_t* m_head{}; // nullptr
};

void print(const List<int>& lst) {  // 有什么值得改进的？
    printf("[");
    for (auto curr = lst.front(); curr; curr = curr->next) {
        printf(" %d", curr->value);
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
    // stack overflow? memory leak?
    for (int i = 0; i < 1000000; ++i) {
        a.push_front(i);
    }
    print(a);

    return 0;
}
