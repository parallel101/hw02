/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>
#include <stack>

struct Node {
    // 这两个指针会造成什么问题？请修复
    // std::shared_ptr<Node> next;
    // std::shared_ptr<Node> prev;

    // std::weak_ptr<Node> prev;
    // 如果能改成 unique_ptr 就更好了!

    std::unique_ptr<Node> next;
    Node *prev;

    int value;

    // Node(int value) : value(value) {} // 有什么可以改进的？
    Node(int const &value) : value(value) {}  //通过传入reference来避免不必要的复制

    // void insert(int value) {
    //     auto node = std::make_shared<Node>(value);
    //     node->value = value;
    //     node->next = next;
    //     node->prev = prev;
    //     if (prev)
    //         prev->next = node;
    //     if (next)
    //         next->prev = node;
    // }

    // void erase() {
    //     if (prev)
    //         prev->next = next;
    //     if (next)
    //         next->prev = prev;
    // }

    void insert(int value) {
        std::unique_ptr<Node> node = std::make_unique<Node>(value);
        node->value = value;
        node->next = std::move(next); //need to take the next pointer will expire into count!
        node->prev = prev;
        if (prev)
            prev->next = std::move(node);
        if (next)
            next->prev = node.get();
    }

    void erase() {
        printf("erasing Node(%d)\n",value);
        if (prev)
            prev->next = std::move(next); //should beware the next pointer will expire
        if (prev->next)
            prev->next->prev = prev;
    }

    ~Node() {
        printf("~Node(%d)\n", value);   // 应输出多少次？为什么少了？
    }
};

struct List {
    std::unique_ptr<Node> head;
    // Node* head;
    List() = default;

    List(List &other) {
        printf("List 被拷贝！\n");
        // head = other.head;  // 这是浅拷贝!
        // 请实现拷贝构造函数为 **深拷贝**
        
        //try to avoid building wheels
        /*using stack*/
        std::stack<int> stk;
        List b;
        for(auto curr=other.front(); curr; curr = curr->next.get()){
            stk.push(curr->value);
        }
        
        while(!stk.empty()){
            b.push_front(stk.top());
            stk.pop();
        }
        head = std::move(b.head);
    }

    List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？
                                            //首先，我们删除的其实是默认的constructor，避免了编译器自己构造constructor来执行shallow copy的操作
                                            //我们自己实现了一个copy constructor，指定了赋值方式

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
        if (head)
            head->prev = node.get();
        node->next = std::move(head);
        head = std::move(node);
    }

    void push_back(int value){
        auto node = std::make_unique<Node>(value);
        if(head)
            head->next = std::move(node);
        head->next->prev = head.get();
        head = std::move(head->next);
    }

    Node *at(size_t index) const {
        auto curr = front();
        for (size_t i = 0; i < index; i++) {
            curr = curr->next.get();
        }
        return curr;
    }
};

class iterator{
    private:
        Node *ptr;
    public:
        iterator(Node *ptr = nullptr) : ptr(ptr) {}

        int operator* () const{
            return ptr->value;
        }

        Node* operator->(){
            return ptr;
        } 
        
        iterator& operator++(){
            ptr = ptr->next.get();
            return *this;
        }

        bool operator==(const iterator &other){
            return ptr==other.ptr;
        }

        bool operator!=(const iterator &other){
            return ptr!=other.ptr;
        }

        iterator begin(){
            Node *tmp = ptr;
            while(tmp->prev)
                tmp = tmp->prev;
            return iterator(tmp);
        }

        iterator end(){
            Node *tmp = ptr;
            while(tmp)
                tmp = tmp->next.get();
            return iterator(tmp);
        }

};

void print(List const &lst, bool use_iterator=false){
    if(use_iterator){
        printf("using iterator!!! \n");
        iterator it = iterator(lst.front());
        auto start = it.begin();
        auto end = it.end();
        printf("[");
        for(it=start;it!=end;++it){
            printf(" %d", it->value);
        }
        printf(" ]\n");
    }else{
        printf("using default mode!!! \n");
        printf("[");
        for (auto curr = lst.front(); curr; curr = curr->next.get()) {
            printf(" %d", curr->value);
        }
        printf(" ]\n");
    }
}

// void print(List const &lst) {  // 有什么值得改进的？ 
// /*原函数的input by value 会默认调用copy constructor,
// 这样会影响程序的性能问题，毕竟仅仅是print的话其实不会对原数据进行改动。
// 所以为了避免调用copy constructor，设置程序为input by reference。
// 同时，为了避免数据修改，我们设置为const reference。
// */    
//     if(!lst.front()){
//         printf("There is something wrong with the list\n");
//         return;
//     }
//     printf("[");
//     for (auto curr = lst.front(); curr; curr = curr->next.get()) {
//         printf(" %d", curr->value);
//     }
//     printf(" ]\n");
// }

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

    print(a, true);
    b = {};
    a = {};
    return 0;
}
