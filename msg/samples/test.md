## 正确性测试
1. svr, cli正常
2. cli不断发送消息，每发一条打印一次进度；svr暂不recv，先sleep一段时间后再recv。
3. svr中途kill后立刻重启，cli则持续不断发送消息（每隔一秒一条，发送一百条）。
4. svr先recv再send_async，send_async后立刻退出程序，看看cli接收情况。

## 性能测试
1. latency：无payload，测试一次send，recv耗时。参考nng的perf。
2. throughput：cli发送固定大小大数据量消息，svr持续接收，一段时间后统计每秒多少MB。参考nng的perf。
3. concurrency：正常运行的并发数，最大支持的并发数。

## 


