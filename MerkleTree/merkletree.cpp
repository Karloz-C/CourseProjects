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
	char index[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";//������
	unsigned unum;//���Ҫת���������ľ���ֵ,ת�������������Ǹ���
	int i = 0, j, k;//i����ָʾ�����ַ�����Ӧλ��ת��֮��i��ʵ�����ַ����ĳ��ȣ�ת����˳��������ģ��������������k����ָʾ����˳��Ŀ�ʼλ��;j����ָʾ����˳��ʱ�Ľ�����

	//��ȡҪת���������ľ���ֵ
	if (radix == 10 && num < 0)//Ҫת����ʮ�����������Ǹ���
	{
		unum = (unsigned)-num;//��num�ľ���ֵ����unum
		str[i++] = '-';//���ַ�����ǰ������Ϊ'-'�ţ�����������1
	}
	else unum = (unsigned)num;//����numΪ����ֱ�Ӹ�ֵ��unum

	//ת�����֣�ע��ת�����������
	do
	{
		str[i++] = index[unum % (unsigned)radix];//ȡunum�����һλ��������Ϊstr��Ӧλ��ָʾ������1
		unum /= radix;//unumȥ�����һλ

	} while (unum);//ֱ��unumΪ0�˳�ѭ��

	str[i] = '\0';//���ַ���������'\0'�ַ���c�����ַ�����'\0'������

	//��˳���������
	if (str[0] == '-') k = 1;//����Ǹ��������Ų��õ������ӷ��ź��濪ʼ����
	else k = 0;//���Ǹ�����ȫ����Ҫ����

	char temp;//��ʱ��������������ֵʱ�õ�
	for (j = k; j <= (i - 1) / 2; j++)//ͷβһһ�Գƽ�����i��ʵ�����ַ����ĳ��ȣ��������ֵ�ȳ�����1
	{
		temp = str[j];//ͷ����ֵ����ʱ����
		str[j] = str[i - 1 + k - j];//β����ֵ��ͷ��
		str[i - 1 + k - j] = temp;//����ʱ������ֵ(��ʵ����֮ǰ��ͷ��ֵ)����β��
	}

	return str;//����ת������ַ���
}


inline void printhex(uchar* c, int len)
{
	FOR(i, 0, len)printf("%02x", c[i]);
	puts("");
}


inline MerkleTreeNode* findLastNode(MerkleTreeNode* mt)//�ҵ���ǰ���һ���ڵ�
{
	MerkleTreeNode* p = mt;
	if (p->left == nullptr && p->right == nullptr)return p;
	else if (p->right == nullptr && p->left != nullptr)return findLastNode(p->left);
	else return findLastNode(p->right);
}


inline MerkleTreeNode* findInsertPos(MerkleTreeNode* mt)//�ҵ������λ��
{
	MerkleTreeNode* p = mt->parent;
	//��ǰ�������������Ѵ��ڣ������ϲ���
	while (p->left != nullptr && p->right != nullptr && p->parent != nullptr)p = p->parent;
	//�Ҳ�����ȷ��λ��
	if (p->parent == nullptr && p->left != nullptr && p->right != nullptr)return nullptr;
	//��ǰ�ڵ���Բ���
	else return p;
}	


inline void updateNode(MerkleTreeNode* mt)
{
	if (mt->depth == 0)return;
	uchar m[65];
	m[0] = 1;//��Ҷ�ڵ㣺0x01
	FOR(i, 0, 32)m[i + 1] = mt->left->hash_str[i];
	if (mt->right != nullptr)FOR(i, 0, 32)m[33 + i] = mt->right->hash_str[i];
	else FOR(i, 0, 32)m[33 + i] = mt->left->hash_str[i];
	SHA256(m, 65, mt->hash_str);
}


//�ݹ齨��
inline MerkleTreeNode* build(MerkleTreeNode* mt, uint* arr, int num)
{
	MerkleTreeNode* node, * tmp, * p;
	int dep;
	if (num == 0)return mt;//ʣ������Ϊ0��������ɣ����ظ��ڵ㡣
	else
	{
		node = new MerkleTreeNode(0);//�����µ�Ҷ�ڵ�
		leaves[MAXN - num] = node;
		node->hash_num = *arr;
		char* data = new char[16];
		itos(*arr, data, 10);
		SHA256((const uchar*)data, strlen(data), node->hash_str);
		delete[] data;

		if (mt == nullptr)//Ϊ�գ���ʱû��������Ҫ�ȴ���һ��ͷ���ڵ�
		{
			mt = new MerkleTreeNode(1);
			mt->left = node;//�µ�������Ҷ�ӽڵ�һ����������
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


inline void mergePeer(MerkleTreeNode* mt, uchar* dst, uchar* hs)//Ѱ���ֵܽڵ㲢����ϲ�hashֵ
{
	uchar h1[32], h2[32];
	uchar msg[65];
	msg[0] = 1;
	MerkleTreeNode* p = mt->parent, * tmp;
	if (!p)return;//û���ϼ��ڵ㣬˵���������û���ֵܽڵ�

	if (p->right == mt)//������������һ����������
	{
		tmp = p->left;
		FOR(i, 0, 32)
		{
			msg[i + 1] = tmp->hash_str[i];
			msg[i + 33] = hs[i];
		}
	}
	else//��������
	{
		if (p->right)//��������
		{
			tmp = p->right;
			FOR(i, 0, 32)
			{
				msg[i + 1] = hs[i];
				msg[i + 33] = tmp->hash_str[i];
			}
		}
		else //û���������������������ڼ���hash
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


inline int inProve(const char* data, int id)//֤������data�����У����ڵ�id�ŵ���
{
	MerkleTreeNode* p = leaves[id];//������ң���ǰ��������Ҷ�ӽڵ��λ��
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

