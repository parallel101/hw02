/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>
#include <iostream>

template <typename T>
struct Node {
    // 这两个指针会造成什么问题？请修复
    // 如果能改成 unique_ptr 就更好了!
    std::unique_ptr<Node> next;
    Node* prev;

    T value;

    explicit Node(T value) : value(value), prev(nullptr), next(nullptr) {
        std::cout << "constrcuter "<< value <<'\n';
    }  // 有什么可以改进的？

    void insert(T value) {
        this->next = std::make_unique<Node>(value);
        this->next->prev = this;
    }

    void erase() {
        if (this->next)
            next->prev = this->prev;
        if (this->prev)
            prev->next = std::move(this->next);
    }

    ~Node() {
        printf("~Node() + %d \n", value);   // 应输出多少次？为什么少了？
    }
};

template <typename T> struct List;



template <typename T>
struct ListIterator {
    Node<T>* node;

    ListIterator(Node<T>* node): node(node) {}

    ListIterator& operator+=(int addon) {
        for (int i=0;i<addon&&node!=nullptr;i++) {
            node = node->next.get();
        }
        return *this;
    }

    Node<T> *operator->() { return node; }

    bool operator!=(const ListIterator<T>& rhs) { return node != rhs.node; }
};

template <typename T>
struct List {
    typedef Node<T> NodeType;
    typedef ListIterator<T> iterator;

    std::unique_ptr<NodeType> head;

    List() = default;

    List(List const &other) {
        printf("List being copied!\n");
        // head = other.head; // 这是浅拷贝！
        // 请实现拷贝构造函数为 **深拷贝**

        auto curr=other.front();
        // tail insertion step 1. go to the tail
        while (curr->next) {
            curr = curr->next.get();
        }
        // tail insertion step 2. traverse from tail to head
        while (curr) {
            push_front(curr->value);
            curr = curr->prev;
        }
    }

    List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？

    List(List &&) = default;
    List &operator=(List &&) = default;

    NodeType *front() const {
        return head.get();
    }

    T pop_front() {
        T ret = head->value;
        head = std::move(head->next);
        return ret;
    }

    void push_front(T value) {
        auto node = std::make_unique<NodeType>(value);
        if (head)
            head->prev = node.get();
        node->next = std::move(head);
        head = std::move(node);
    }

    NodeType *at(size_t index) const {
        auto curr = front();
        for (size_t i = 0; i < index; i++) {
            curr = curr->next.get();
        }
        return curr;
    }

    iterator begin() const {
        return iterator(head.get());
    }

    iterator end() const {
        return iterator(nullptr);
    }
};

template <typename T>
void print(const List<T>& lst) {  // 有什么值得改进的？
    printf("[");
    for (auto curr = lst.front(); curr; curr = curr->next.get()) {
        printf(" %d", curr->value);
    }
    printf(" ]\n");
}

template <typename T>
void printplus(const List<T>& lst) {
    printf("[");
    for (auto it = lst.begin(); it!=lst.end(); it+=1) {
        printf(" %d", it->value);
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

    printplus(a);   // [ 1 4 9 2 8 5 7 ]

    a.at(2)->erase();

    printplus(a);   // [ 1 4 2 8 5 7 ]

    List b = a;

    a.at(3)->erase();

    printplus(a);   // [ 1 4 2 5 7 ]
    printplus(b);   // [ 1 4 2 8 5 7 ]
    b = {};
    a = {};

    return 0;
}
