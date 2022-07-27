# 网络空间安全创新创业实践项目实验

所有项目均为本人独立完成。

##### 所做项目

1. **Project: implement the naïve birthday attack of reduced SM3**

对SM3哈希算法尝试进行生日攻击寻找碰撞。考虑到算力限制，截取hash值的前32位作为结果。前32位相同的视为找到一组碰撞。使用python代码实现并成功找到一组碰撞。

2. **Project: implement the Rho method of reduced SM3**

使用低存储的$\rho$方法寻找碰撞，解释性语言的运行速度慢，在可接受的时间内暂时只能找到16位的碰撞。

3. **Project: implement length extension attack for SM3, SHA256, etc.**

对SM3进行长度扩展攻击。

4. **Project: do your best to optimize SM3 implementation (software)**

使用宏定义、SIMD等方法优化SM3。

5. **Project: Impl Merkle Tree following RFC6962**

实现了Merkle Tree以及其存在性证明。

6. **Project: report on the application of this deduce technique in Ethereum with ECDSA**

对以太坊中使用的ECDSA签名以及其公钥推导总结出一份报告。

7. **Project: impl sm2 with RFC6979**

根据RFC6979实现了对k的生成，在此基础上实现SM2签名算法。

8. **Project: Implement a PGP scheme with SM2**

实现了基础的PGP模块，包括SM2密钥交换生成会话密钥、SM2加密会话密钥、根据会话密钥使用SM4对称加密会话数据三个部分。

9. **Project: implement sm2 2P sign with real network communication**

使用socket网络编程，在真实的网络环境下实现了SM2两方签名，并检查了验证签名的正确性。

10. **Project: implement sm2 2P decrypt with real network communication**

同样使用socket，实现了数据的两方加密和解密，并验证了加解密正确性。

11. **Project: forge a signature to pretend that you are Satoshi**

在不验证消息明文m的情况下，根据已有的正确签名伪造了新的签名，并正确通过验证。

12. **Project: Find a key with hash value “sdu_cst_20220610” under a message composed of your name followed by your student ID. For example, “San Zhan 202000460001**

针对meow_hash库，利用逆向工程原理，完整地将hash计算过程完全逆回初始状态，从而实现了根据消息明文和目标hash值计算出一组合适的密钥。并且使用所得密钥尝试进行hash，所得结果和目标值完全一致。

13. **Project: verify the above pitfalls with proof-of-concept code**

编写测试代码，验证各种ECC签名算法的漏洞。

14. **Project: Implement the above ECMH scheme**

实现了将集合hash到椭圆曲线点的ECMH方案，并验证了其同态性质。

15. **Project: PoC impl of the scheme, or do implement analysis by Google**

在真实网络交互的场景下实现了基于argon2的口令检查方案。

##### 存在问题的项目

11. **Project: forge a signature to pretend that you are Satoshi**

由于并未搜索到明确属于中本聪本人的公钥，这里选择随机生成了一对公私钥，但伪造的原理依然适用。

12. **Project: Find a key with hash value “sdu_cst_20220610” under a message composed of your name followed by your student ID. For example, “San Zhan 202000460001**

本项目基于使用AES-NI指令集的meow_hash库，其中的一轮解密模块对应的逆过程并不是使用同一个密钥进行一轮加密。搜索到的办法是先将密钥通过加解密组合的方式进行列混合得到等价密钥，再进行加密，但多次尝试后实验结果仍不正确。**也就是说，暂时没有解决AES-NI指令集一轮解密过程的逆向。**

为了进一步推进项目，人为修改了原库函数的过程。将AES一轮加解密的过程替换成了简单的异或。这样做的影响仅在于安全方面的特性，而本项目的核心思想没有改变。因为AES本身可逆，在本质上可以替换成任意一种可逆运算，这里的替换只是由于暂未解决加解密一致的问题。

除了这一个部分，其它的过程都与原库函数过程完全一致，实现了整体逻辑和过程的逆向，并最终得到了正确的密钥。

###### 更新日志

22-05-15：提交项目

**Project: implement length extension attack for SM3, SHA256, etc.**。

22-07-16：提交项目

**Project: implement the Rho method of reduced SM3**

**Project: implement the naïve birthday attack of reduced SM3**。

22-07-17：提交项目

**Project: Impl Merkle Tree following RFC6962**

22-07-18：提交项目

**Project: Find a key with hash value “sdu_cst_20220610” under a message composed of your name followed by your student ID. For example, “San Zhan 202000460001**

22-07-19：提交项目

**Project: impl sm2 with RFC6979**

22-07-20：提交项目

**Project: do your best to optimize SM3 implementation (software)**

22-07-21：提交项目

**Project: Implement a PGP scheme with SM2**

22-07-22：提交项目

**Project: implement sm2 2P sign with real network communication**

**Project: implement sm2 2P decrypt with real network communication**

22-07-25：

提交项目

**Project: verify the above pitfalls with proof-of-concept code**

**Project: Implement the above ECMH scheme**

更新了部分项目的README。

更新了椭圆曲线相关库函数。

根据代码运行环境需要，上传了OpenSSL库。

22-07-26：

更新了所有项目中涉及到的的hash函数，考虑到安全性，将md5等不再安全的算法更换成SHA256。

22-07-27：提交项目

**Project: PoC impl of the scheme, or do implement analysis by Google**

