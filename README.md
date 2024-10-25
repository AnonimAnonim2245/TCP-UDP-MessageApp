# **TCP-UDP Messaging Application**
**Network Protococol Assignment, Luca Plian - 322CA**

A client-server application using **TCP** and **UDP** protocols for real-time messaging and topic-based subscriptions. This allows clients to connect to a server, subscribe to topics, and receive messages based on their subscriptions.

## **Commands**
- `subscribe <topic>` - Subscribes to a topic.
- `unsubscribe <topic>` - Unsubscribes from a topic.
- `exit` - Disconnects from the server.

## **Client Features**
1. **Topic-Based Subscription Management**
   - Clients can **subscribe** and **unsubscribe** to various topics in real-time.
   - Uses **poll** for asynchronous handling, supporting real-time updates.

2. **Data Handling**
   - Processes multiple data types (integer, short real, float, and string) to format topic-relevant messages.

## **Server Features**
1. **Multiplexed I/O with TCP and UDP**
   - Manages multiple clients with **poll** and handles real-time topic-based communication.
   - Processes both **TCP** and **UDP** messages, sending relevant data to subscribers.

2. **Wildcard Matching for Topics**
   - Matches client subscriptions to topics using **exact matches** or **wildcard rules**, ensuring messages are sent only once per topic, even for multiple subscribers.

## **Dependencies**
- **C++ Standard Libraries**
- **Networking Libraries** (TCP/UDP)
