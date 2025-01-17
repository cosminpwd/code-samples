# Observer design pattern in a server with a local database
Simple database observer that notifies server clients when a row was updated. It is actual production code from my issue tracker project.

## Details
- `DatabaseObserver.h` - observer base interface and a class that manages subscribed observers.
- `UpdatesTCPConnection.h` - header for a class that connects to a server client and sends a database update.
- `UpdatesTCPConnection.cpp` - implementation for the class that connects to a server client and sends a database update.
- Server updates go through safety checks in `UpdatesTCPConnection.cpp`: user license, connection conflict, connection timeout; integrity checks are also implemented in other classes which are not included in this sample.
