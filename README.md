# 概述

1. 该项目根据LevelDB复刻，用于学习CPP服务开发。
2. 想要支持两种使用模式
   1. 带`main.cpp`的独立进程启动，通过暴露出的API与用户进程进行交互
   2. 作为一个第三方库，被引入到宿主项目中提供`key_value对`的存储引擎的能力
3. 



# NOTE
1. 入口文件 db.hpp datebase_impl.hpp 
