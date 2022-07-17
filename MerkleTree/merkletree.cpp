#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include<cstdio>
#include<ctime>
#include<string>
#include<cstring>
#include<openssl/sha.h>
#include<iostream>

#define reg register int
#define FOR(i,a,b) for(reg i=a;i<b;++i)

using namespace std;

typedef unsigned int uint;
typedef unsigned char uchar;

const int MAXN = 100000;

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



uint Data[MAXN];


inline char* itos(int num, char* str, int radix)
{
	char index[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";//索引表
	unsigned unum;//存放要转换的整数的绝对值,转换的整数可能是负数
	int i = 0, j, k;//i用来指示设置字符串相应位，转换之后i其实就是字符串的长度；转换后顺序是逆序的，有正负的情况，k用来指示调整顺序的开始位置;j用来指示调整顺序时的交换。

	//获取要转换的整数的绝对值
	if (radix == 10 && num < 0)//要转换成十进制数并且是负数
	{
		unum = (unsigned)-num;//将num的绝对值赋给unum
		str[i++] = '-';//在字符串最前面设置为'-'号，并且索引加1
	}
	else unum = (unsigned)num;//若是num为正，直接赋值给unum

	//转换部分，注意转换后是逆序的
	do
	{
		str[i++] = index[unum % (unsigned)radix];//取unum的最后一位，并设置为str对应位，指示索引加1
		unum /= radix;//unum去掉最后一位

	} while (unum);//直至unum为0退出循环

	str[i] = '\0';//在字符串最后添加'\0'字符，c语言字符串以'\0'结束。

	//将顺序调整过来
	if (str[0] == '-') k = 1;//如果是负数，符号不用调整，从符号后面开始调整
	else k = 0;//不是负数，全部都要调整

	char temp;//临时变量，交换两个值时用到
	for (j = k; j <= (i - 1) / 2; j++)//头尾一一对称交换，i其实就是字符串的长度，索引最大值比长度少1
	{
		temp = str[j];//头部赋值给临时变量
		str[j] = str[i - 1 + k - j];//尾部赋值给头部
		str[i - 1 + k - j] = temp;//将临时变量的值(其实就是之前的头部值)赋给尾部
	}

	return str;//返回转换后的字符串
}


inline void printhex(uchar* c, int len)
{
	FOR(i, 0, len)printf("%02x", c[i]);
	puts("");
}


inline MerkleTreeNode* findLastNode(MerkleTreeNode* mt)//找到当前最后一个节点
{
	MerkleTreeNode* p = mt;
	if (p->left == nullptr && p->right == nullptr)return p;
	else if (p->right == nullptr && p->left != nullptr)return findLastNode(p->left);
	else return findLastNode(p->right);
}


inline MerkleTreeNode* findInsertPos(MerkleTreeNode* mt)//找到插入的位置
{
	MerkleTreeNode* p = mt->parent;
	//当前点左右子树均已存在，则向上查找
	while (p->left != nullptr && p->right != nullptr && p->parent != nullptr)p = p->parent;
	//找不到正确的位置
	if (p->parent == nullptr && p->left != nullptr && p->right != nullptr)return nullptr;
	//当前节点可以插入
	else return p;
}	


inline void updateNode(MerkleTreeNode* mt)
{
	if (mt->depth == 0)return;
	uchar m[65];
	m[0] = 1;//非叶节点：0x01
	FOR(i, 0, 32)m[i + 1] = mt->left->hash_str[i];
	if (mt->right != nullptr)FOR(i, 0, 32)m[33 + i] = mt->right->hash_str[i];
	else FOR(i, 0, 32)m[33 + i] = mt->left->hash_str[i];
	SHA256(m, 65, mt->hash_str);
}


//递归建树
inline MerkleTreeNode* build(MerkleTreeNode* mt, uint* arr, int num)
{
	MerkleTreeNode* node, * tmp, * p;
	int dep;
	if (num == 0)return mt;//剩余数量为0，构造完成，返回根节点。
	else
	{
		node = new MerkleTreeNode(0);//创建新的叶节点
		leaves[MAXN - num] = node;
		node->hash_num = *arr;
		char* data = new char[16];
		itos(*arr, data, 10);
		SHA256((const uchar*)data, strlen(data), node->hash_str);
		delete[] data;

		if (mt == nullptr)//为空，此时没有树，需要先创建一个头部节点
		{
			mt = new MerkleTreeNode(1);
			mt->left = node;//新的树，新叶子节点一定在左子树
			node->parent = mt;

			updateNode(mt);

		}
		else
		{
			p = findInsertPos(findLastNode(mt));
			if (p != nullptr)
			{
				if (p->left->left == nullptr && p->right == nullptr)
				{
					p->right = node;
					node->parent = p;
					while (p != mt)
					{
						updateNode(p);
						p = p->parent;
					}
					updateNode(p);
				}
				else
				{
					dep = p->depth - 1;
					tmp = new MerkleTreeNode(dep);
					p->right = tmp;
					tmp->parent = p;

					p = p->right;
					dep = p->depth - 1;

					while (dep > 0)
					{
						tmp = new MerkleTreeNode(dep);
						p->left = tmp;
						tmp->parent = p;

						p = p->left;
						dep--;
					}

					p->left = node;
					node->parent = p;
					while (p != mt)
					{
						updateNode(p);
						p = p->parent;
					}
					updateNode(p);
				}
			}
			else
			{
				tmp = mt;
				mt = new MerkleTreeNode(tmp->depth + 1);
				mt->left = tmp;
				tmp->parent = mt;

				tmp = new MerkleTreeNode(mt->depth - 1);
				mt->right = tmp;
				tmp->parent = mt;

				p = mt->right;
				dep = p->depth - 1;

				while (dep > 0)
				{
					tmp = new MerkleTreeNode(dep);
					p->left = tmp;
					tmp->parent = p;
					p = p->left;
					dep--;
				}

				p->left = node;
				node->parent = p;

				while (p != mt)
				{
					updateNode(p);
					p = p->parent;
				}
				updateNode(p);
			}
		}
	}
	return mt;
}


inline int isleft(MerkleTreeNode* mt)
{
	MerkleTreeNode* p = mt->parent;
	if (p->right == mt)return 0;
	else
		if (p->right)return 1;
		else return -1;
}


inline void mergePeer(MerkleTreeNode* mt, uchar* dst, uchar* hs)//寻找兄弟节点并计算合并hash值
{
	uchar h1[32], h2[32];
	uchar msg[65];
	msg[0] = 1;
	MerkleTreeNode* p = mt->parent, * tmp;
	if (!p)return;//没有上级节点，说明到达根，没有兄弟节点

	if (p->right == mt)//是右子树，则一定有左子树
	{
		tmp = p->left;
		FOR(i, 0, 32)
		{
			msg[i + 1] = tmp->hash_str[i];
			msg[i + 33] = hs[i];
		}
	}
	else//是左子树
	{
		if (p->right)//有右子树
		{
			tmp = p->right;
			FOR(i, 0, 32)
			{
				msg[i + 1] = hs[i];
				msg[i + 33] = tmp->hash_str[i];
			}
		}
		else //没有右子树，复制自身用于计算hash
		{
			FOR(i, 0, 32)
			{
				msg[i + 1] = hs[i];
				msg[i + 33] = hs[i];
			}
		}
	}
	SHA256(msg, 65, dst);
}


inline bool cmp(uchar* s1, uchar* s2, int len)
{
	FOR(i, 0, len)if (s1[i] != s2[i])return 1;
	return 0;
}


inline int inProve(const char* data, int id)//证明数据data在树中，且在第id号点中
{
	MerkleTreeNode* p = leaves[id];//方便查找，提前存下所有叶子节点的位置
	uchar h1[32];	
	uchar h[32];
	SHA256((const uchar*)data, strlen(data), h1);

	mergePeer(p, h, h1);

	while (p != root)
	{
		p = p->parent;
		if (p == root)break;
		mergePeer(p, h, h);
	}
	printf("final: ");
	printhex(h, 32);
	printf("root hash: ");
	printhex(root->hash_str, 32);
	return !cmp(h, root->hash_str, 32);
}


inline void init()
{
	FOR(i, 0, MAXN)Data[i] = i;
}



inline void dfs(MerkleTreeNode* t)
{
	if (!t->left && !t->right)
	{
		printf("leaf%d " , t->hash_num);
		printhex(t->hash_str, 32);
		return;
	}
	printhex(t->hash_str, 32);
	if (t->left)dfs(t->left);
	if (t->right)dfs(t->right);
}

#define exec

#ifdef exec
int main()
{
	init();
	FOR(i, 0, MAXN)root = build(root, Data + i, MAXN - i);

	printf("%d\n", inProve("0", 0));
	printf("%d\n", inProve("0", 1));
	return 0;
}
#endif // main

