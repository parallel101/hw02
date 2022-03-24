/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>

struct Node {
    /*  Q: 这两个指针会造成什么问题？
        A: 产生了环形引用，导致无法解构。 */
    std::unique_ptr<Node> next;
    struct Node* prev;
    // 如果能改成 unique_ptr 就更好了!

    int value;

    /*  Q: 这个构造函数有什么可以改进的？
        A: 添加 explicit 避免隐式转换。 */
    explicit Node(int val) {
        value = val;
    }

    void insert(int val) {
        auto node = std::make_unique<Node>(val);
        node->prev = prev;
        if (next)
            next->prev = node.get();
        if (prev)
            prev->next = std::move(node);
        node->next = std::move(next);
    }

    void erase() {
        if (prev)
            prev->next = std::move(next);
        if (next)
            next->prev = prev;
    }

    ~Node() {
        /*  Q: 应输出多少次？为什么少了？
            A: 产生了环形引用，导致无法解构。 */
        printf("~Node(%d)\n", value);
    }
};

struct List {
    std::unique_ptr<Node> head;
    struct Node* tail;

    List() {
        auto node_tail = std::make_unique<Node>(0);
        tail = node_tail.get();
        head = std::move(node_tail);
    };

    List(List const &other) {
        printf("List 被拷贝！\n");
        // 请实现拷贝构造函数为 **深拷贝**
        auto node_tail = std::make_unique<Node>(0);
        tail = node_tail.get();
        head = std::move(node_tail);
        std::unique_ptr<Node>* last = nullptr;
        for (auto curr = other.front(); curr != other.tail; curr = curr->next.get()) {
            auto node = std::make_unique<Node>(curr->value);
            if (last == nullptr) {
                node->next = std::move(head);
                head = std::move(node);
                last = &head;
            } else {
                node->next = std::move((*last)->next);
                node->prev = last->get();
                (*last)->next = std::move(node);
                last = &((*last)->next);
            }
            tail->prev = last->get();
        }
    }

    /*  Q: 为什么删除拷贝赋值函数也不出错？
        A: 因为这相当于使用拷贝构造函数就地构造出对象，从而进一步调用移动赋值函数完成整个过程。
           如果使用 explicit 关键词修饰拷贝构造函数，则会产生编译错误。 */
    List &operator=(List const &) = delete;

    List(List &&) = default;
    List &operator=(List &&) = default;

    Node *front() const {
        return head.get();
    }

    int pop_front() {
        int ret = head->value;
        if (head->next.get() == tail) {
            head = nullptr;
            tail->prev = nullptr;
        } else {
            head = std::move(head->next);
        }
        return ret;
    }

    void push_front(int value) {
        auto node = std::make_unique<Node>(value);
        if (head)
            head->prev = node.get();
        else
            tail->prev = node.get();
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

    class iterator : public std::iterator<std::input_iterator_tag, Node, int,
                                          const Node *, int> {
        Node *node;

    public:
        explicit iterator(Node *_node) : node(_node) {}
        iterator &operator++() {
            node = node->next.get();
            return *this;
        }
        iterator operator++(int) {
            iterator retval = *this;
            ++(*this);
            return retval;
        }
        bool operator==(iterator other) const { return node == other.node; }
        bool operator!=(iterator other) const { return !(*this == other); }
        reference operator*() const { return node->value; }
    };
    iterator begin() { return iterator(head.get()); }
    iterator end() { return iterator(tail); }
};


/*  Q: 有什么值得改进的？
    A: 改为引用传递，减少一次深拷贝。 */
void print(List &lst) {
    printf("[");
    for (auto it = lst.begin(); it != lst.end(); it++) {
        printf(" %d", *it);
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

    puts("------------");
    b = {};
    puts("------------");
    a = {};
    puts("------------");

    return 0;
}
