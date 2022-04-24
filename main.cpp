/* 基于智能指针实现双向链表 */
#include <iostream>
#include <memory>

template<typename T>
struct Node {
    using value_type = T;

    // 这两个指针会造成什么问题？请修复
    std::unique_ptr<Node> next{ nullptr };
    Node *prev{ nullptr };
    // 如果能改成 unique_ptr 就更好了!

    value_type value;

    // 这个构造函数有什么可以改进的？
    explicit Node(value_type val)
        : value{ val } {
    }

    void insert(value_type val) {
        auto node = std::make_unique<Node>(val);
        node->prev = this;
        if (next)
            next->prev = node.get();
        node->next = std::move(next);
        next = std::move(node);
    }

    void erase() {
        if (next)
            next->prev = prev;
        if (prev)
            prev->next = std::move(next);
    }

    ~Node() {
        std::cout << "~Node() this: " << this << ", value: " << value << '\n'; // 应输出多少次？为什么少了？
    }
};

template<typename T>
struct List {
    using value_type = T;
    using node_type = Node<T>;

    std::unique_ptr<node_type> head{ nullptr };

    List() = default;

    List(List const &other) {
        printf("List 被拷贝！\n");
        // head = other.head;  // 这是浅拷贝！
        // 请实现拷贝构造函数为 **深拷贝**
        if (!other.head)
            return;
        auto other_curr = other.head.get();
        head = std::make_unique<node_type>(other_curr->value);
        auto prev = head.get();
        other_curr = other_curr->next.get();
        while (other_curr)
        {
            auto node = std::make_unique<node_type>(other_curr->value);
            node->prev = prev;
            prev->next = std::move(node);
            prev = prev->next.get();
            other_curr = other_curr->next.get();
        }
    }

    List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？

    List(List &&) = default;
    List &operator=(List &&) = default;

    node_type *front() const {
        return head.get();
    }

    value_type pop_front() {
        auto ret = head->value;
        head = std::move(head->next);
        return ret;
    }

    void push_front(value_type value) {
        auto node = std::make_unique<node_type>(value);
        if (head)
            head->prev = node.get();
        node->next = std::move(head);
        head = std::move(node);
    }

    node_type *at(size_t index) const {
        auto curr = front();
        for (size_t i = 0; i < index; i++) {
            curr = curr->next.get();
        }
        return curr;
    }
};

template<typename T>
void print(const List<T> &lst) {  // 有什么值得改进的？
    std::cout << '[';
    for (auto curr = lst.front(); curr; curr = curr->next.get()) {
        std::cout << ' ' << curr->value;
    }
    std::cout << " ]\n";
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

    auto b = a;

    a.at(3)->erase();

    print(a);   // [ 1 4 2 5 7 ]
    print(b);   // [ 1 4 2 8 5 7 ]

    b = {};
    a = {};

    return 0;
}
