### Project: implement sm2 2P sign with real network communication

#### 代码说明

使用socket网络编程，在真实网络场景下实现两方签名。

算法流程见下图

![image-20220725212518909](./algorithm.png)

分别实现了用户A和B的不同行为以实现交互，具体交互过程和图中完全一致，不再过多解释。

#### 运行结果

最终得到签名值并输出。

![image-20220725212810904](./A.png)

![image-20220725212833991](./B.png)

#### 运行指导

先运行UserB.py，监听端口。

再运行UserA.py即可。