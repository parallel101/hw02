/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>

struct Node {
    // 这两个指针会造成什么问题？请修复
    // std::shared_ptr<Node> next;
    // std::shared_ptr<Node> prev;
    // 如果能改成 unique_ptr 就更好了!
    Node* prev;
    std::unique_ptr<Node> next;
    /*  frsama:
        链表相邻两点相互引用，会导致shared_ptr无法自动释放
        因为中间节点会被前驱的next和后继的prev同时指向，所以不能都改用unique_ptr
        因为链表初始化的时候，本身就有指针指向第一个元素，所以不应该是prev为智能指针
    */

    int value;

    // 这个构造函数有什么可以改进的？
    /* frsama:
        这个构造函数并没有初始化全部成员变量，需要增加
        unique_ptr有默认构造函数，可以不写
        另外为了兼容性，定义默认参数
    */
    Node(int val = 0, Node* Prev = nullptr, std::unique_ptr<Node> Next = nullptr):
        value(val),
        prev(Prev),
        next(std::move(Next))
    {}

    // frsama:这玩意好像没用到啊，说到底insert应该是List的功能才对，不应该封装在Node
    // void insert(int val) {
    //     auto node = std::make_shared<Node>(val);
    //     node->next = next;
    //     node->prev = prev;
    //     if (prev)
    //         prev->next = node;
    //     if (next)
    //         next->prev = node;
    // }

    /* frsama:
        感觉让自己删除自己是很危险的.....还是那个问题
        erase应该是List的功能，让外部删除才对，不应该自己删除自己
        但是这样的话，基本上等于要重构了，不想卷了
        
        有另一个办法，让前驱来删除目标节点，这样的会可能需要一个哨兵做头节点

        因为类成员函数实际上隐含了一个this指针参数，函数和this并不是强绑定关系，
        this被析构了，并不会影响正在运行的函数。
        所以用前驱删除this应该是可行的
        
        所以增加哨兵头，用前驱删除自己，应该是可行的
    */
    void erase() {
        next->prev = prev;
        prev->next = std::move(next);
    }

    ~Node() {
        printf("~Node()\n");   // 应输出多少次？为什么少了？
        /* frsama:
            应该是多了吧.....原来只有2次，现在有2+6+7次
            使用shared_ptr因为相邻节点相互持有对方，
            导致最后释放空间时，引用cnt不能降为0，结果一直不被释放
            调用析构函数只有主动删除节点的那两次

            使用了unique_ptr和Node*，除了主动调用析构函数的那两次
            还有最后a={};b={};调用的析构函数
            因为有哨兵，所以分别是1+5和1+6次。
            总共2+6+7次
            相比于shared_ptr实现方式，使用unique_ptr不会内存泄漏
        */
    }
};

struct List {
    std::unique_ptr<Node> head{std::make_unique<Node>()}; // frsama:head做了哨兵

    List() = default;

    List(List const &other) {
        printf("List 被拷贝！\n");
        // head = other.head;  // 这是浅拷贝！
        // 请实现拷贝构造函数为 **深拷贝**
        Node* p = other.head.get();
        Node* m_p = head.get();
        while(p->next) {
            p = p->next.get();
            m_p->next = std::make_unique<Node>(p->value, m_p);
            m_p = m_p->next.get();
        }
    }

    List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？
    /* frsama:
        样例是在初始化的时候用的“=”，调用的是拷贝构造函数。
    */

    List(List &&) = default;
    List &operator=(List &&) = default;

    Node *front() const {
        return head->next.get();// frsama:head是哨兵
    }

    int pop_front() {
        //原版不太安全，没有检查是否为空
        if(head->next){ // frsama:head是哨兵
            int ret = head->next->value;
            head->next = std::move(head->next->next);
            return ret;
        }
        return -1;//可能需要约定一个代表为空的常量
    }

    void push_front(int value) {
        auto node = std::make_unique<Node>(value, head.get());
        if(head->next){
            node->next = std::move(head->next);
            node->next->prev = node.get();
        }
        head->next = std::move(node);
    }

    Node *at(size_t index) const {
        auto curr = front();
        for (size_t i = 0; i < index; i++) {
            curr = curr->next.get();
        }
        return curr;
    }
};

void print(List& lst) {  // 有什么值得改进的？
// frsama:这样调用会产生List形参，用到构造函数，开销很大，可以改为引用传参
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
