
 # UDPZ
Specification of the "protocol".
The conversation is based on a event pattern. Will be sent a topic and params, and trigged in destination as an event.

## Configuration
- Max packet size: `512B`
- Client timeout: `15s without new packet`
- Ping request frequency: `7s from last packet received`

## Client
- Request connect message:
```json
{
  "topic": "connect",
  "data": {
    "name": "[NAME]",
    "type": "[TYPE]",
    "version": "[VERSION]"
  }
}
```

- Request disconnect message:
```json
{
  "topic": "disconnect"
}
```

- Response ping message:
```json
{
  "topic": "ping"
}
```

## Server
- Response connect topic: `disconnect`
- Request ping topic: `.`

## Messages
Messages are JSON strings with this pattern:

```json
{
  "topic": "topic-name",
  "data": {}
}
```

All messages from the module will have a param named `module`, appended by the lib UDPZ, with the id of the module, eg:
```json
{
  "module": "the-module-id-GUID",
  "topic": "topic-name",
  "data": {}
}
```


*JavaScript example*

```js
  // Send
  b.send('topic', data).then(...);

  // Receive
  b.on('topic', (data) => {
    // Callback
  });
```

*C++ example*

```js
  // Send
  Module.send("topic");
  Module.send("topic", JsonObject& data);

  // Receive
  Module.on("topic", [&](String* params) {
    // Callback
  });
```
