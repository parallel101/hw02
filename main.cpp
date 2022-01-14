/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>

struct Node {
    // 这两个指针会造成什么问题？请修复
    // 答:循环引用造成引用计数不会清零，内存将无法释放
    
    //使用unique_ptr + 普通指针的组合
	std::unique_ptr<Node> next;
	Node* prev;

    int value;

     //改进如下:
    explicit Node(int val)
	{
		value = val;
		next = nullptr;
		prev = nullptr;
	}

    //修复insert功能
    void insert(int val)
	{
		//新建节点
		std::unique_ptr<Node> node = std::make_unique<Node>(val);
		//判断
		if (this->next)
		{
			//更新next的prev节点指向当前node
			next->prev = node.get();
			//将this->next的生命周期转移到node->next上,this->next为nullptr
			node->next = std::move(this->next);		
		}		
		//this->next指向当前node
		this->next = std::move(node);
		//更新node的prev指针
		node->prev = this;
	}

    void erase() {
		if (next)
		{
			next->prev = this->prev;		
		}
		if (prev)
		{
			prev->next = std::move(this->next);
		}
	}

    ~Node() {
        printf("~Node()\n");   // 应输出多少次？为什么少了？ // 答:应该输出7次(List a的析构次数) + 6(List b的析构次数)，shared_ptr的计数未归为零导致内存未释放
    }
};

struct List {
   //改为unique_ptr
    std::unique_ptr<Node> head;

    List() = default;

    List(List const &other) {
        printf("List 被拷贝！\n");
        //临时遍历other链表的指针,无需生命周期，因此使用普通指针
		Node* temp_curr_ptr = other.head.get();
		// 请实现拷贝构造函数为 **深拷贝**
		//this->head 指向新构造的头节点
		head = std::make_unique<Node>(other.head->value);
		//临时指向上一个节点的指针，,无需生命周期，因此使用普通指针
		Node* prev_ptr = head.get();
		//循环终止条件是当next为nullptr
		while (temp_curr_ptr->next != nullptr)
		{
			//构造新的next节点curr_next_ptr
			std::unique_ptr<Node> curr_next_ptr = std::make_unique<Node>(temp_curr_ptr->next->value);	
			//temp_shared_ptr的prev指向上一个节点
			curr_next_ptr->prev = prev_ptr;
			//让上一个节点的next节点指向temp_shared_ptr
			prev_ptr->next = std::move(curr_next_ptr);
			//在进入下一次循环时更新临时指向：
			//让上一节点指向temp_shared_ptr
			prev_ptr = prev_ptr->next.get();
			//临时遍历other链表的指针指向它的next
			temp_curr_ptr = temp_curr_ptr->next.get();
		}
		//完成深拷贝        
    }

    List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？答:因为默认移动赋值函数的存在，List b = a调用了拷贝构造函数构造临时的a备份，再转交生命周期给b。

    List(List &&) = default;
    List &operator=(List &&) = default;

    Node *front() const {
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
		std::unique_ptr<Node> node = std::make_unique<Node>(value);
		if (head)
		{
			head->prev = node.get();
			node->next = std::move(head);
		}
		this->head = std::move(node);
	}

    Node *at(size_t index) const {
        auto curr = front();
        for (size_t i = 0; i < index; i++) {
            curr = curr->next.get();
        }
        return curr;
    }
};

void print(const List& lst) {  // 有什么值得改进的？ 答：使用常量引用，避免拷贝
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
