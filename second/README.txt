
异常和处理

CMD_TYPE : 4B  命令的命令部分
CMD_DATA : 28B 命令带有的数据部分


CMD_FILE_UP   文件上传命令,带有文件的大小信息
CMD_ACK		  服务器的应答信号,表示收到  
FILE_DATA	  文件的数据[.....]
 
SERVICE   等待命令 ---> 等待文件 ---> 接收数据 <---> 返回丢包 --> 退出




SERVICE 服务器
CLIENT  客户端  


一. 开始发送阶段

		CMD_FILE
CLIENT -------------> SERVICE [等待命令->文件接收状态]

		CMD_ACK
CLIENT <------------- SERVICE 

		FILE_DATA
CLIENT -------------> SERVICE

可能遇到的错误
1.SERVICE 未收到 CLIENT 的 CMD_FILE,自然SERVICE 不会返回 CMD_ACK
,400ms TX 未收到 CMD_ACK,则重新发送。

2. SERVICE 收到 CMD_FILE,SERVICE 回 CMD_ACK后 ,SERVICE 等待 300ms 未收到接下来的数据,
退回到等待命令。

3. SERVICE 收到 CMD_FILE,SERVICE 回CMD_ACK ,SERVICE 等到400MS 收到 CLIENT 的再次发来 CMD_FILE
而不是发来数据，即 SERVICE 收到2次 CLIENT 的 CMD_FILE ,进入情况2处理。

4. SERVICE 收到,SERVICE 回包比较久,导致 CLIENT 重发,导致 SERVICE 接收到两个包,
而 CLIENT 则收到CMD_ACK,准备开始发数据,进入情况2处理。因为也是准备接收数据。








		FILE_DATA
CLIENT -------------> SERVICE




2. CLIENT 数据持续发送 每次发送 512~1024 字节
 SERVICE 处理接收数据。
 
1.CLIENT 发送失败,导致SERVICE长时间接收不到 连续500MS没有继续受到数据
  则RX返回等待指令给发送方。等待继续指令发送2次未果退出!!

  CLIENT 每次发送完判断接收缓存是等待继续指令 若没有收到则继续。
  
  
数据初步发送完毕 CLIENT 发送获取丢包总数
	获取丢包总数
CLIENT --------------> SERVICE
	返回丢包总数 
CLIENT --------------- SERVICE 
  
CLIENT 在300MS未收到回复重发一次.  
SERVICE 未收到 500MS  则 SERVICE 返回等待指令给发送方。等待继续指令发送2次未果退出!!
SERVICE  收到但返回的 CLIENT 的包丢失。SERVICE 接续回到上面等500MS

























 
 
 
 
 
 