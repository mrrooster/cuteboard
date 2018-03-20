# Cuteboard

## Introduction

Cuteboard is a lightweight clipboard history program, written using Qt. It also supports clipboard sharing between machines using a small server.

## Cuteboard

Cuteboard is the client program. It provides a clipboard history feature, allowing you to recall previous clipboard entries.

If you have a cuteboardd server running somewhere you can also share clipboards between different machines. To do this open the options window and enter the connection details for the server.

The username and passphrase should be the same between machines.

## Cuteboardd

Cuteboardd is the server process for the cuteboard protocol, it takes incoming connections from the cuteboard client and distributes the cliboard between them. Clipboards are distributed between the same users.

### The cuteboard protocol

The cuteboard protocol is a very simple line based protocol. Lines are seperated by <CR>-<LF>. Apart from the initial header all lines take the form:
```
<command name>: <command value>
```

#### Stage 1. Initial connection

The client connects and sends the protocol header, currently `CB 1.0`. The server sends no response.

#### Stage 2. Identify

The client sends `User` and `Client` commands. The server responds with a challenge.

##### Commands

`User`: This is the  user name
`Client`: This is a version string to identify the client, currently <client name>/<client version> (<client os and cpu>)

##### Response

`Challenge`: A string that should be used to construct a challenge response.

#### Stage 3. Authenticate

The client sends the 'ChallengeResponse' command. The server will respond with an 'Error' command.

Using the challenge string in the previous stage you should construct a suitable response. This response should be the same across devices. The server stores the response, and on subsiquent connections with the same user will issue the same challenge.

The cuteboard client constructs a response by concatenating the password and the challenge and then performing a sha256 of the result. This hash is then converted to base64 and returned as the response.

The response should be sent with the `ChallengeResponse` command.

#### Stage 4. Connected

If the correct 'ChallengeResponse' was given the server will respond with an `Error` with a value of `0/0 OK`. At this point you are connected.

##### Pinging the server

You should send a `Ping` command to the server every so often. The default client sends a ping once a minute. The server will disconnect you if you don't.

##### Sending a clipboard

To send a clipboard send a `Clipboard` command with the value being the clipbord data to send. This will be transmitted to each connected client with the same username.

##### Receiving a clipboard

The server will send a `Clipboard` command when a remote client sends a clipboard. The value will be the incoming clipboard.

##### Encoding the clipboard

The cuteboard client encodes the cliboard as a series of <nl> seperated values. Each value is a mimetype and it's value, seperated by `=`. The values are base 64 encoded.

This entire string is then prepended withe 'CUTE' and encrypted using the twofish algorithm with the passphrase as a key. The result is then base 64 encoded and used as the payload for the `Clipboard`.

### Example protocol exchange

An example protocol exchange:

```
Client :CB 1.0
Client :User: auser
Client :Client: cuteboard/180101 (Windows 10 (10.0) x86_64)
Server :Challenge: {1234-abcd-2345-cdef}
Client :ChallengeResponse: j94ijfgkja9w034ut90sug9sdujsopfwu0349ruiw==
Server :Error: 0/0 OK
Client :Ping: 
Server :Pong: 
```

```
Client :Clipboard: j3489jegiohat4o09auwo0h8oaw4t89agh9o4gha4o089yuho5hijeghaoi489guy04f89uwa0efas8ufgaowghawouthwuirgaw3==
```