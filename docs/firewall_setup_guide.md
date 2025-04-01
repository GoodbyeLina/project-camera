# Windows 11 防火墙8080端口设置指南

## 方法一：图形界面设置
1. 打开Windows安全中心
   - 点击开始菜单 → 设置 → 隐私和安全性 → Windows安全中心
   - 或直接搜索"Windows安全中心"

2. 进入防火墙设置
   - 点击"防火墙和网络保护"
   - 点击"高级设置"

3. 添加入站规则
   - 右侧点击"入站规则" → 新建规则
   - 选择"端口" → 下一步
   - 选择"TCP"，特定本地端口输入"8080" → 下一步
   - 选择"允许连接" → 下一步
   - 勾选所有网络类型(域/专用/公用) → 下一步
   - 名称输入"ESP32视频接收" → 完成

## 方法二：命令行设置(管理员权限)
```powershell
# 添加8080端口入站规则
New-NetFirewallRule -DisplayName "ESP32视频接收" -Direction Inbound -LocalPort 8080 -Protocol TCP -Action Allow

# 验证规则是否生效
Get-NetFirewallRule -DisplayName "ESP32视频接收" | Select-Object Enabled,Profile,Direction,Action
```

## 验证端口是否开放
1. 在接收程序运行状态下，另开命令提示符执行：
```powershell
Test-NetConnection -ComputerName 127.0.0.1 -Port 8080
```
2. 应该显示"TcpTestSucceeded: True"

## 注意事项
1. 首次运行接收程序时，Windows会弹出防火墙提示，请选择"允许访问"
2. 如果使用公共网络，可能需要在网络设置中将其改为"专用网络"
3. 确保没有其他程序占用8080端口