/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>

template<typename T>
struct Node {
    // 这两个指针会造成什么问题？请修复
    // 双向链表很容易就产生循环引用了
    std::unique_ptr<Node> next{};
    Node* prev{};
    // 如果能改成 unique_ptr 就更好了!

    T value;

    // 这个构造函数有什么可以改进的？
    // 1, 可以使用初始化列
    // 2, 可以加const 给编译器优化的机会
    // 3,两个指针没有初始化
    Node(T const val):value(val) {
    }
    // 没看懂原函数想干啥(似乎是替换当前节点?) 这里我定义成了插在当前节点之后
    void insert(T val) {
        auto node = std::make_unique<Node>(val);
        node->prev = prev;
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
    // 实际输出了2次 应该输出2+5次(如果拷贝函数是浅拷贝) 或2 + 11次如果拷贝函数是深拷贝且print使用的参数是引用
    // 少了的原因:list的所有node指针都互锁了 ,无法正确释放
    ~Node() {
        printf("~Node()\n");   // 应输出多少次？为什么少了？
    }
};

template<typename T>
struct List {
    std::unique_ptr<Node<T>> head;

    List() = default;

    List(List const &other) {
        printf("List 被拷贝！\n");
        Node<T>* otherPtr = other.front();
        head = nullptr;
        Node<T>* thisPtr = nullptr;
        while (otherPtr != nullptr) {
            if (head == nullptr) {
                head = std::make_unique<Node<T>>(otherPtr->value);
                thisPtr = head.get();
            }
            else {
                thisPtr->insert(otherPtr->value);
                thisPtr = thisPtr->next.get();
            }
            otherPtr = otherPtr->next.get();
        }
        // head = other.head;  // 这是浅拷贝！
        // 请实现拷贝构造函数为 **深拷贝**
    }

    List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？

    List(List &&) = default;
    List &operator=(List &&) = default;

    Node<T> *front() const {
        return head.get();
    }

    T pop_front() {
        T ret = head->value;
        head = std::move(head->next);
        return ret;
    }

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
// 应该传引用
void print(List<T> const &lst) {  // 有什么值得改进的？
    printf("[");
    for (auto curr = lst.front(); curr; curr = curr->next.get()) {
        printf(" %d", curr->value);
    }
    printf(" ]\n");
}

int main() {
    // system("chcp 65001 && cls"); // windows平台问题

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

    List b = a;

    a.at(3)->erase();

    print(a);   // [ 1 4 2 5 7 ]
    print(b);   // [ 1 4 2 8 5 7 ]

    b = {};
    a = {};

    return 0;
}
