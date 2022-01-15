/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>

struct Node {
    // 这两个指针会造成什么问题？请修复 //死锁
    /*考虑到让头指针为空时，释放整个链表*/
    std::unique_ptr<Node>    next;
    Node                    *prev;
    // 如果能改成 unique_ptr 就更好了!

    int value;

    // 这个构造函数有什么可以改进的？ 
    // 初始化列表提高性能, explicit防止莫名的隐式类型转换
    explicit Node(int val) : value{val} { }

    void insert(int val) {
        auto node = std::make_unique<Node>(val);
        node->next = std::move(next);
        node->prev = prev;
        if (next)
            next->prev = node.get();
        if (prev)
            prev->next = std::move(node);
    }
    void erase() {    
        if (next)
            next->prev = prev;
        if (prev)
            prev->next = std::move(next);
    }

    ~Node() {
        static int counter = 1;
        printf("%d ~Node()\n", counter++);   // 应输出多少次？为什么少了？//应该13次，但锁死了
    }
};

struct List {
    std::unique_ptr<Node> head;

    List() = default;

    List(List const &other) {
        printf("List 被拷贝！\n");
        //head = other.head;  // 这是浅拷贝！
        // 请实现拷贝构造函数为 **深拷贝**

        // 处理头指针
        if(other.head == nullptr)
            return;
        head = std::make_unique<Node>(other.head->value);

        // 初始化当前指针
        Node                 *cur_ptr = head.get();                 //this
        std::unique_ptr<Node> cur_ptr_nxt;                          //this

        Node                 *cur_ptr_oth = other.head->next.get(); //other

        // 遍历链表
        while(cur_ptr_oth != nullptr)
        {
            cur_ptr_nxt = std::make_unique<Node>(cur_ptr_oth->value);
            cur_ptr_nxt->prev = cur_ptr;
            cur_ptr->next = std::move(cur_ptr_nxt);

            cur_ptr = cur_ptr->next.get();
            cur_ptr_oth = cur_ptr_oth->next.get();
        }
    }

    // List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？
    // 删除不出错是因为，临时变量是右值，a = {}, b = {} 调用是移动赋值。
    // 如果非要写赋值，可以改换成这个。
    List &operator=(const List &other)
    {
        head = {};
        this->~List();
        new(this) List{other};
        return *this;
    }

    // 转移头的控制权
    List(List &&) = default;
    List &operator=(List &&) = default;

    Node *front() const {
        return head.get();
    }

    int pop_front() {
        int ret = head->value;
        head = std::move(head->next);
        return ret;
    }

    void push_front(int value) {
        auto node = std::make_unique<Node>(value);
        node->next = std::move(head);
        if (node->next)
            node->next->prev = node.get();
        head = std::move(node);
    }

    Node *at(size_t index) const {
        auto curr = front();
        for (size_t i = 0; i < index; i++) {
            curr = curr->next.get();
        }
        return curr;
    }
};

void print(const List &lst) {  // 有什么值得改进的？不要深拷贝,const &就行
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


//  List b = a;
    List b;
    b = a;

    a.at(3)->erase();

    print(a);   // [ 1 4 2 5 7 ]
    print(b);   // [ 1 4 2 8 5 7 ]

    b = {};
    a = {};

    return 0;
}
