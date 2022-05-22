/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>
struct Node {
    // 这两个指针会造成什么问题？请修复
    //std::shared_ptr<Node> next;
    //std::shared_ptr<Node> prev;
    std::unique_ptr<Node> next;
    Node* prev;
    // 如果能改成 unique_ptr 就更好了!

    int value;

    // 这个构造函数有什么可以改进的？//使用初始化列表，不用等创建后再初始化赋值即可以在创建时赋值。
    Node(int val):value(val) {
    }

    void insert(int val) {
        auto node = std::make_unique<Node>(val);
        node->next = std::move(next);
        node->prev = prev;
        if (prev)
            prev->next = std::move(node);
        if (next)
            next->prev = node.get();
    }

    void erase() {
        if (next)
            next->prev = prev;
        if (prev){
            //prev->next = nullptr;//不能置为nullptr，否则链表当前节点之后的会全部解构。
            prev->next = std::move(next);
        }
    }

    ~Node() {
        printf("~Node(%d)\n",this->value);   // 应输出多少次？为什么少了？//prev清除了但next没有清除，智能指针引用没有清零，需要手动erase或者使用弱指针或uniptr
    }
};

struct List {
    //std::shared_ptr<Node> head;
    //Node* head{nullptr};
    std::unique_ptr<Node> head;

    List() = default;

    List(List const &other) {
        printf("List 被拷贝！\n");
        printf("拷贝构造函数\n");
        //head = other.head;  // 这是浅拷贝！
        // 请实现拷贝构造函数为 **深拷贝**
        Node* tailnode;//用于记录尾结点
        for(auto curr = other.front(); curr; curr = curr->next.get())
        {
            auto node = std::make_unique<Node>(curr->value);
            if(head)
            {
                node->prev = tailnode;
                tailnode->next = std::move(node);
                tailnode = tailnode->next.get();
            }
            else
            {   
                head = std::move(node);
                tailnode = head.get();
            }
            
        }
    }

    List &operator=(List const &other) = delete; // 为什么删除拷贝赋值函数也不出错？// 转而默认调用移动赋值函数。先创建一个即时对象，移动到目标对象上。

    List(List &&) = default;
    List &operator=(List &&other)//移动赋值
    {
        printf("移动赋值函数\n"); 
        printf("先解构原来的\n");;
        for(auto curr = this->front(); curr; curr=curr->next.get())
        {
            curr->erase();
        }
        printf("再移动Head\n");
        this->head = std::move(other.head);
        other.head = nullptr;
        return *this;
    }

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
        if (head)
        {
            head->prev = node.get();
            Node* tmp = head.get();
            node->next = std::move(head);
            head = std::move(node);
        }
        else
        {
            //node->next = std::move(head);
            head = std::move(node);
        }
    }

    Node *at(size_t index) const {
        auto curr = front();
        for (size_t i = 0; i < index; i++) {
            curr = curr->next.get();
        }
        return curr;
    }
};

void print(const List& lst) {  // 有什么值得改进的？//使用常引用，减少拷贝构造
    printf("[");
    for (auto curr = lst.front(); curr; curr = curr->next.get()) {
        printf(" %d", curr->value);
    }
    printf(" ]\n");
}

int main() {
    List a; //无参构造a

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

    List b = a; //拷贝a构造b， 深拷贝

    a.at(3)->erase();

    print(a);   // [ 1 4 2 5 7 ]
    print(b);   // [ 1 4 2 8 5 7 ]

    b = {};//默认构造一个{}， 移动赋值到b， b原来的元素应该被释放
    a = {};//默认构造一个{}， 移动赋值到a， a原来的元素应该被释放

    return 0;
}
