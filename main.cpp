/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>

struct Node
{
    // 这两个指针会造成什么问题？请修复
    std::unique_ptr<Node> next;
    Node *prev;
    // 如果能改成 unique_ptr 就更好了!

    int value;

    // 这个构造函数有什么可以改进的？
    Node(int val) : value(val), prev(nullptr), next(nullptr)
    {
    }

    void insert(int val)
    {
        auto node = std::make_unique<Node>(val);
        node->next = std::move(next);
        node->prev = this;
        if (node->next)
            node->next->prev = node.get();
        next = std::move(node);
    }

    void erase()
    {

        if (next)
            next->prev = prev;
        if (prev)
            prev->next = std::move(next);
        prev = nullptr;
    }

    ~Node()
    {
        printf("~Node()\n"); // 应输出多少次？为什么少了？
    }
};

struct List
{
    std::unique_ptr<Node> head;

    List() = default;

    List(List const &other)
    {
        printf("List 被拷贝！\n");
        // head = other.head; // 这是浅拷贝！
        // 请实现拷贝构造函数为 **深拷贝**
        if (other.head)
        {
            head = std::make_unique<Node>(other.head->value);
            Node *curr = head.get();
            Node *nextCurr = other.head->next.get();
            while (nextCurr)
            {
                auto newNode = std::make_unique<Node>(nextCurr->value);
                curr->next = std::move(newNode);
                curr->next->prev = curr;
                curr = curr->next.get();
                nextCurr = nextCurr->next.get();
            }
        }
    }

    List &operator=(List const &) = delete; // 为什么删除拷贝赋值函数也不出错？

    List(List &&) = default;
    List &operator=(List &&) = default;

    Node *front() const
    {
        return head.get();
    }

    int pop_front()
    {
        int ret = head->value;
        head = std::move(head->next);
        return ret;
    }

    void push_front(int value)
    {
        auto node = std::make_unique<Node>(value);
        node->next = std::move(head);
        if (node->next)
            node->next->prev = node.get();
        head = std::move(node);
    }

    Node *at(size_t index) const
    {
        auto curr = front();
        for (size_t i = 0; i < index; i++)
        {
            if (!curr)
                return nullptr;
            curr = curr->next.get();
        }
        return curr;
    }
};

void print(const List &lst)
{ // 有什么值得改进的？
    printf("[");
    for (auto curr = lst.front(); curr; curr = curr->next.get())
    {
        printf(" %d", curr->value);
    }
    printf(" ]\n");
}

int main()
{
    List a;

    a.push_front(7);
    a.push_front(5);
    a.push_front(8);
    a.push_front(2);
    a.push_front(9);
    a.push_front(4);
    a.push_front(1);

    print(a); // [ 1 4 9 2 8 5 7 ]

    a.at(2)->erase();

    print(a); // [ 1 4 2 8 5 7 ]

    List b = a;

    a.at(3)->erase();

    print(a); // [ 1 4 2 5 7 ]
    print(b); // [ 1 4 2 8 5 7 ]

    b = {};
    a = {};

    return 0;
}
