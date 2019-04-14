# Transport Layer

The transport layer encapsulates sockaddr, asynchronous connect/listen/scatter-gather IO, and its user interface: tasks. 

You should submit a task(listen_task, connect_task, read_task or write_task) to transport objects and let them handle it. You need to implement subclasses of these tasks and override on_success and on_failure callbacks. 





