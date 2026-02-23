# ft_irc

## 📡 Description

**ft_irc** is a custom implementation of an **Internet Relay Chat (IRC) server** written in **C++ (C++98 standard)**.

The goal of this project is to build a fully functional IRC server capable of handling multiple clients simultaneously using non-blocking I/O and a single multiplexing mechanism (`poll()` or equivalent).

The server allows IRC clients (such as `irssi`, `weechat`, or `nc`) to connect via TCP/IP and interact through channels, private messages, and operator commands — similar to real IRC servers.

This project focuses on:

* Socket programming
* TCP/IP communication
* Event-driven architecture
* Non-blocking file descriptors
* Multiplexing with `poll()` (or equivalent)
* Protocol implementation
* Clean and modular C++ design

---

## 🧠 What is IRC?

**Internet Relay Chat (IRC)** is a text-based communication protocol that enables real-time messaging.

* Users connect to an IRC server using an IRC client.
* They authenticate, choose a nickname, and join channels.
* Messages sent to a channel are broadcast to all connected users.
* Operators can moderate channels with specific commands.

---

## ⚙️ Features (Mandatory Part)

### ✅ Core Requirements

* Multiple client handling (simultaneously)
* Non-blocking I/O
* Single `poll()` (or equivalent) for all operations
* No forking
* TCP/IP communication (IPv4 or IPv6)
* Fully C++98 compliant
* No external libraries

### 👤 Authentication & User Management

* Password authentication
* Nickname setting
* Username registration

### 💬 Messaging

* Private messages between users
* Channel messaging
* Message broadcasting to all channel members

### 🏷 Channels

* Create and join channels
* Channel operators and regular users

### 🛠 Operator Commands

* `KICK` – Remove a user from a channel
* `INVITE` – Invite a user to a channel
* `TOPIC` – Change/view channel topic
* `MODE` – Modify channel modes:

| Mode | Description                         |
| ---- | ----------------------------------- |
| `i`  | Invite-only channel                 |
| `t`  | Restrict topic changes to operators |
| `k`  | Set/remove channel key (password)   |
| `o`  | Grant/remove operator privileges    |
| `l`  | Set/remove user limit               |

---

## 🏗 Technical Constraints

* Must compile with:

  ```bash
  -Wall -Wextra -Werror -std=c++98
  ```
* No Boost or external libraries
* Only allowed system calls as defined in the subject
* Clean, modular, and robust error handling
* Server must **never crash**

---

## 🧪 Compilation & Execution

### 🔨 Build

```bash
make
```

Available Makefile rules:

```bash
make        # Build the project
make clean  # Remove object files
make fclean # Remove objects and executable
make re     # Rebuild everything
```

---

### ▶️ Run the Server

```bash
./ircserv <port> <password>
```

Example:

```bash
./ircserv 6667 mypassword
```

* `port` → Listening port
* `password` → Required password for client connection

---

## 🔌 Connecting with an IRC Client

You can use any IRC client. Example using `nc`:

```bash
nc -C 127.0.0.1 6667
```

Or using `irssi`:

```bash
irssi -c 127.0.0.1 -p 6667
```

You will then need to:

1. Authenticate with the password
2. Set a nickname
3. Set a username
4. Join a channel

---

## 🧪 Testing Partial Commands

Since TCP packets may arrive fragmented, the server correctly reconstructs commands.

Test example:

```bash
nc -C 127.0.0.1 6667
```

Then manually type:

```
com[Ctrl+D]man[Ctrl+D]

```

The server must properly aggregate packets before processing the command.

---

## 🧩 Architecture Overview

The server is built using:

* Event-driven loop with `poll()`
* Non-blocking sockets
* Client abstraction
* Channel abstraction
* Command parsing system
* Message dispatch system

Main components include:

* Server core
* Client manager
* Channel manager
* Command parser
* Mode handler

---

## 📚 Resources

### IRC Protocol Documentation

* RFC 1459 – Internet Relay Chat Protocol
* RFC 2810, 2811, 2812 – IRC Client & Server Protocol

### Socket Programming

* Beej’s Guide to Network Programming
