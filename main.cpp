/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>

template<typename T>
struct Node
{
    // 这两个指针会造成什么问题？请修复
    std::unique_ptr<Node<T>> next;
    Node<T>*                 prev;
    // 如果能改成 unique_ptr 就更好了!

    T value;

    // 这个构造函数有什么可以改进的？
    Node(T val) : next(nullptr), prev(nullptr), value(val)
    {}
    Node(T val, std::unique_ptr<Node<T>> next, Node<T>* prev)
        : next(std::move(next)), prev(prev), value(val)
    {
        if (next)
            next->prev = this;
    }

    void insert(T val)
    {
        prev->next = std::make_unique<Node<T>>(val, std::move(next), prev);
    }

    void erase()
    {
        if (next)
            next->prev = prev;
        if (prev)
            prev->next = std::move(next);
    }

    ~Node()
    {
        printf("~Node()\n");   // 应输出多少次？为什么少了？
    }
};

template<typename T>
struct List
{
    std::unique_ptr<Node<T>> head;

    List() = default;

    List(List<T> const& other)
    {
        printf("List 被拷贝！\n");
        // head = other.head;   // 这是浅拷贝！
        // 请实现拷贝构造函数为 **深拷贝**
        auto* other_ptr = other.head.get();
        auto* this_ptr  = head.get();
        while (other_ptr) {
            if (this_ptr) {
                this_ptr->next = std::make_unique<Node<T>>(other_ptr->value, nullptr, this_ptr);
                this_ptr       = this_ptr->next.get();
            }
            else {
                head     = std::make_unique<Node<T>>(other_ptr->value, nullptr, nullptr);
                this_ptr = head.get();
            }
            other_ptr = other_ptr->next.get();
        }
    }

    List& operator=(List const&) = delete;   // 为什么删除拷贝赋值函数也不出错？

    List(List&&)  = default;
    List& operator=(List&&) = default;

    Node<T>* front() const
    {
        return head.get();
    }

    T pop_front()
    {
        T ret = head->value;
        head  = std::move(head->next);
        return ret;
    }

    void push_front(T value)
    {
        auto node = std::make_unique<Node<T>>(value);
        if (head) {
            node->next       = std::move(head);
            node->next->prev = node.get();
        }
        head = std::move(node);
    }

    Node<T>* at(size_t index) const
    {
        auto curr = front();
        for (size_t i = 0; i < index; i++) {
            curr = curr->next.get();
        }
        return curr;
    }
};

template<typename T>
void print(const List<T>& lst);

template<>
void print(const List<int>& lst)
{   // 有什么值得改进的？
    printf("[");
    for (auto curr = lst.front(); curr; curr = curr->next.get()) {
        printf(" %d", curr->value);
    }
    printf(" ]\n");
}
template<>
void print(const List<char>& lst)
{   // 有什么值得改进的？
    printf("[");
    for (auto curr = lst.front(); curr; curr = curr->next.get()) {
        printf(" %c", curr->value);
    }
    printf(" ]\n");
}

int main()
{
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

    List<char> ac;

    ac.push_front('7');
    ac.push_front('5');
    ac.push_front('8');
    ac.push_front('2');
    ac.push_front('9');
    ac.push_front('4');
    ac.push_front('1');

    print(ac);   // [ 1 4 9 2 8 5 7 ]

    ac.at(2)->erase();

    print(ac);   // [ 1 4 2 8 5 7 ]

    List<char> bc = ac;

    ac.at(3)->erase();

    print(ac);   // [ 1 4 2 5 7 ]
    print(bc);   // [ 1 4 2 8 5 7 ]
    bc = {};
    ac = {};
    return 0;
}
