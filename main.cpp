/* 基于智能指针实现双向链表，总计 60 分 */
#include <cstdio>
#include <memory>

struct Node {
    std::shared_ptr<Node> next;  // 能否改进为 unique_ptr？   20 分
    std::shared_ptr<Node> prev;  // 这会造成什么问题？请修复  15 分
    int value;

    Node(int value) : value(value) {}  // 有什么可以改进的？   5 分

    void insert(int value) {
        auto node = std::make_shared<Node>(value);
        node->value = value;
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
};

struct List {
    std::shared_ptr<Node> head;

    List() = default;

    List(List const &) = default;             // 请实现拷贝构造函数 15 分
    List &operator=(List const &) = default;  // 请实现拷贝赋值函数  5 分

    List(List &&) = default;
    List &operator=(List &&) = default;

    Node *front() const {
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

    Node *at(size_t index) const {
        auto curr = front();
        for (size_t i = 0; i < index; i++) {
            curr = curr->next.get();
        }
        return curr;
    }

    size_t size() const {
        size_t count = 0;
        for (auto curr = front(); curr; curr = curr->next.get()) {
            count++;
        }
        return count;
    }

    void print() const {
        printf("[");
        for (auto curr = front(); curr; curr = curr->next.get()) {
            printf(" %d", curr->value);
        }
        printf(" ]\n");
    }
};

int main() {
    List a;

    a.push_front(7);
    a.push_front(5);
    a.push_front(8);
    a.push_front(2);
    a.push_front(9);
    a.push_front(4);
    a.push_front(1);

    a.print();   // [ 1 4 9 2 8 5 7 ]

    a.at(2)->erase();

    a.print();   // [ 1 4 2 8 5 7 ]

    List b = a;

    printf("%d\n", a.pop_front());  // 1
    printf("%d\n", a.pop_front());  // 4

    a.print();   // [ 2 8 5 7 ]
    b.print();   // [ 1 4 2 8 5 7 ]

    return 0;
}
