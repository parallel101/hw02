/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>
#include <utility>
#include <type_traits>

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
    Node(T val): value(val), prev(nullptr) {}

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
    std::unique_ptr<Node<T>> head;

    List() = default;

    List(List const &other) {
        printf("List 被拷贝！\n");
        head = std::make_unique<Node<T>>(other.head->value);
        auto last = head.get();
        for (auto ptr = other.head->next.get(); ptr; ptr = ptr->next.get()) {
            auto node = std::make_unique<Node<T>>(ptr->value);
            node->prev = last;
            last->next = std::move(node);
            last = last->next.get();
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

    int pop_front() {
        int ret = head->value;
        head = std::move(head->next);
        return ret;
    }

    void push_front(int value) {
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

void print(const List<int> &lst) {  // 有什么值得改进的？: 传入const引用，避免拷贝
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
