# MXDS012-2 G474 RTOS

## 外设

|外设|目标|功能|
|-|-|-|
|USART1|STLINK VCP|串口调试输出|
|I2C1|OLED 屏幕|显示系统状态|
|I2C3|INA231|电源监测|
|ADC1 IN1|MAX14808 LVOUT1||
|ADC2 IN3|MAX14808 LVOUT2||
|ADC3 IN1|MAX14808 LVOUT3||
|ADC4 IN5|MAX14808 LVOUT4||
|ADC5 IN1|REF35 1.6V 电压基准||
|USB FS|上位机|与上位机通信|

## 通信协议

当前协议帧格式：

```text
AA 55 | lenL lenH | type | payload... | checksumL checksumH | 0D 0A
```

- `len` 为 payload 长度（小端）
- `checksum` 计算规则：

```text
checksum = lenL XOR lenH XOR type XOR payload[0] XOR ... XOR payload[n-1]
```

当前实现中 `checksum` 为 16 位字段，但实际有效值在低 8 位，因此一般为：

```text
checksumL = checksum
checksumH = 00
```

### 指令类型

上行：

- `0x04` (`USB_PKT_TYPE_NOTIFY_CONFIG`): 设备上报当前配置

下行：

- `0x10` (`USB_PKT_TYPE_SET_CONFIG`): 设置系统参数
- `0x11` (`USB_PKT_TYPE_ACQ_CONFIG`): 查询系统参数

## 仪表放大器增益选择

1. 620Ω: G = 10.67, diff = 50;
2. 330Ω: G = 19.18, diff = 97;
3. 130Ω: G = 47.16, diff = 170;
4. 56Ω: G = 108.14, 触底失真了
5. 91Ω: G = 66.93, diff = 260, 有一点失真;
6. 100Ω: G = 61.0, diff = 240, 就你了。

## 指令测试

### 查询系统参数

```text
请求(GET_CONFIG, payload_len=0):
AA 55 00 00 11 11 00 0D 0A

期望返回(NOTIFY_CONFIG):
AA 55 03 00 04 [freq] [depthL] [depthH] [csL] 00 0D 0A

例如当前配置为 freq=30(0x1E), depth=512(0x0200):
AA 55 03 00 04 1E 00 02 1B 00 0D 0A
```

### 设置系统参数

`SET_CONFIG` 的 payload 固定 3 字节：

```text
payload[0] = scan_frequency_hz
payload[1] = sample_depth 低 8 位
payload[2] = sample_depth 高 8 位
```

示例：设置 `freq=30`, `depth=512`

```text
请求(SET_CONFIG):
AA 55 03 00 10 1E 00 02 0F 00 0D 0A

期望返回(NOTIFY_CONFIG):
AA 55 03 00 04 1E 00 02 1B 00 0D 0A
```

示例：设置 `freq=10`, `depth=400(0x0190)`

```text
请求(SET_CONFIG):
AA 55 03 00 10 0A 90 01 88 00 0D 0A

期望返回(NOTIFY_CONFIG):
AA 55 03 00 04 0A 90 01 9C 00 0D 0A
```

### 异常测试

1. 错误校验（应被丢弃，无响应）

```text
AA 55 03 00 10 1E 00 02 FF 00 0D 0A
```

1. 不支持的下行类型（应被丢弃，无响应）

```text
AA 55 00 00 12 12 00 0D 0A
```

1. `SET_CONFIG` 长度错误（应被丢弃，无响应）

```text
AA 55 02 00 10 1E 00 0C 00 0D 0A
```
