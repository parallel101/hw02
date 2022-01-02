/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>

struct Node {
    // 这两个指针会造成什么问题？请修复 -> 循环引用
    std::unique_ptr<Node> next;  // 安全，智能指针
    Node *prev;                  // 安全，智能指针 get 出的原始指针
    // 如果能改成 unique_ptr 就更好了!

    int value;

    // 这个构造函数有什么可以改进的？ -> 使用初始化列表；标记为 explicit 避免陷阱如 Node n = 2;
    explicit Node(int val) : value(val) {}

    void insert(int val) {
        auto node = std::make_unique<Node>(val);
        if (next)
            next->prev = node.get();
        node->next = std::move(next);  // 这里 move 了 next，在这之前完成对 next 的所有操作
        node->prev = prev;
        if (prev)
            prev->next = std::move(node);  // 这里 move 了 node，在这之前完成对 node 的所有操作
    }

    void erase() {
        if (next)
            next->prev = prev;
        if (prev)
            prev->next = std::move(next);  // 这里 move 了 next，在这之前完成对 next 的所有操作
    }

    ~Node() {
        printf("~Node()\n");  // 应输出多少次？为什么少了？-> 应输出 x 次，x 为 Node 总数；循环引用导致 Node 无法正常解构
    }
};

struct List {
    std::unique_ptr<Node> head;

    List() = default;

    List(List const &other) {
        printf("List 被拷贝！\n");
        // head = other.head;  // 这是浅拷贝！
        // 请实现拷贝构造函数为 **深拷贝**
        if (other.head == nullptr) {
            head = nullptr;
        } else {
            head = std::make_unique<Node>(other.head->value);
            auto curr = head.get();
            auto other_next = other.head->next.get();
            while (other_next != nullptr) {
                auto new_node = std::make_unique<Node>(other_next->value);
                new_node->prev = curr;
                curr->next = std::move(new_node);
                curr = curr->next.get();
                other_next = other_next->next.get();
            }
        }
    }

    List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？ -> 因为删除了拷贝复制函数后，编译器会做 b = List(a)，就地构造，变成了移动语义（而我们有移动赋值函数）

    List(List &&) = default;
    List &operator=(List &&) = default;

    Node *front() const {
        return head.get();
    }

    int pop_front() {
        int ret = head->value;
        head = std::move(head->next);
        return ret;
    }

    void push_front(int value) {
        auto node = std::make_unique<Node>(value);
        if (head)
            head->prev = node.get();
        node->next = std::move(head);
        head = std::move(node);
    }

    Node *at(size_t index) const {
        auto curr = front();
        for (size_t i = 0; i < index; i++) {
            curr = curr->next.get();
        }
        return curr;
    }
};

void print(List const &lst) {  // 有什么值得改进的？ -> 改为常引用，避免不必要的拷贝
    printf("[");
    for (auto curr = lst.front(); curr; curr = curr->next.get()) {
        printf(" %d", curr->value);
    }
    printf(" ]\n");
}

int main() {
    List a;

    a.push_front(7);
    a.push_front(5);
    a.push_front(8);
    a.push_front(2);
    a.push_front(9);
    a.push_front(4);
    a.push_front(1);

    print(a);  // [ 1 4 9 2 8 5 7 ]

    a.at(2)->erase();  // 1 "~Node()" printed

    print(a);  // [ 1 4 2 8 5 7 ]

    List b = a;

    a.at(3)->erase();  // 1 "~Node()" printed

    print(a);  // [ 1 4 2 5 7 ] // 5 "~Node()" printed
    print(b);  // [ 1 4 2 8 5 7 ] // 6 "~Node()" printed

    b = {};
    a = {};

    return 0;
}
