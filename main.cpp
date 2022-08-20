/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>

template<typename T>
struct Node {
    // 这两个指针会造成什么问题？请修复
    std::unique_ptr<Node> next;
    Node* prev{nullptr};
    // 如果能改成 unique_ptr 就更好了!

    T value;

    // 这个构造函数有什么可以改进的？
    Node(T val) : value(val)
    {}

    void insert(T val) {
        auto node = std::make_unique<Node<T>>(val);
        node->next = std::move(next);
        node->prev = prev;
        if (prev)
            prev->next = std::move(node);
        if (next)
            next->prev = node.get();
    }

    void erase() {
        if (prev)
            prev->next = std::move(next);
        if (next)
            next->prev = prev;
    }

    ~Node() {
        // 应该输出13次, a原来的数据和b复制的数据
        printf("~Node()\n");   // 应输出多少次？为什么少了？
    }
};

template<typename T>
struct List {
    std::unique_ptr<Node<T>> head;  // `shared_ptr`不能变成`unique_ptr`, 反过来可以

    List() = default;

    List(List const &other) {
        printf("List 被拷贝！\n");
        // head = other.head;  // 这是浅拷贝！
        // 请实现拷贝构造函数为 **深拷贝**
        auto other_head = other.head.get();
        head = std::make_unique<Node<T>>(other_head->value);
        if (!other_head->next) {
            // 如果只有一个元素, 直接返回
            return;
        }

        auto cur_node = head.get();
        auto other_next_node = other_head->next.get();
        while (other_next_node) {
            auto node = std::make_unique<Node<T>>(other_next_node->value);
            node->prev = cur_node;
            cur_node->next = std::move(node);

            other_next_node = other_next_node->next.get();
            cur_node = cur_node->next.get();  // 经过`std::move`后的变量就相当于nullptr, 不应该再使用
        }
    }

    // 删除拷贝赋值函数后会先调用拷贝构造函数, 然后调用移动赋值函数传递一个拷贝
    // MARK: 在我的实验中并没有在`List b = a;`这条语句输出移动赋值函数被调用
    // 因为指令是`List b = a;`而不是`List b; b = a;`, 后一条会报错, 因为调用了拷贝赋值函数
    List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？

    List(List &&) = default;
    // List &operator=(List &&) = default;

    List &operator=(List && list) {
        printf("移动赋值函数被调用!\n");
        head = std::move(list.head);
        return *this;
    }

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
        if (head) {
            head->prev = node.get();
        }

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
