/* ��������ָ��ʵ��˫������ */
#include <cstdio>
#include <memory>

struct Node {
	// ������ָ������ʲô���⣿���޸�
	std::unique_ptr<Node> next;
	Node* prev;
    
	// ����ܸĳ� unique_ptr �͸�����!

    int value;

    Node(int value) : next(nullptr),prev(NULL),value(value) {
		
	}  // ��ʲô���ԸĽ��ģ�

    void insert(int value) {
        auto node = std::unique_ptr<Node>(new Node(value));
        node->next = std::move(next);

		if (node->next)
		{
			node->prev = node->next->prev;
			node->next->prev = node.get();
		}
		next = std::move(node);
    }

    void erase() {
		
		if (next)
		{
			next->prev = prev;
		}
		if (prev)
		{
			prev->next = std::move(next);
		}
		
    }

    ~Node() {
        printf("~Node()\n");    // Ӧ������ٴΣ�Ϊʲô���ˣ�
    }
};

struct List {
	std::unique_ptr<Node> head;

    List() = default;

    List(List const &other) {
        printf("List  ������\n");
		head.reset();
		if (!other.front())return;
		Node* pOtherNodeFront = other.front();
		head = std::unique_ptr<Node>(new Node(pOtherNodeFront->value));
		Node* pthisNodeFront = head.get();
		while (pOtherNodeFront->next)
		{
			pthisNodeFront->insert(pOtherNodeFront->next->value);
			pthisNodeFront = pthisNodeFront->next.get();
			pOtherNodeFront = pOtherNodeFront->next.get();
		}
		// ��ʵ�ֿ������캯��Ϊ **���**
    }

    List &operator=(List const &) = delete;  // Ϊʲôɾ��������ֵ����Ҳ������

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
        auto node = std::unique_ptr<Node>(new Node(value));
        node->next = std::move(head);
		if (node->next)
		{
			node->next->prev = node.get();
		}
		head = std::move(node);
    }

    Node *at(size_t index) const {
        auto curr = front();
        for (size_t i = 0; i < index; i++) {
			if (!curr)
			{
				printf("Error:Exceed List Volumn!/n");
				return NULL;
			}	
			curr = curr->next.get();
			
        }
        return curr;
    }
};

void print(List lst) {  
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
