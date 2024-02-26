#!/bin/bash


# 检查是否成功编译
if [ $? -eq 0 ]; then
  # 运行服务器程序并传递端口号参数，将输出重定向到日志文件
  
  stdbuf -o0 ./httpserver 8080 1> run.log 2> error.log &
  # ./httpserver 8080
  echo "服务器已在后台启动，日志保存在 run.log 文件中"
else
  echo "编译失败，无法启动服务器."
fi
