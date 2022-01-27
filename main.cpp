/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>

struct Node
{
    // 这两个指针会造成什么问题？请修复
#ifdef USE_SHARED_PTR
    std::shared_ptr<Node> next;
    std::weak_ptr<Node> prev;

#else
    // 如果能改成 unique_ptr 就更好了!
    std::unique_ptr<Node> next;
    Node *prev{nullptr};
#endif //USE_SHARED_PTR
    int value;

    // 这个构造函数有什么可以改进的？
    explicit Node(int val) : value(val)
    {
    }

    void insert(int val)
    {
#ifdef USE_SHARED_PTR
        auto node = std::make_shared<Node>(val);
        node->next = next;
        node->prev = prev;
        if (!prev.expired())
            prev.lock()->next = node;
        if (next)
            next->prev = node;
#else
        auto node = std::make_unique<Node>(val);
        auto next_node = next.get();
        node->next = std::move(next);
        node->prev = prev;
        if (prev)
            prev->next = std::move(node);
        if (next_node)
            next_node->prev = node.get();
#endif //USE_SHARED_PTR
    }

    void erase()
    {
#ifdef USE_SHARED_PTR
        if (!prev.expired())
            prev.lock()->next = next;
        if (next)
            next->prev = prev;
#else
        auto next_node = next.get();
        if (prev)
            prev->next = std::move(next);
        if (next_node)
            next_node->prev = prev;
#endif //USE_SHARED_PTR
    }

    ~Node()
    {
        printf("~Node()\n"); // 应输出多少次？为什么少了？
    }
};

struct List
{
#ifdef USE_SHARED_PTR
    std::shared_ptr<Node> head;
#else
    std::unique_ptr<Node> head;
#endif

    List() = default;

    List(List const &other)
    {
        printf("List 被拷贝！\n");
        // head = other.head; // 这是浅拷贝！
        // 请实现拷贝构造函数为 **深拷贝**

#ifdef USE_SHARED_PTR
        //不能用原始指针初始化多个shared_ptr，否则会造成内存泄漏
        head = std::make_shared<Node>(other.head->value);

        auto p1 = head, p2 = other.head->next;
        while (p2)
        {
            auto p1_node = std::make_shared<Node>(p2->value);
            p1_node->prev = p1;
            p1->next = p1_node;
            p1 = p1_node;
            p2 = p2->next;
        }
#else
        head = std::make_unique<Node>(other.head->value);
        auto p1 = head.get();
        auto p2 = (other.head->next).get();
        while (p2)
        {
            auto p = std::make_unique<Node>(p2->value);
            p->prev = p1;
            p1->next = std::move(p);
            p1 = (p1->next).get();
            p2 = (p2->next).get();
        }
#endif //USE_SHARED_PTR
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
#ifdef USE_SHARED_PTR
        int ret = head->value;
        head = head->next;
        return ret;

#else
        int ret = head->value;
        head = std::move(head->next);
        return ret;
#endif //USE_SHARED_PTR
    }

    void push_front(int value)
    {
#ifdef USE_SHARED_PTR
        auto node = std::make_shared<Node>(value);
        node->next = head;
        if (head)
            head->prev = node;
        head = node;
#else

        auto node = std::make_unique<Node>(value);
        auto head_ptr = head.get();
        node->next = std::move(head);
        if (head_ptr)
            head_ptr->prev = node.get();
        head = std::move(node);
#endif //USE_SHARED_PTR
    }

    Node *at(size_t index) const
    {
        auto curr = front();
        for (size_t i = 0; i < index; i++)
        {
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
