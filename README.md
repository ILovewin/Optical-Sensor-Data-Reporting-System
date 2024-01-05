# Optical-Sensor-Data-Reporting-System
两块STM32L475的板子结合两个Lora模块定向传输完成环境数据采集、传输和显示，并且通过ESP32接入阿里云物联网平台上报采集的传感器数据以及实现远程点灯控制。（此仓库只包含esp32模块代码）



## STM32L475结合Lora实现定向传输

### 实验步骤

- 实现两块stm32的串口2通信调试
- 通过串口2实现与lora的通信，并通过AT指令完成定向传输这一通信模式的配置
- 接线，通过LCD屏或串口监视工具查看定向传输结果

### 实验细节

- 串口发送接收数据的缓冲区需要注意清空；
- 由于例程的串口接收数据机制是，检测到 ’ \r\n’ 即视为接收成功，但是并没有把’\r\n’存入对应串口接收的缓存区，所以当一个数据要经过多次串口时，需在串口接收数据结束后，手动给缓冲区加上’ \r\n’，不然，下一个串口接收数据会失败；
- 配置Lora模块时需使用ATE0关掉回显。假如不关闭回显（发AT，Lora模块会返回AT+回车换行+ok+回车换行），串口的接收处理在读到第一个’ \r\n’就截断数据，视为成功接收，这样会造成数据的缺失；
- Lora模块的配置除了配置工作模式（一般模式，唤醒模式…）外，还要配置发送状态（透明传输，定向传输…）；
- 光照传感器例程采集的数据类型的和发送接收的数据类型需要注意进行转换。



## 阿里云物联网平台配置

[参考博客](https://blog.csdn.net/qq_26070183/article/details/128656156?ops_request_misc=&request_id=&biz_id=102&utm_term=esp32接入阿里云物联网&utm_medium=distribute.pc_search_result.none-task-blog-2~all~sobaiduweb~default-7-128656156.142^v96^pc_search_result_base5&spm=1018.2226.3001.4187)

[参考视频](https://www.bilibili.com/video/BV1Xz4y1n7Uh?p=2&vd_source=ba9c88e8d1ac2d744eceecae960ce225)



## esp32开发板环境配置 

- 安装CP2102 USB to UART Bridge Controller 驱动（[参考博客](https://blog.csdn.net/qq_34935373/article/details/107228053)）

- Arduion IDE配置开发板环境（参考博客，需要注意的是博客里的附加开发板管理器网址的ESP32的网址有问题，应改为：https://www.arduino.cn/package_esp32_index.json）




## 硬件设备与云端的通信

1. 复制在阿里云物联网平台创建的设备证书，使用MQTT协议的password计算器生成设备签名；
2. 使用设备签名配置mqtt.fx，验证与阿里云物联网平台设备连接是否正常
3. 复用前面Lora定向传输的接收端stm32串口3与esp32进行数据传输
4. 编写esp32模块代码，连接WiFi网络、连接MQTT服务器、订阅主题、发布信息和处理串口数据等（完整代码已上传）

提示：esp32处理串口数据时，在中断服务例程里进行数据处理可能会导致无法及时喂狗而触发重启，因此建议在主循环里面进行处理。但是，需要注意stm32发送数据的频率是否能让ESP32及时接收并发送数据，否则可能会导致数据的丢失和混乱。