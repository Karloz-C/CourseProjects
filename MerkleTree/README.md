# Merkle Tree

### 实现细节

#### 建立树结构

设置${10}^5$个节点。为方便检查，令第i个点中保存的数据为数字i的字符串形式。

节点结构如下：

包括左右子树，深度，保存的hash值。

```c++
struct MerkleTreeNode
{
	MerkleTreeNode* left,*right;
	MerkleTreeNode* parent;
	uint hash_num;
	uchar hash_str[32];
	int depth;
	MerkleTreeNode() { this->left = this->right = this->parent = nullptr; hash_num = 0; memset(hash_str, 0, 32); depth = 0; }
	MerkleTreeNode(int depth) { this->left = this->right = this->parent = nullptr; hash_num = 0; memset(hash_str, 0, 32); this->depth = depth; };
}*root = nullptr, *leaves[MAXN];
```

最关键的地方在于建树的过程。

最开始考虑的是递归建树方案，将整体的建立过程分解为每次向已有的完全二叉树中添加新的点，并选择合适位置，使其仍保持完全二叉树形式。

递归的方案是可行的，但这实际上每添加一个点就要调用一次，节点数量较多时会爆栈，因此必须选择循环的方式。

事实发现递归其实是完全没有必要的，直接修改为循环即可。具体方法为每添加一个点后返回当前的根节点指针，代表新的树结构。下次添加时，从根节点开始考虑。

下面介绍建树如何维护完全二叉树结构。（可以参考build函数代码）

我们以指针**mt**标志树的根节点，最开始**mt**为空。

当添加一个点时，有三种情况：

1. **mt**不存在，此时只需要创建新的根节点，将新点添加到根的左子树。
2. **mt**已存在，即已有一棵树。此时这棵树不是完整的完全二叉树，也即叶子节点的数量不是$2^n$个，有合适的位置可以顺序插入到树中。
3. 已有一颗完全二叉树，没有可用的位置插入。此时需要创建新的根节点，将已有的整个树看作根的左子树，并建立与其深度相同的右子树（一路向下创建左子树），并将新叶子节点插入到最后一层。

每次插入完成后，自底向上更新hash值，直到根节点。

#### 存在性证明

为了方便查找目的位置，提前保存了所有叶子结点的指针。

输入一个数据和位置，希望证明该数据确实在树中，且确实在给定的位置上。

只需要找到目的位置，计算给定数据的hash值作为该点可能的真实hash值，迭代向上每次与0x01||兄弟节点hash值级联，计算新的hash值，一直到根。最后比较根的hash值是否与计算结果相同。如果相同，说明数据正确，输出1，反之为0。

结果见下图，分别证明数据0是否在0号和1号节点中，可见证明有效。

![image-20220717205607711](C:\Users\Karloz\AppData\Roaming\Typora\typora-user-images\image-20220717205607711.png)
