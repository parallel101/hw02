/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>
#include <iostream>

template <typename T>
struct Node
{
    // 这两个指针会造成什么问题？请修复
    std::unique_ptr<Node> next;
    Node*                 prev = nullptr;
    // 如果能改成 unique_ptr 就更好了!

    T value;

    // 这个构造函数有什么可以改进的？
    explicit Node(T val) : value{ std::move(val) } {}

    void insert(T val)
    {
        auto node = std::make_unique<Node<T>>(std::move(val));

        node->prev = prev;
        if (next) next->prev = node.get();

        node->next = std::move(next);
        if (prev) prev->next = std::move(node);
    }

    void erase()
    {
        if (next) next->prev = prev;
        if (prev) prev->next = std::move(next);
    }

    ~Node()
    {
        printf("~Node()\n");  // 应输出多少次？为什么少了？
    }
};

template <typename T>
struct List
{
    std::unique_ptr<Node<T>> head;

    List() = default;

    List(const List& other)
    {
        printf("List 被拷贝！\n");
        //head = other.head;  // 这是浅拷贝！
        // 请实现拷贝构造函数为 **深拷贝**

        auto store = std::make_unique<Node<T>>(T{});
        auto ptr   = other.front();
        auto res   = store.get();

        while (ptr)
        {
            auto node  = std::make_unique<Node<T>>(ptr->value);
            node->prev = res->prev;
            node->next = std::move(res->next);
            res->next  = std::move(node);
            res        = res->next.get();
            ptr        = ptr->next.get();
        }

        head = std::move(store->next);
        head->prev = nullptr;
    }

    List& operator=(const List&) = delete;  // 为什么删除拷贝赋值函数也不出错？

    List(List&&)            = default;
    List& operator=(List&&) = default;

    Node<T>* front() const { return head.get(); }

    T pop_front()
    {
        T ret = std::move(head->value);
        head  = std::move(head->next);
        return ret;
    }

    void push_front(T value)
    {
        auto node = std::make_unique<Node<T>>(std::move(value));
        if (head) head->prev = node.get();
        node->next = std::move(head);
        head       = std::move(node);
    }

    Node<T>* at(size_t index) const
    {
        auto curr = front();
        for (size_t i = 0; i < index && curr; i++)
        {
            curr = curr->next.get();
        }
        return curr;
    }
};

template <typename T>
void print(const List<T>& lst)
{  // 有什么值得改进的？
    std::cout << "[";
    for (auto curr = lst.front(); curr; curr = curr->next.get())
    {
        std::cout << " " << curr->value;
    }
    std::cout << " ]" << std::endl;
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
