/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>

struct Node {
    std::unique_ptr<Node> next;
    Node* prev;

    int value;

    // 避免了 value = 0; value = val;
    explicit Node(int val) : next(nullptr), prev(nullptr), value(val) { }

    void insert(int val) {
        auto node = std::make_unique<Node>(val);
        // 改下顺序，unique_ptr被移动之前访问
        if (next) 
            next->prev = node.get();
        node->prev = prev;
        node->next = std::move(next);
        if (prev) 
            prev->next = std::move(node);
    }

    void erase() {
        // 同样在 next 移动之前修改 next->prev;
        if (next)
            next->prev = prev;
        if (prev)
            prev->next = std::move(next);
        
    }

    ~Node() {
        printf("%d ~Node()\n", value);  // 打印value便于调试，应输出 7+6=13 次
    }
};

struct List {
    std::unique_ptr<Node> head;

    List() = default;

    List(List const &other) : head(nullptr) {
        printf("List 被拷贝!\n");
        if(other.head) 
        {
            head = std::make_unique<Node>(other.head->value);
            auto curr = head.get();
            for(auto ptr = other.head->next.get(); ptr; ptr = ptr->next.get())
            {
                curr->next = std::make_unique<Node>(ptr->value);
                curr->next->prev = curr;
                curr = curr->next.get();
            }
        }
    }

    List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？

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

void print(const List& lst) {  // 有什么值得改进的？
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

    print(a);   // [ 1 4 9 2 8 5 7 ]

    a.at(2)->erase();

    print(a);   // [ 1 4 2 8 5 7 ]

    List b = a;

    a.at(3)->erase();

    print(a);   // [ 1 4 2 5 7 ]
    print(b);   // [ 1 4 2 8 5 7 ]

    b = {};
    a = {};

    return 0;
}
