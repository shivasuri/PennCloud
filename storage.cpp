#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <malloc.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <iostream>
#include <signal.h>
#include <algorithm>

using namespace std;

struct inputs {
	int p;
	int v;

	inputs() {
		p = 0;
		v = 0;
	}
};

vector<pthread_t> threads;
vector<int*> fds;
inputs in;

void shut_down_server(int signal) {

//	Main thread calls SIGTERM on each thread when it receives SIGINT (CTRL + C)

	for (int i = 0; i < threads.size(); i++) {
		if (in.v == 1) {
			fprintf(stderr, "Sending SIGINT to thread: %d\n", (int)threads[i]);
		}
		pthread_kill(threads[i], SIGTERM);
	}

//	By now threads would have started to cleanup and terminate
//	Join all threads

	for (int i = 0; i < threads.size(); i++) {
		if (in.v == 1) {
			fprintf(stderr, "Joining thread: %d\n", (int)threads[i]);
		}
		void *status;
		pthread_join(threads[i], &status);
	}
	if (in.v == 1) {
		fprintf(stderr, "Shutting down server\n");
	}

	exit(0);
}

void shut_down_thread(int signal) {

//	Worker Thread performs shutdown and cleanup on receiving the SIGTERM

	pthread_t thread;
	thread = pthread_self();
	int index = 0;
	while (threads[index] != thread) {
		index++;
	}

	int comm_fd = *fds[index];

	string writer = "-ERR Server shutting down\r\n";
	for (int i = 0; i < writer.length(); i++) {
		char item = writer[i];
		write(comm_fd, &item, sizeof(char));
	}
	close(comm_fd);
	pthread_exit(NULL);
}

int isEnd(string read) {

//	Check if \r\n has been encountered

	if (read.size() < 2) {
		return 0;
	} else if (read[read.size() - 1] == '\n' && read[read.size() - 2] == '\r') {
		return 1;
	} else {
		return 0;
	}
}

int isCommand(string read) {

//	Check if it is a potential command (echo/quit and return the code)

	for (int i = 0; i < read.size(); i++) {
		read[i] = tolower(read[i]);
	}

	if (read.substr(0, 5) == "echo ") {
		return 1;
	} else if (read.substr(0, 4) == "quit") {
		if (read.length() == 6) {
			return -1;
		} else {
			return 0;
		}
	} else {
		return 0;
	}
}

string extractMessage(string read) {

//	Extracts message between ECHO and \r\n
	return read.substr(5, read.length() - 7);
}

void *worker(void *arg) {

//	Event handler when main thread is interrupted, it interrupts all the threads concurrently
	signal(SIGTERM, shut_down_thread);

	int comm_fd = *(int*) arg;

	string buffer;
	string writer;
	string initial = "+OK Server ready (Author: Garvit Gupta / garvit)\r\n";
	char character;
	bool shutdown = false;

	for (int i = 0; i < initial.length(); i++) {
		char element = initial[i];
		write(comm_fd, &element, sizeof(char));
	}

	while (true) {

//    	Check if client wants to terminate connection

		if (shutdown == true) {
			if (in.v == 1) {
				fprintf(stderr, "Terminating connection with the client\n");
			}
			close(comm_fd);
			if (in.v == 1) {
				fprintf(stderr, "[%d] Connection closed\n", comm_fd);
			}
			pthread_exit(NULL);
		}

//		Keep waiting to read something

		if (read(comm_fd, &character, sizeof(char))) {

			buffer.push_back(character);
			string encoded = buffer;

			if (in.v == 1) {

				int pos;

				while ((pos = encoded.find('\r')) != std::string::npos) {
					encoded.replace(pos, 1, "<LR>");
				}
				while ((pos = encoded.find('\n')) != std::string::npos) {
					encoded.replace(pos, 1, "<LN>");
				}

//				fprintf(stderr, "[%d] Server received: %s (%d)\n", comm_fd, encoded.c_str(), buffer.length());
			}
		} else {
			if (in.v == 1) {
				fprintf(stderr, "The client died before saying goodbye\n");
			}
			close(comm_fd);
			if (in.v == 1) {
				fprintf(stderr, "[%d] Connection closed\n", comm_fd);
			}
			pthread_exit(NULL);
		}

//		After reading each character, check if \r\n has been encountered, if that is the case parse input

		if (isEnd(buffer)) {

			if (in.v == 1) {
				fprintf(stderr, "[%d] C: %s\n", comm_fd, buffer.c_str());
			}

			int command = isCommand(buffer);

			if (command == -1) {
				writer = "+OK Goodbye!";
				shutdown = true;
			} else if (command == 1) {
				writer = "+OK " + extractMessage(buffer);
				buffer = "";
			} else {
				writer = "-ERR Unknown command";
			}
		}

//		There is something to send back to the client

		if (writer.length() > 0) {
			writer += "\r\n";
			char * push = new char[writer.length() + 1];
			strcpy(push, writer.c_str());

			if (in.v == 1) {
				fprintf(stderr, "[%d] S: %s\n", comm_fd, writer.c_str());
			}

			write(comm_fd, push, writer.length());

//			Clear both the write and read buffers
			writer.clear();
			buffer.clear();
		}
	}

	return NULL;
}

inputs parseInput(int argc, char *argv[]) {

//	Parses user input and pushes to the structure inputs

	inputs in;
	int c = 0;
	int error = 0;

	while ((c = getopt(argc, argv, ":p:av")) != -1) {

		switch (c) {
		case 'p':
			in.p = atoi(optarg);
			break;

		case 'a':
			fprintf(stderr, "%s\n", "Garvit Gupta");
			fprintf(stderr, "%s\n", "garvit@seas.upenn.edu");
			exit(0);
			break;

		case 'v':
			in.v = 1;
			break;

		case ':':
			fprintf(stderr, "Error: -%c expects an argument\n", optopt);
			error++;
			break;

		case '?':
			fprintf(stderr, "Error: Invalid argument, %c\n", optopt);
			error++;
			break;
		}
	}

	if (in.p == 0) {
		in.p = 10000;
	}

	if (error == 1) {
		printf("Usage: %s -p <portno> [-a][-v]\n", argv[0]);
		exit(1);
	}

	return in;
}

int main(int argc, char *argv[]) {

	in = parseInput(argc, argv);

//	Signal handler when the user presses CTRL + C
	signal(SIGINT, shut_down_server);

//	Open socket and set port reuse
	int enable = 1;
	int listen_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (listen_fd < 0) {
		if (in.v == 1) {
			fprintf(stderr, "Failed to open socket\n");
		}
	} else {
		if (in.v == 1) {
			fprintf(stderr, "Socket opened\n");
		}
	}
	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))
			< 0) {
		if (in.v == 1) {
			fprintf(stderr, "Failed to set socket for reuse\n");
		}
	} else {
		if (in.v == 1) {
			fprintf(stderr, "Socket set for reuse (SO_REUSEADDR)\n");
		}
	}

//    Bind and listen
	struct sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htons(INADDR_ANY );
	servaddr.sin_port = htons(in.p);
	bind(listen_fd, (struct sockaddr*) &servaddr, sizeof(servaddr));
	listen(listen_fd, 10);

	int i = 0;

//    Continue accepting connections - and send to thread for communication
	while (true) {
		struct sockaddr_in clientaddr;
		socklen_t clientaddrlen = sizeof(clientaddr);
		int * fd = (int*) malloc(sizeof(int));
		*fd = accept(listen_fd, (struct sockaddr*) &clientaddr, &clientaddrlen);

		if (in.v == 1) {
//			fprintf(stderr, "[%d] %s\n", fd, "New connection");
			fprintf(stderr, "From: %s\n", inet_ntoa(clientaddr.sin_addr));
		}

		pthread_t thread;
		pthread_create(&thread, NULL, worker, fd);
		threads.push_back(thread);
		fds.push_back(fd);

		if (in.v == 1) {
			fprintf(stderr, "Thread created: %d\n", (int)thread);
		}
	}
}
