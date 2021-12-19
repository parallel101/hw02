/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>

struct Node {
    // 这两个指针会造成什么问题？请修复
    // 如果能改成 unique_ptr 就更好了!
    std::unique_ptr<Node> next;
    Node *prev;

    int value;

    // 这个构造函数有什么可以改进的？
    Node(int val) {
        value = val;
    }

    void insert(int value) {
        auto node = std::make_unique<Node>(value);
        node->value = value;
        node->next = std::move(next);
        node->prev = prev;
        Node* tmp_node = node.get();
        if (prev)
            prev->next = std::move(node);
        if (next)
            next->prev = tmp_node;
    }

    void erase() {
        // 换了下顺序，否则next move到prev-next后，next指针就失效了
        if (next.get())
            next->prev = prev;
        if (prev)
            prev->next = std::move(next);
    }

    ~Node() {
        // Node 构建多少次，就应该输出多少次，如果出现指针未释放，则会减少．
        printf("~Node()\n");   // 应输出多少次？为什么少了？
    }
};

struct List {
    std::unique_ptr<Node> head;

    List() = default;

    List(List const &other) {
        printf("List 被拷贝！\n");
        // head = other.head;  // 这是浅拷贝！
        // 请实现拷贝构造函数为 **深拷贝**
        if(head.get()){
            Node* tmp_ptr = other.head.get();
            // 根据next指针找到队尾
            while(tmp_ptr->next.get()!= nullptr){
                tmp_ptr = tmp_ptr->next.get();
            }
            // 从队尾开始向前遍历，并push值到新的List中
            while (tmp_ptr != other.head.get())
            {
                push_front(tmp_ptr->value);
                tmp_ptr = tmp_ptr->prev;
            }
            // push　head
            push_front(tmp_ptr->value);
        }else{
            head = nullptr;
        }
    }

    List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？

    List(List &&) = default;
    List &operator=(List &&) = default;

    Node *front() const {
        return head.get();
    }

    int pop_front() {
        int ret = -1;
        if(head.get()){
            ret = head->value;
            if(head->next.get()){
                head = std::move(head->next);
            }else{
                head = nullptr;
            }
            printf("ret = %d \n",ret);
        }else{
            printf("size = 0!\n");
            return -1;
        }
        return ret;
    }

    void push_front(int value) {
        auto node = std::make_unique<Node>(value);
        Node* tmp_head = head.get();
        node->next = std::move(head);
        if (tmp_head)
            tmp_head->prev = node.get();
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

void print(List &lst) {  // 有什么值得改进的？
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

    // 似乎这里的 at(2)　无法做越界保护？
    a.at(2)->erase();

    print(a);   // [ 1 4 2 8 5 7 ]
    
    // 编译器调用拷贝构造，然后使用移动赋值函数
    // List b = List{a};
    List b = a;

    a.at(3)->erase();

    print(a);   // [ 1 4 2 5 7 ]
    print(b);   // [ 1 4 2 8 5 7 ]

    b = {};
    a = {};

    return 0;
}
