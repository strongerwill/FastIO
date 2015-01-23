A.  
  print_dmesg.c 运行结果 hypercall.log（14M） //收集数据源
  func_process.py 运行结果 result.txt //处理数据源，得到每个进程的function flow，计算出每个function flow在数据源中总的出现次数，并按从大到小排列。
  note: 按照本次的result.txt，各种function flow的总和是：602

B.
  some errors in result.txt

  解决方案：

  把不符合 ?prink(首) ->...-> syscall_call(尾) 模式的function flow 直接过滤掉；


  