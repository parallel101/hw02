/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>

struct Node {
    // 这两个指针会造成什么问题？请修复
    // 当两个都是shared_ptr时 如果node1.next = node2, node2.prev = node1的话，
    // 当我们想recycle这两个node，node1 = nptr; node2 = nptr.
    // 由于node2.prev还指向node1，并且node1的next还只向node2，那么就会出现释放不了的清空
    // 所以最好一个是unique--省去了维护ref count的开销，另一个可以是原生pointer或者weak。
    // 我觉得这样declare attributes的好处还有维护爷爷，爸爸，儿子的关系。
    // 始终是你的儿子是unique，然后你儿子对自己不是unique，你儿子对你孙子是unique。
    // 保证了同时只有一个指向unique的东西存在。
    std::unique_ptr<Node> next;
    Node* prev;
    // 如果能改成 unique_ptr 就更好了!

    int value;

    // 这个构造函数有什么可以改进的？use list members to avoid direct assigning
    Node(int val) : value(val) {
    }
    
    // 由于next是unique，那么当我插入的时候，我要全权将next的所有权给当前的node
    void insert(int val) {
        auto node = std::make_unique<Node>(val);
        if (this->next) { //当前node的下一个node的prev应该指向新建的node
            this->next->prev = node.get();
        }
        node->next = std::move(this->next); //将我原本next的所有权给当前插入的node
        if (this->prev) {
            node->prev = this;
        }
        this->next = std::move(node); // 将新建node的所有全给我现在的这个node
        // if (prev)
        //     prev->next = node;
        // if (next)
        //     next->prev = node;
    }

    void erase() {
        // 这里的顺序尤为重要，之前因为先move了next的所有权导致了后access next->prev造成segfault
        if (next)
            next->prev = prev;  
            
        if (prev)
            prev->next = std::move(this->next);
        
    }

    ~Node() {
        printf("~Node(%d)\n", value);   // 应输出多少次？为什么少了？因为shared_ptr带来的循环指针导致总是有一个ref count让shared_ptr不能自动回收
    }
};

struct List {
    std::unique_ptr<Node> head;

    List() = default;

    // List(List const &other) {
    //     printf("List 被拷贝！\n");
    //     head = other.head;  // 这是浅拷贝！
    //     // 请实现拷贝构造函数为 **深拷贝**
    // }
    
    List(List const &other) {
        printf("深拷贝\n");
        // 我第一想法是把other.head的所有权给this，但是需求应该不是这样
        if (other.head) {
            head = std::make_unique<Node>(other.head->value);
            //直接other.head access是不行的，因为unique_ptr, 所以用.get()获取本体
            Node* curr_src = other.head.get(); 
            
            // 然后获取当前需要被copy into的内容
            Node* curr_dst = head.get(); 
            while (curr_src->next) {
                curr_dst->next = std::make_unique<Node>(curr_src->next->value);
                curr_dst->next->prev = curr_dst; // prev是本体指针，直接assign
                curr_src = curr_src->next.get();
                curr_dst = curr_dst->next.get();
            }
            
        }
        
    }

    List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？
    // 因为实现了move operator overloading

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

    Node *at(size_t index) const {
        auto curr = front();
        for (size_t i = 0; i < index; i++) {
            curr = curr->next.get();
        }
        return curr;
    }
    
    ~List() {
        printf("Destructor of list is called\n");
    }
};

void print(const List& lst) {  // 有什么值得改进的？加上&避免调用copy constructor
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