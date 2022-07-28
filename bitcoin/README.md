## Project: send a tx on Bitcoin testnet, and parse the tx data down to every bit, better write script yourself

### 项目说明

本项目实现的工作如下：

- 在bitcoin testnet上创建钱包。
- 申请一定数量的bitcoin并自己在测试网上提交一笔交易。
- 编写脚本对提取的交易的原始数据进行解析。

并最终对于以上工作编辑了详细的报告。

下面将分别介绍几个重要的部分。

#### 发布交易

首先介绍如何申请比特币并在testnet上发布交易。

第一，我们需要一个比特币地址，用于申请测试用币。

进入网站www.bitaddress.org。

<img src="./image-20220728204219824.png" alt="image-20220728204559278" style="zoom:50%;" />

该网站可以通过键盘输入随机字符或者读取鼠标运动轨迹生成一个地址及其对应的私钥。当进度达到100%时可以得到结果，如同上图。

其中，左边的字符串是创建的地址，右边是私钥。真实的情况下私钥应当秘密保存，绝不可泄露。由于本项目只是进行测试，故不做保密。有了这个地址后，我们就可以在网络上申请一些测试用的比特币到这个地址中。如果我们需要使用这些比特币，则应使用对应的私钥提取出来。

------

下面介绍一个发放测试用币的网站https://bitcoinfaucet.uo1.net。

<img src="./tx.png" alt="image-20220728205105758" style="zoom:50%;" />

在此，一个IP可以申请到至多0.00072个可在testnet中使用的比特币。这里我们输入刚刚生成的地址，点击申请获得0.0001个比特币。可以看到下面马上出现了一条交易信息，恰好对应了我们的地址。到此为止，我们已经成功地取得了一些可用的比特币。

------

进入https://live.blockcypher.com/btc-testnet/可以查看testnet上进行的所有交易。在搜索框中输入我们的地址可以看到下面的信息：

<img src="./account.png" alt="image-20220728205129035" style="zoom:50%;" />

点击查看交易详情

<img src="C:\Users\Karloz\Desktop\Innovation-and-Entrepreneurship-Practice-Projects\bitcoin\image-20220728230905530.png" alt="image-20220728230905530" style="zoom: 50%;" />

进一步地，通过API Call可以查看详细的交易JSON数据，提取出这些数据。

```json
{
  "block_height": -1,
  "block_index": -1,
  "hash": "068b5110dc0661892e1b64b372ad1e7291201b0bcaecc0b71d620997882ff44e",
  "hex": "020000000001010e414fb48a5039d9dfdc27a67a25716ae6907ce3902d8598bf97d6143c65ff150000000000feffffff02ce8de7240000000016001431327d6496b79d6e43b5c996171244e0a63145ee10270000000000001976a91413cb0cd3484ead63a6753ed255a120f39771224f88ac0247304402202d31d21e3dbfc66cce19d4d7e517cc23ca0d045d543e66b67e3518652c80506c02200d9977febffea9c39404d4d41dd9f6caca086957839a8131f806e1347baa1e2101210223b29c07fcb671c4fbdaf106c872149d54c3f61632728ce1c4ea6fe1ab254c3b76432300",
  "addresses": [
    "mhKcNsErco1UscdWbWxYYkgXoX2noQw4NP",
    "tb1qqdccjzph45vhm5gahxqmhahzzvspl9l9gmz2pc",
    "tb1qxye86eykk7wkusa4extpwyjyuznrz30wghy3g9"
  ],
  "total": 619164894,
  "fees": 145,
  "size": 225,
  "vsize": 144,
  "preference": "low",
  "relayed_by": "54.84.85.20:18333",
  "received": "2022-07-28T12:50:54.7Z",
  "ver": 2,
  "lock_time": 2311030,
  "double_spend": false,
  "vin_sz": 1,
  "vout_sz": 2,
  "confirmations": 0,
  "inputs": [
    {
      "prev_hash": "15ff653c14d697bf98852d90e37c90e66a71257aa627dcdfd939508ab44f410e",
      "output_index": 0,
      "output_value": 619165039,
      "sequence": 4294967294,
      "addresses": [
        "tb1qqdccjzph45vhm5gahxqmhahzzvspl9l9gmz2pc"
      ],
      "script_type": "pay-to-witness-pubkey-hash",
      "age": 0,
      "witness": [
        "304402202d31d21e3dbfc66cce19d4d7e517cc23ca0d045d543e66b67e3518652c80506c02200d9977febffea9c39404d4d41dd9f6caca086957839a8131f806e1347baa1e2101",
        "0223b29c07fcb671c4fbdaf106c872149d54c3f61632728ce1c4ea6fe1ab254c3b"
      ]
    }
  ],
  "outputs": [
    {
      "value": 619154894,
      "script": "001431327d6496b79d6e43b5c996171244e0a63145ee",
      "addresses": [
        "tb1qxye86eykk7wkusa4extpwyjyuznrz30wghy3g9"
      ],
      "script_type": "pay-to-witness-pubkey-hash"
    },
    {
      "value": 10000,
      "script": "76a91413cb0cd3484ead63a6753ed255a120f39771224f88ac",
      "addresses": [
        "mhKcNsErco1UscdWbWxYYkgXoX2noQw4NP"
      ],
      "script_type": "pay-to-pubkey-hash"
    }
  ]
}
```

------

下面我们将使用获得的比特币尝试进行交易。

为了实现交易，首先需要拥有一个钱包。

可以在https://block.io/中创建一个testnet账户。当账户创建成功时，其会自动给与一个地址。此时我们可以使用前面记录的私钥将申请的0.0001个比特币提取出来到钱包中。这样事实上就又产生了一笔交易。

<img src="C:\Users\Karloz\Desktop\Innovation-and-Entrepreneurship-Practice-Projects\bitcoin\image-20220728223730072.png" alt="image-20220728231531337" style="zoom:50%;" />

我们就选择这一条交易进行解析。

------

为了解析数据，需要了解区块链交易的数据格式。

详情如下：

![image-20220728225255609](./raw_tx.png)

下面是我们提取出的交易信息。其中，hex字段就是区块链中实际传输的数据，也就是我们解析的目标。

```json
{
  "block_height": -1,
  "block_index": -1,
  "hash": "27fb140935aaea927f38738841a37ff0848686ca251b9297841c0043fa123c9e",
  "hex": "01000000014ef42f889709621db7c0ecca0b1b2091721ead72b3641b2e896106dc10518b06010000006a4730440220276dc0d6822dc84f60d5d68b10a19ea0041b451175b67f94d874206198c08aa802202060755d1061b9a45ebf366a9abbdebd86f3d81e068950e001de73c8b385c461012102dcecddaba2d7ef86cdbad5aa79aecd00593801d99deff79ca796b0dcd136908cffffffff01411e00000000000017a9148901d45c15f2734ce3e5bc1654d362cd316242558700000000",
  "addresses": [
    "2N5jekgYrnVgKyodJMUiPAaj5mqcwbzLzk7",
    "mhKcNsErco1UscdWbWxYYkgXoX2noQw4NP"
  ],
  "total": 7745,
  "fees": 2255,
  "size": 189,
  "vsize": 189,
  "preference": "low",
  "relayed_by": "54.84.85.20:18333",
  "received": "2022-07-28T14:37:24.727Z",
  "ver": 1,
  "double_spend": false,
  "vin_sz": 1,
  "vout_sz": 1,
  "confirmations": 0,
  "inputs": [
    {
      "prev_hash": "068b5110dc0661892e1b64b372ad1e7291201b0bcaecc0b71d620997882ff44e",
      "output_index": 1,
      "script": "4730440220276dc0d6822dc84f60d5d68b10a19ea0041b451175b67f94d874206198c08aa802202060755d1061b9a45ebf366a9abbdebd86f3d81e068950e001de73c8b385c461012102dcecddaba2d7ef86cdbad5aa79aecd00593801d99deff79ca796b0dcd136908c",
      "output_value": 10000,
      "sequence": 4294967295,
      "addresses": [
        "mhKcNsErco1UscdWbWxYYkgXoX2noQw4NP"
      ],
      "script_type": "pay-to-pubkey-hash",
      "age": 0
    }
  ],
  "outputs": [
    {
      "value": 7745,
      "script": "a9148901d45c15f2734ce3e5bc1654d362cd3162425587",
      "addresses": [
        "2N5jekgYrnVgKyodJMUiPAaj5mqcwbzLzk7"
      ],
      "script_type": "pay-to-script-hash"
    }
  ]
}
```

