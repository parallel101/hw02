/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory> // For three smart pointers
#include <cassert> // It's a invaluable thing

// Actually, we need public inherit
struct Node : std::enable_shared_from_this<Node> {
    int value;
    // 这两个指针会造成什么问题？请修复
    // Obviously, it will cause cycle reference
    std::shared_ptr<Node> next;
    std::weak_ptr<Node> prev;
    // 如果能改成 unique_ptr 就更好了!
    // We can do this from another branch.

    // 这个构造函数有什么可以改进的？
    // Explicit and Initializer list
    explicit Node(int val) : value(val) {
        /*
         *value = val;
         */
    }

    void insert(int val) {
        auto node = std::make_shared<Node>(val);
        node->next = next;
        node->prev = shared_from_this();
        if(next)
            next->prev = node;
        next = node;
    }

    void erase() {
        if (prev.lock())
            prev.lock()->next = next;
        if (next)
            next->prev = prev;
    }

    ~Node() {
        printf("~Node()\n");   // 应输出多少次？为什么少了？
    }
};

struct List {
    std::shared_ptr<Node> head;

    List() = default;

    List(List const &other) {
        printf("List copy constructor！\n");
        /*
         *head = other.head;  // 这是浅拷贝！
         */
        // 请实现拷贝构造函数为 **深拷贝**
        auto origin_head = other.head;
        if(!origin_head)
            return;

        auto current_head = std::make_shared<Node>(origin_head->value); 
        auto result = current_head;
        origin_head = origin_head->next;
        while(origin_head){
        //while(origin_head.get()){
            current_head->insert(origin_head->value);
            origin_head = origin_head->next; 
            current_head = current_head->next;
        }
        // Inside constructor, there is no content before, so it's redundant.
        // You can see the content of front()
        assert(!front());
        printf("front() == %p\n", front());
        for (auto cur = front(); cur; ) {
            auto tmp = cur;
            cur = cur->next.get();
            delete tmp;
        }
        head = result;
    }

    // From this source file context, there is no lvalue assignment copy action.
    List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？

    // Move sementics
    List(List &&) = default;
    List &operator=(List &&) = default;

    Node* front() const {
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
        // auto is Node*
        auto curr = front();
        for (size_t i = 0; i < index; i++) {
            curr = curr->next.get();
        }
        return curr;
    }
};

void print(const List& lst) {  // 有什么值得改进的？
    printf("[");
    for (auto cur = lst.front(); cur; cur = cur->next.get()) {
        printf(" %d", cur->value);
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
    a.push_front(9);

    print(a);   // [9 1 4 9 2 8 5 7]

    a.pop_front();
    print(a);   // [1 4 9 2 8 5 7]
                
    a.at(2)->erase();
    print(a);   // [ 1 4 2 8 5 7 ]

    List b = a;

    a.at(3)->erase();
    print(a);   // [ 1 4 2 5 7 ]

    a.at(3)->insert(10);
    print(a);   // [ 1 4 2 5 10 7 ]

    print(b);   // [ 1 4 2 8 5 7 ]

    b = {};
    a = {};

    return 0;
}
