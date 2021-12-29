/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>

template<typename T>
struct Node {
    // 这两个指针会造成什么问题？请修复
//    std::shared_ptr<Node> next;
//    std::shared_ptr<Node> prev;
    // 如果能改成 unique_ptr 就更好了!
    std::unique_ptr<Node> next;
    Node* prev;

    T value;

    // 这个构造函数有什么可以改进的？
//    Node(T val) {
//        value = val;
//    }
    //使用初始化列表
    explicit Node(T val):value(val),prev(nullptr)
    {}

//    void insert(int val) {
//        auto node = std::make_shared<Node>(val);
//        node->next = next;
//        node->prev = prev;
//        if (prev)
//            prev->next = node;
//        if (next)
//            next->prev = node;
//    }
    void insert(T val) {
        auto node = std::make_unique<Node>(val);
        node->prev = this;
        if(next) {
            next->prev = node.get();
            node->next = std::move(next);
        }
        next = std::move(node);
    }

//    void erase() {
//        if (prev)
//            prev->next = next;
//        if (next)
//            next->prev = prev;
//    }

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

template<typename T>
struct List {
//    std::shared_ptr<Node> head;
    std::unique_ptr<Node<T>> head;
    List() = default;

    List(List const &other) {
        printf("List 被拷贝！\n");
//        head = other.head;  // 这是浅拷贝！
        // 请实现拷贝构造函数为 **深拷贝**
        auto last = other.head.get();
        while (last->next != nullptr){
            last = last->next.get();
        }
        head = std::make_unique<Node<T>>(last->value);
        while(last != other.head.get()) {
            last = last->prev;
            push_front(last->value);
        }
    }

    List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？

    List(List &&) = default;
    List &operator=(List &&) = default;

    Node<T> *front() const {
        return head.get();
    }

    int pop_front() {
        int ret = head->value;
//        head = head->next;
        head = std::move(head->next);
        return ret;
    }

//    void push_front(int value) {
//        auto node = std::make_shared<Node>(value);
//        node->next = head;
//        if (head)
//            head->prev = node;
//        head = node;
//    }
    void push_front(T value) {
        auto node = std::make_unique<Node<T>>(value);

        if (head)
            head->prev = node.get();

        node->next = std::move(head);
        head = std::move(node);
    }

    Node<T> *at(size_t index) const {
        auto curr = front();
        for (size_t i = 0; i < index; i++) {
            curr = curr->next.get();
        }
        return curr;
    }
};

template<typename T>
void print(List<T> const & lst) {  // 有什么值得改进的？
    printf("[");
    for (auto curr = lst.front(); curr; curr = curr->next.get()) {
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

    return 0;
}
