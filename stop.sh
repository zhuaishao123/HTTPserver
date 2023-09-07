#!/bin/bash

# 查找并终止名为 "server" 的进程
pkill -f "httpserver"
echo "服务器已停止运行"
