# PennCloud

Final Project - CIS 505: Software Systems, University of Pennsylvania. Spring 2018

# Information

This repository only contains the binaries, in compliance with the course policies that prevent sharing and publishing code as per the academic integrity requirements.

The private repo, [CIS505Project](https://github.com/TheGarvitGupta/CIS505Project) contains the entire code. Please [write to me](garvit@cis.upenn.edu) if you'd like access to the code or instantiate the demo on AWS link below (It is highly likely that node would have crashed, otherwise)


* AWS Link: http://ec2-18-217-241-213.us-east-2.compute.amazonaws.com:8000
* Binaries: https://github.com/TheGarvitGupta/PennCloud
* Code: https://github.com/TheGarvitGupta/CIS505Project

# Description

PennCloud is a distributed cloud platform with a highly scalable, fault-tolerant key-value datastore at the backend that supports strong consistency and efficient replication, checkpointing and recovery.

# Instructions

## Local Execution

Clone the repository
```sh
git clone https://github.com/TheGarvitGupta/PennCloud.git
```

The following commands can be copied as a block and executed on the terminal. This will start three `front end servers` (html-server), `master storage` (master-storage), `heartbeat` (heartbeat), `SMTP server` (smtp+) and nine instances of `backend servers` (quorum-server) in three groups: 500X, 600X, 700X

```sh
cd PennCloud/bin
mkdir ckpts
mkdir Drive
mkdir logs

./html-server config.md -p 8000 &
./html-server config.md -p 8001 &
./html-server config.md -p 8002 &
./master-storage &
./heartbeat config.md &
./smtp+ config.md -v &
./quorum-server 5000 &
./quorum-server 5001 &
./quorum-server 5002 &
./quorum-server 6000 &
./quorum-server 6001 &
./quorum-server 6002 &
./quorum-server 7000 &
./quorum-server 7001 &
./quorum-server 7002 &
```

# Team Members
* [Garvit Gupta](https://www.linkedin.com/in/garvitgupta)
* [Anant Maheshwari](https://github.com/anantm95)
* [Sahana Vijaya Prasad](https://www.linkedin.com/in/sahana-vijaya-prasad)
* [Shiva Suri](https://github.com/shivasuri)

# Screenshots

![Drive](https://github.com/TheGarvitGupta/PennCloud/blob/master/Images/Screen%20Shot%202018-06-17%20at%204.38.26%20PM.png "Drive")
Users can upload files, create folders, rename, delete, or move the items to different directories. All the files are replicated on `W` quorum servers at the backend.

![Admin](https://github.com/TheGarvitGupta/PennCloud/blob/master/Images/Screen%20Shot%202018-06-17%20at%204.34.34%20PM.png "Admin")
The admin console lists all the active and inactive back-end servers, with an option to start or stop them.

![Mail](https://github.com/TheGarvitGupta/PennCloud/blob/master/Images/Screen%20Shot%202018-06-17%20at%204.35.09%20PM.png "Mail")
Users can receive emails and send emails to local (@PennCloud) users and external users, which are relayed to the appropriate servers by checking the DNS MX records. The email server follows the `SMTP` protocol.

![Game ID](https://github.com/TheGarvitGupta/PennCloud/blob/master/Images/Screen%20Shot%202018-06-17%20at%204.35.22%20PM.png "Game ID")
Allows players to create tic-tac-toe game and share the unique generated ID with friends to join the game.

![Game Play](https://github.com/TheGarvitGupta/PennCloud/blob/master/Images/Screen%20Shot%202018-06-17%20at%204.36.49%20PM.png "Game Play")
Supports multiple users to play concurrently with two people joining one game.

## Components and system design

* Load balancer
* Front-end Server: HTTP Server, Quorum Client
* SMTP Mail Server
* Master Node
* Quorum group constituting of Storage Nodes
* Heartbeat Server
* Admin Console

<p align="center">
  <img src="https://github.com/TheGarvitGupta/PennCloud/blob/master/Images/Untitled-1.png" alt="System Design"/>
  <p align="center">System Design</p>
</p>
