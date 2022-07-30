#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib,"libssl.lib")
#pragma comment(lib,"libcrypto.lib")

#include<cstdio>
#include <iomanip>
#include<iostream>
#include <fstream> 
#include<string>
#include <sstream>
#include<cassert>
#include<algorithm>
#include<openssl/bio.h>
#include<openssl/evp.h>
#include <openssl/pem.h>
#include<openssl/crypto.h>
#include<openssl/ec.h>
#include<openssl/ossl_typ.h>
#include<openssl/bn.h>
#include<openssl/rand.h>
#include<openssl/opensslconf.h>
#include<openssl/obj_mac.h>
#include<openssl/err.h>
#include<openssl/opensslconf.h>
#include<openssl/sha.h>

using namespace std;

const char* P = "FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF";
const char* A = "FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFC";
const char* B = "28E9FA9E9D9F5E344D5A9E4BCF6509A7F39789F515AB8F92DDBCBD414D940E93";
const char* GX = "32c4ae2c1f1981195f9904466a39c9948fe30bbff2660be1715a4589334c74c7";
const char* GY = "bc3736a2f4f6779c59bdcee36b692153d0a9877cc62a474002df32e52139f0a0";
const char* N = "FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFF7203DF6B21C6052B53BBF40939D54123";

class SM2RingSign
{
public:
	int SIZE,ID;// 签名集合个数、用户ID
	#pragma warning(suppress : 4996)
	EC_GROUP* ecGroup = EC_GROUP_new(EC_GFp_mont_method());// 椭圆曲线群E
	BN_CTX* ctx = BN_CTX_new();
	EC_POINT* G = EC_POINT_new(ecGroup);// 基点
	BIGNUM* order = BN_new();
	BIGNUM** S = NULL;
	string* PK = NULL;// 公钥集合
	string pk, sk;// 节点公私钥
	int index = -1;
	
	SM2RingSign(int size,int id) {
		this->SIZE = size;
		this->ID = id;
		this->S = new BIGNUM * [SIZE];
		this->PK = new string[SIZE];
	}

	void init();
	void setKey();
	void readPK();
	string getOrder();
	void genS();
	EC_POINT* getPoint(BIGNUM* x, BIGNUM* y);
	string point2hex(EC_POINT* pt);
	BIGNUM* H(string* PK, string msg, string Z);
	BIGNUM* culc_s(BIGNUM* k, BIGNUM* sk, BIGNUM* Ci);
	string culc_z(string pk, BIGNUM* Ci, BIGNUM* Si);
	BIGNUM** Sign(string msg);
	bool Verify(string msg, BIGNUM** sign);
};

inline string str2hex(string s, int len)
{
	char* ret = new char[len * 2 + 1];
	memset(ret, 0, len * 2 + 1);
	for (int i = 0; i < len;i++)
		sprintf(ret + i * 2, "%02x", s[i]);
	return string(ret);
}


inline int str2int(char c[], int len)
{
	int temp = 0;
	for (int i = 0; i < len; i++)
		temp = temp * 10 + c[i] - '0';
	return temp;
}
/*
* argument[1] RingSize:num of PK
* argument[2] Id: identification
* argument[3] Mode: Sign->0 Vertify->1
* argument[4] Msg
* argument[5] Signature
*/
int main(int argc, char* argv[])
{
	int size = str2int(argv[1], strlen(argv[1]));
	int id = str2int(argv[2], strlen(argv[2]));
	SM2RingSign t(size,id);
	int mode = *argv[3] - '0';
	string msg = argv[4];

	t.init();
	t.setKey();
	t.readPK();

	if (mode == 0)
	{
		t.genS();
		BIGNUM** res = t.Sign(msg);
		printf("Sign:");
		for (int i = 0; i < t.SIZE + 1; i++)printf("%s ", BN_bn2hex(res[i]));
		/*for (int i = 0; i<t.SIZE + 1; i++)
			BN_free(res[i]);*/
	}

	else if (mode == 1)
	{
		BIGNUM** SignedMsg = new BIGNUM * [t.SIZE + 1];
		for (int i = 0; i < t.SIZE + 1; i++)
		{
			SignedMsg[i] = BN_new();
			BN_hex2bn(&SignedMsg[i], argv[5 + i]);
			//printf("%s ", BN_bn2hex(SignedMsg[i]));
		}
		printf("Verify:%d", t.Verify(msg, SignedMsg));
	}

	else printf("Wrong mode!\n");

	return 0;
}

inline void SM2RingSign::init()
{
	BIGNUM* p = BN_new(), * a = BN_new(), * b = BN_new(), * Gx = BN_new(), * Gy = BN_new(), * h = BN_new();
	BN_hex2bn(&p, P);
	BN_hex2bn(&a, A);
	BN_hex2bn(&b, B);
	BN_hex2bn(&Gx, GX);
	BN_hex2bn(&Gy, GY);
	BN_hex2bn(&order, N);
	BN_set_word(h, 1);

	EC_GROUP_set_curve(ecGroup, p, a, b, ctx);

	int ret = EC_POINT_set_affine_coordinates(ecGroup, G, Gx, Gy, ctx);

	EC_GROUP_set_generator(ecGroup, G, order, h);

	BN_free(p); BN_free(a); BN_free(b); BN_free(Gx); BN_free(Gy); BN_free(h);
}

inline void SM2RingSign::setKey()
{
	//id = 0,then it's key file is "key0.txt"
	//the file have two line: first line is sk, and the next is pk
	stringstream fileNameStream;
	fileNameStream << ".\\keys\\key" << this->ID << ".txt";
	string file = fileNameStream.str();
	ifstream keystream(file);
	if (!keystream.is_open())
	{
		cout << "file is not open" << endl;
	}
	getline(keystream, this->sk);
	getline(keystream, this->pk);
	keystream.close();
}

inline void SM2RingSign::readPK()
{
	ifstream PKstream(".\\keys\\PKs.txt");
	assert(PKstream.is_open());
	for (int i = 0; i < SIZE; i++)
	{
		getline(PKstream, PK[i]);
	}
	sort(this->PK, this->PK + SIZE);//Attention
	for (int i = 0; i < SIZE;i++)
		if (pk.compare(PK[i]) == 0) index = i;
	PKstream.close();
}

inline string SM2RingSign::getOrder()
{
	BIGNUM* Order = BN_new();
	EC_GROUP_get_order(ecGroup, Order, ctx);
	return string(BN_bn2hex(Order));
}

inline EC_POINT* SM2RingSign::getPoint(BIGNUM* x, BIGNUM* y)
{
	EC_POINT* point = EC_POINT_new(ecGroup);
	EC_POINT_set_affine_coordinates(ecGroup, point, x, y, ctx);
	return point;
}

inline string SM2RingSign::point2hex(EC_POINT* pt)
{
	BIGNUM* x = BN_new(), * y = BN_new();
	EC_POINT_get_affine_coordinates(ecGroup, pt, x, y, ctx);
	stringstream hex;
	string x_str = BN_bn2hex(x);
	string y_str = BN_bn2hex(y);
	BN_free(x); BN_free(y);
	hex << setw(64) << setfill('0') << x_str;
	hex << setw(64) << setfill('0') << y_str;
	return hex.str();
}

inline void SM2RingSign::genS()
{
	int ret = -1;
	BIGNUM* rand = BN_new(), * temp = BN_new();

	char* res = NULL;
	for(int i= 0;i<SIZE;i++)
	{
		while (1)
		{
			ret = BN_rand_range(rand, order);
			ret = BN_gcd(temp, rand, order, ctx);
			if (BN_is_one(temp))break;
		}
		S[i] = BN_dup(rand);
	}
	BN_free(rand);
	BN_free(temp);
}

inline BIGNUM* SM2RingSign::H(string* PK, string msg, string Z)
{
	string hex_msg = str2hex(msg, msg.length());
	string dst,data;
	for (int i = 0; i < SIZE;i++)
		data+=PK[i];
	data += hex_msg;
	data += Z;

	unsigned char md[33];
	memset(md, 0, 33);
	SHA256((const unsigned char*)data.c_str(), data.length(), md);
	for (int i = 0; i < 32; i++)
	{
		dst+=(char)md[i];
	}
	dst = str2hex(dst,32);
	BIGNUM* hash = BN_new();
	BN_hex2bn(&hash, dst.c_str());
	int l1 = dst.length(), l2 = data.length();
	BN_mod(hash, hash, order, ctx);

	return hash;
}

inline BIGNUM* SM2RingSign::culc_s(BIGNUM* k, BIGNUM* sk, BIGNUM* Ci)
{
	BIGNUM* invert = BN_new();
	BIGNUM* temp = BN_new();
	BIGNUM* One = BN_new();

	BN_set_word(One, 1);
	BN_add(temp, sk, One);//temp=sk+1

	BN_mod_inverse(invert, temp, order, ctx);//invert = inv_mod(1 + sk, q)
	BN_mul(temp, Ci, sk, ctx);//temp=Ci*sk
	BN_sub(temp, k, temp);//temp = k - Ci * sk
	BN_mod_mul(temp, invert, temp, order, ctx);

	BN_free(invert);
	BN_free(One);
	return temp;
}


inline string SM2RingSign::culc_z(string pki, BIGNUM* Ci, BIGNUM* Si)
{
	EC_POINT* sg = EC_POINT_new(ecGroup);
	auto a = this->G;
	auto b = Si;
	auto c = ctx;
	EC_POINT_mul(ecGroup, sg, NULL, this->G, Si, ctx);//sg = Si * G

	BIGNUM* sc = BN_new();
	BN_add(sc, Si, Ci);//sc = (Si + Ci)

	char x[65], y[65];
	for(int i = 0;i < 64;i++)
	{
		x[i] = pki[i];
		y[i] = pki[i + 64];
	}
	x[64] = y[64] = 0;
	BIGNUM* px = BN_new(), * py = BN_new();
	BN_hex2bn(&px, x); BN_hex2bn(&py, y);
	EC_POINT* pt = getPoint(px, py);

	EC_POINT_mul(ecGroup, pt, NULL, pt, sc, ctx);//pt = sc * point
	EC_POINT_add(ecGroup, pt, sg, pt, ctx);//pt = sg + sc * point
	string hex = point2hex(pt);
	BN_free(px); BN_free(py); EC_POINT_free(sg); BN_free(sc); EC_POINT_free(pt);

	return hex;
}


inline BIGNUM** SM2RingSign::Sign(string msg)
{
	int retval = 0;
	BIGNUM** C = new BIGNUM * [SIZE];
	for(int i = 0;i < SIZE;i++)
		C[i] = BN_new();
	int index = 0;
	for (int i = 0; i < SIZE; i++) {
		if (this->PK[i] == this->pk)
			index = i;
	}

	BIGNUM* k = BN_new(), * temp = BN_new();
	BN_rand_range(k, order);
	char* res = NULL;
	while (1)
	{
		BN_rand_range(k, order);
		BN_gcd(temp, k, order, ctx);
		if (BN_is_one(temp))break;
	}
	EC_POINT* kg = EC_POINT_new(ecGroup);
	retval = EC_POINT_mul(ecGroup, kg, NULL, G, k, ctx);

	BIGNUM* bn = H(this->PK, msg, point2hex(kg));
	C[(index + 1) % SIZE] = bn;
	for (int i = (index + 1)%SIZE; i != index; i = (i + 1) % SIZE)
	{
		string Zi = culc_z(this->PK[i], C[i], S[i]);
		C[(i + 1)%SIZE] = H(this->PK, msg, Zi);
	}
	BIGNUM* Sk = BN_new();
	BN_hex2bn(&Sk, sk.c_str());
	S[index] = culc_s(k, Sk, C[index]);
	BIGNUM** ret = new BIGNUM * [SIZE + 1];
	ret[0] = BN_dup(C[0]);
	for(int i = 0;i < SIZE;i++)
		ret[i + 1] = BN_dup(S[i]);

	BN_free(k); BN_free(temp); BN_free(Sk); EC_POINT_free(kg);
	for (int i = 0; i < SIZE; i++)BN_free(C[i]);
	delete[]C;
	return ret;
}


inline bool SM2RingSign::Verify(string msg, BIGNUM** sign)
{
	BIGNUM** C_ = new BIGNUM * [SIZE + 1];
	for(int i = 0; i < SIZE + 1; i++)C_[i] = BN_new();
	C_[0] = sign[0];
	for (int i = 0; i < SIZE;i++)
		BN_set_word(C_[i + 1], 0);
	BIGNUM** S_ = new BIGNUM * [SIZE];
	for (int i = 0; i < SIZE; i++) {
		S_[i] = sign[i + 1];
	}
	BIGNUM* temp = BN_new();
	char* res;
	for(int i = 0; i < SIZE;i++)
	{
		BN_gcd(temp, C_[i], order, ctx);
		res = BN_bn2dec(temp);
		if (strlen(res) != 1 || res[0] != '1')
			return false;
		string Zi = culc_z(PK[i], C_[i], S_[i]);
		C_[i + 1] = H(PK, msg, Zi);
		//printf("Ci : %d ,%s \n", (i + 1) , BN_bn2hex(C_[(i + 1)]));
	}
	if (BN_cmp(C_[0], C_[SIZE]))
	{
		for (int i = 0; i < SIZE; i++){
			BN_free(C_[i]);
			BN_free(S_[i]);
		}
		delete[]C_;
		return false;
	}
	for (int i = 0; i < SIZE; i++) {
		BN_free(C_[i]);
		BN_free(S_[i]);
	}
	delete[]C_;
	return true;
}