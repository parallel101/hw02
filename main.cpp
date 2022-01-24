/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>

struct Node {
    // 这两个指针会造成什么问题？请修复
    std::shared_ptr<Node> next;
    std::shared_ptr<Node> prev;
    // 如果能改成 unique_ptr 就更好了!
    // std::unique_ptr<Node> next;
    // std::unique_ptr<Node> prev;

    int value;

    // 这个构造函数有什么可以改进的？
    // Node(int val) {
    //     value = val;
    // }
    Node(int val)
        : value(val), next(nullptr), prev(nullptr) {}

    void insert(int val) {
        auto node = std::make_shared<Node>(val);
        node->next = next;
        node->prev = prev;
        if (prev)
            prev->next = node;
        if (next)
            next->prev = node;
    }

    void erase() {
        if (prev)
            prev->next = std::move(next);
        if (next)
            next->prev = std::move(prev);
    }

    ~Node() {
        printf("~Node()\n");  // 应输出多少次？为什么少了？
    }
};

struct List {
    std::shared_ptr<Node> head;

    List() = default;

    List(List const& other) {
        printf("List 被拷贝！\n");

        // 这是浅拷贝！
        // 浅拷贝只是增加了other.head的引用计数
        // head = other.head;

        // 请实现拷贝构造函数为 **深拷贝**
        std::shared_ptr<Node> rear;
        for (auto p = other.head; p != nullptr; p = p->next) {
            // printf("create node: %d\n", p->value);
            std::shared_ptr<Node> new_node(new Node(p->value));
            if (p == other.head) {
                head = new_node;
                rear = head;
            } else {
                rear->next = new_node;
                new_node->prev = rear;
                rear = new_node;
            }
        }
    }

    List& operator=(List const&) = delete;  // 为什么删除拷贝赋值函数也不出错？

    List(List&&) = default;
    List& operator=(List&&) = default;

    Node* front() const {
        return head.get();
    }

    int pop_front() {
        int ret = head->value;
        head = head->next;
        return ret;
    }

    void push_front(int value) {
        auto node = std::make_shared<Node>(value);
        node->next = head;
        if (head)
            head->prev = node;
        head = node;
    }

    Node* at(size_t index) const {
        auto curr = front();
        for (size_t i = 0; i < index; i++) {
            curr = curr->next.get();
        }
        return curr;
    }
};

// 改进传入参数形式 ，避免隐式拷贝
void print(List const& lst) {  // 有什么值得改进的？
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

    a.at(2)->erase();

    print(a);  // [ 1 4 2 8 5 7 ]

    List b = a;

    a.at(3)->erase();

    print(a);  // [ 1 4 2 5 7 ]
    print(b);  // [ 1 4 2 8 5 7 ]

    b = {};
    a = {};

    return 0;
}
