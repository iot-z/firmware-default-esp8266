# UDPZ
Specification of the "protocol".
The conversation is based on a event pattern. Will be sent a topic and params, and trigged in destination as an event.

## Configuration
- Max packet size: `512B`
- Client timeout: `250ms without new packet`
- Ping request frequency: `50ms from packet received`

## Client
- Request connect topic: `+`
- Request disconnect topic: `-`
- Response ping topic: `.`

## Server
- Response connect topic: `+`
- Request ping topic: `.`

## Messages
The pattern of messages is: `topic:params`

The demilitter beetwen `topic` and `params` is the `:` character

Params can be passed separeted with a `|` (pipe), eg: `topic:param1|param2|paramN`  
Note: Max 5 params can be handled by Module
Note: All params will be sent as string

It will be received like a event and each param like a argument:

*JavaScript example*

```js
  // Send
  b.send('topic', param1, param2, paramN).then(...);

  // Receive
  b.on('topic', (param1, param2, paramN)) {
    // Callback
  });
```

*C++ example*

```js
  // Send
  Module.send("topic", "param1|param2|paramN");

  // Receive
  Module.on("topic", [&](String* params)) {
    // Callback
  });
```
