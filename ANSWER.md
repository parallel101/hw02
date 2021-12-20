# hw02 submission

## 避免函数参数不必要的拷贝

`print(List lst) // 有什么可以改进的？`

​	首先这种的输入「input parameter by value」来说，cpp首先会对输入数据进行复制「deep copy」然后将复制的数字在函数的作用域内进行处理。这样的好处其实是避免了传入的数字的改动而造成了其他使用了这个传入参数的函数计算的结果的改动。但是对于我们一些需要性能的计算场景来说，我们其实并不希望这样的数据的复制的产生。因此，相应的改进视为将传入的改为数据本身「input parameter by reference」。

```print(List &lst)```

如果我们不希望数据在函数中被改变：

`print(List const &lst)`

## 修复智能指针造成的问题

​	在`Node`中使用到了两个智能指针:

```c++
    std::shared_ptr<Node> next;
    std::shared_ptr<Node> prev;
```

​	使用两个shared_ptr作为一个指向前后结点的pointer其实可能会造成一个问题：如果产生循环引用的话，指针没有办法完全进行释放，造成死锁。

​	作为一个解决方案，可以将其中一个改为weak_ptr就行。因为weak_ptr不会造成shared_ptr的计数器「counter的改变」，因此不会引发死锁的问题。

```cpp
std::shared_ptr<Node> next;
std::weak_ptr<Node> prev;
```

## 改用 `unique_ptr<Node>` 

### 为什么

​	改用unique_ptr<Node>是为了避免产生上一个Node进行deconstruction的时候原本的shared_ptr没有进行释放的情况。

### 怎么做

​	直接将两个pointer都设置为unique_ptr是不行的，因为这样会导致对prev或者next进行控制权提交的时候，结点位置为空的情况

​		

```
//考虑这样一种情况
struct Node{
	int value;
	unique_ptr<Node> prev;
	unique_ptr<Node> next;
};

auto a = make_unique<Node>(1);
auto b = make_unique<Node>(2);

a->next = std::move(b);//指令执行完之后pointer b被消除
a->next->prev = std::move(b); //错误，因为b已经没了。
a->next->prev = std::move(a);//这样执行完之后pointer a控制权移交给了a->next->prev。造成了一种悖论。因为a的控制权已经移交了。
```

为了避免这样一种情况，我们可以对前后两个pointers中其中一个改为普通的pointer，即：

```
struct Node{
	int value;
	unique_ptr<Node> prev;
	Node* next;
};

auto a = make_unique<Node>(1);
auto b = make_unique<Node>(2);

a->next = std::move(b);//指令执行完之后pointer b被消除
a->next->prev = a;//这样逻辑上避免了出现悖论的情况
```

## 实现拷贝构造函数为深拷贝

实现deep copy的思路：

1. 首先将每一个`Node`进行deep copy
2. 按照先后顺序将生成的`Nodes`连起来

但是在实现的过程中我发现不免要进行造轮子的情况：需要实现一个从前到后对每一个`Node`进行连接的函数。而为了避免这种情况，我在这里用了现成的`push_front`。这时候原本的思路就变成了：

1. 首先将每一个`Node`进行deep copy
2. 按照先后顺序的反序将生成的`Nodes`连起来

不过这种方式存在一个问题，如果需要数据的反序的话。我们需要借助stack来实现这个目的。不管是数据结构的stack还是程序的stack，这种方式的对空间的占用都是`O(n)`  。虽然避免了造轮子，但是还是需要占用空间。

## 说明为什么可以删除拷贝赋值函数

首先，我们删除的其实是默认的constructor，避免了编译器自己构造constructor来执行shallow copy的操作

我们自己实现了一个copy constructor，强制执行了deep copy。

## 改进 `Node` 的构造函数 5 分

通过传入reference来避免不必要的复制, 并且加入`const`来避免不必要的改动

```
Node(int const &value) : value(value) {}
```

## 实现interator

实现[interator的参考](https://docs.microsoft.com/en-us/cpp/cpp/increment-and-decrement-operator-overloading-cpp?view=msvc-170)主要考虑对以下运算符的overload

1. 对*进行overload来读取Node的数据
2. 对-> overload来读取当前Node本身
3. 对== 和！= overload判读是否符合条件
4. 实现 begin和end来完成循环的其实终止条件
5. 对++ overload来让当前iterator进行移动