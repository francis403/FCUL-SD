/*
* Grupo 13
* Francisco de Almeida Araujo, 45701
* Nuno Burnay, 46406
* Robin Vassantlal, 46408
*/

#include <error.h>
#include <stdio.h>
#include <signal.h>
#include <poll.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "inet.h"
#include "table-private.h"
#include "message-private.h"
#include "network_client-private.h"
#include "table_skel-private.h"
#include "primary_backup.h"
#include "setup_file.h"

#define N_OF_FDS 5 //servidor + cliente
#define POLL_TIMEOUT 2
#define BK_INDEX 2

struct pollfd fds[1 + N_OF_FDS]; //stdin + server + clients
Server *bk;
const char *file_name = "grupo13.txt";
int primary_server;
const short DEFAULT_UPDATE_PORT = 50000;

pthread_t thread;
pthread_mutex_t mutex_start = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_end = PTHREAD_MUTEX_INITIALIZER;
Message *share_with_thread;
int end_thread;//usar volatil?
int thread_is_on = 0;

void close_fds();
void catchSignal (int signal);
/* Função para preparar uma socket de receção de pedidos de ligação.
*/
int make_server_socket(short port);
void start_thread();
/* Função que recebe uma tabela e uma mensagem de pedido e:
	- aplica a operação na mensagem de pedido na tabela;
	- devolve uma mensagem de resposta com oresultado.
*/
Message *process_message(Message *msg_pedido);
char *extract_ip(int sockfd);
Message *processHello(int sockfd);
/*
* retorna 0 (ok) ou -1 (erro)
*/
int send_message(int sockfd, Message *msg_resposta);
Message *receive(int sockfd);
int isPrimary(int sockfd);
void update_secondary(int sockfd, Message *pedido);
char *read_bk_adrr_port();
int write_bk_addrr_port(const char *addr_port);
/* Função "inversa" da função network_send_receive usada no table-client.
   Neste caso a função implementa um ciclo receive/send:

	Recebe um pedido;
	Aplica o pedido na tabela;
	Envia a resposta.
*/
int network_receive_send(int sockfd);
void testInput(int argc);
void init_fds(struct pollfd *fds);
void add_fd(struct pollfd *fds, int sock);
void printInfo();
Server *connect_to_backup(int argc, char **argv);
void *thread_main(void *param);
void terminate_thread();
void close_all();

int main(int argc, char **argv) {
	int connsock, i, count_fds, c;
	struct sockaddr_in client;
	socklen_t size_client = sizeof(struct sockaddr_in);
	char ip[1024];
	char input[20];
	
	primary_server = argc - 3;

	count_fds = 1;

	pthread_mutex_lock(&mutex_start);
	pthread_mutex_lock(&mutex_end);
	end_thread = 0;

	testInput(argc);
	
	init_fds(fds);

	fds[0].fd = fileno(stdin);
	fds[0].events = POLLIN;
	
	if ((fds[1].fd = make_server_socket(atoi(argv[2]))) < 0) {//server socket para atender clientes
		perror("ERRO: Não foi possível criar server socket\n");
		exit(EXIT_FAILURE);
	}
	fds[1].events = POLLIN;

	signal(SIGINT, catchSignal);
	signal(SIGPIPE, catchSignal);
	
	if (primary_server)
		printf("Servidor primario iniciado\n");
	else
		printf("Servidor secundario iniciado\n");
	
	if (table_skel_init(atoi(argv[1])) < 0) {
		perror("ERRO: Não foi possível criar tabela.\n");
		close_all();
		exit(EXIT_FAILURE);
	}

	bk = connect_to_backup(argc, argv);
	if (bk != NULL) {
		if (hello(bk) == -1) {
			bk->isConnected = 0;
			perror("Erro: Hello() failed\n");
		} 
		else
			printf("Hello() feito com sucesso\n");
		if (primary_server == 0) { //se é secundário
			printf("%s\n", "Iniciar update");
			update_state(bk);
			printf("%s\n", "Fim do update");
			network_close(bk);
			bk = NULL;
		}
	}

	//lançar thread
	if (primary_server)
		start_thread();

	printf("A Espera de clientes\n");

	while((c = poll(fds, N_OF_FDS, POLL_TIMEOUT)) >= 0) {
		if (c > 0) {
			if (fds[0].revents & POLLIN) {
				if (fgets(input, 20, stdin) != NULL)
					if (strcmp(input, "print\n") == 0)
						printInfo();
			}

			if ((fds[1].revents & POLLIN) && (count_fds < N_OF_FDS)) {
				if ((connsock = accept(fds[1].fd, (struct sockaddr *) &client, &size_client)) != -1) {
					add_fd(fds, connsock);
					count_fds++;
					if (inet_ntop(AF_INET, &client.sin_addr, ip, 1024) == NULL)
						memcpy("Não foi possível determinar o IP", ip, 33);
					printf("%d Client is connected! IP: %s\n", count_fds - 1, ip);
				}
			}
			
			for (i = 2; i <= N_OF_FDS; i++) {
				if (fds[i].revents & POLLIN) {
					if (network_receive_send(fds[i].fd) == -1) {
						close(fds[i].fd);
						printf("Cliente foi fechado\n");
						fds[i].fd = -1;
						fds[i].events = 0;
						count_fds--;
					} 
					else
						printf("Atendeu pedido\n");//por ip e port
				}
				if (fds[i].revents & POLLHUP) {
				    close(fds[i].fd);
					printf("Cliente foi fechado\n");
					fds[i].fd = -1;
					fds[i].events = 0;
					count_fds--;
				}
			}
		}
	}
	close_all();
	return 0;
}

void close_all() {
	terminate_thread();
	table_skel_destroy();
	close_fds();
	network_close(bk);
	bk = NULL;
}

void *thread_main(void *param) {
	while(1) {
		pthread_mutex_lock(&mutex_start);
		if (end_thread)
			break;
		if (bk != NULL && bk->isConnected) {
			Message *resp = send_receive(bk, share_with_thread);
			if (resp == NULL) {
				network_close(bk);
				bk = NULL;
			}
			else
				printf("%s\n", "Enviar para o secundario");
			free_message(resp);
		}
		pthread_mutex_unlock(&mutex_end);
	}
	pthread_mutex_unlock(&mutex_end);
	return NULL;
}

void close_fds() {
	int i;
	for (i = 1; i <= N_OF_FDS; i++)
	    if (fds[i].fd != -1)
	        close(fds[i].fd);
	printf("Servidor fechado\n");
}

void terminate_thread() {
	if (thread_is_on == 0)
		return;
	end_thread = 1;
	pthread_mutex_unlock(&mutex_start);
	void *status;
	pthread_join(thread, &status);
}

void catchSignal (int signal) {
	if (signal == SIGPIPE)
		return;
	close_all();
	exit(signal);
}

int make_server_socket(short port){
  int socket_fd, sim = 1;
  struct sockaddr_in server;

  if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
    perror("Erro ao criar socket");
    return -1;
  }

 	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (int *)&sim, sizeof(int)) < 0) {
		perror("setsockopt error");
	}

  server.sin_family = AF_INET;
  server.sin_port = htons(port);  
  server.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(socket_fd, (struct sockaddr *) &server, sizeof(server)) < 0){
      perror("Erro ao fazer bind");
      close(socket_fd);
      return -1;
  }

  if (listen(socket_fd, 0) < 0){
      perror("Erro ao executar listen");
      close(socket_fd);
      return -1;
  }
  return socket_fd;
}

void start_thread() {
	if (pthread_create(&thread, NULL, &thread_main, NULL) != 0) {
		perror("ERRO: Não foi possível lançar thread\n");
		close_all();
		exit(EXIT_FAILURE);
	}
	thread_is_on = 1;
}

Message *process_message(Message *msg_pedido){
	return invoke(msg_pedido);
}

char *extract_ip(int sockfd) {
	if (sockfd < 0)
		return NULL;
	struct sockaddr_in addr;
	socklen_t len = sizeof(struct sockaddr_in);
	if (getpeername(sockfd, (struct sockaddr *)&addr, &len) != 0)
		return NULL;
	char *result = malloc(sizeof(char) * 22);
	if (result == NULL)
		return NULL;
	//unsigned short port = ntohs(addr.sin_port);
	//sprintf(result, "%hu:", port);
	char ip[15];
	if (inet_ntop(AF_INET, &addr.sin_addr, ip, 20) == NULL) {
		free(result);
		return NULL;
	}
	strcpy(result, ip);
	strcat(result, ":50000");
	return result;
}

Message *processHello(int sockfd) {
	printf("%s\n", "Recebi um Hello");

	char *ip_p = read_bk_adrr_port();
	if (ip_p == NULL) {
		char *extracted_ip_p = extract_ip(sockfd);
		if (extracted_ip_p == NULL)
			return NULL;
		if (write_bk_addrr_port(extracted_ip_p) == -1) {
			free(extracted_ip_p);
			return NULL;
		}
		free(extracted_ip_p);
	}
	free(ip_p);

	Message *res = malloc(sizeof(Message));
	if (res == NULL)
		return NULL;
	res->opcode = OC_HELLO + 1;
	res->c_type = CT_RESULT;
	res->content.result = 0;
	
	return res;
}

int send_message(int sockfd, Message *msg_resposta) {
	char *message_resposta;
  	int message_size, msg_size, result;
	
	if (sockfd < 0)
		return -1;
	
	if (msg_resposta == NULL)
		return -1;
	
	/* Serializar a mensagem recebida */
	message_size = message_to_buffer(msg_resposta, &message_resposta);

	/* Verificar se a serialização teve sucesso */
	if (message_size == -1)
		return -1;

	/* Enviar ao cliente o tamanho da mensagem que será enviada
	   logo de seguida
	*/
	msg_size = htonl(message_size);
 	result = write_all(sockfd, (char *) &msg_size, _INT);

	/* Verificar se o envio teve sucesso */
	if (result != _INT) {
		free(message_resposta);
		return -1;
	}
	/* Enviar a mensagem que foi previamente serializada */

	result = write_all(sockfd, message_resposta, message_size);
	
	free(message_resposta);
	/* Verificar se o envio teve sucesso */
	if (result != message_size)
		return -1;
	return 0;
}

Message *receive(int sockfd) {
	char *message_pedido;
 	int msg_size, result;
  	Message *msg_pedido;
  
	/* Verificar parâmetros de entrada */
	if (sockfd < 0)
		return NULL;
	/* Com a função read_all, receber num inteiro o tamanho da 
	   mensagem de pedido que será recebida de seguida.*/
	result = read_all(sockfd, (char *) &msg_size, _INT);
	
	/* Verificar se a receção teve sucesso */
	if (result != _INT)
		return NULL;
	msg_size = ntohl(msg_size);
	/* Alocar memória para receber o número de bytes da
	   mensagem de pedido. */
	message_pedido = malloc(msg_size);
	if (message_pedido == NULL)
		return NULL;
	/* Com a função read_all, receber a mensagem de resposta. */
	result = read_all(sockfd, message_pedido, msg_size);

	/* Verificar se a receção teve sucesso */
	if (result != msg_size) {
		free(message_pedido);
		return NULL;
	}
	/* Desserializar a mensagem do pedido */
	msg_pedido = buffer_to_message(message_pedido, msg_size);
	free(message_pedido);
	/* Verificar se a desserialização teve sucesso */
	if (msg_pedido == NULL)
		return NULL;
	return msg_pedido;
}

int isPrimary(int sockfd) {
	char *temp = read_bk_adrr_port();
	if (temp == NULL)
		return -1;
	char *other = extract_ip(sockfd);
	if (other == NULL) {
		free(temp);
		return -1;
	}
	int result = strcmp(temp, other);
	//printf("%s %s\n", temp, other);
	free(temp);
	free(other);
	return result == 0 ? 1 : 0;
}

void update_secondary(int sockfd, Message *pedido) {
	if (sockfd < 0 || pedido == NULL || pedido->opcode != OC_UPDATE_STATE)
		return;
	printf("%s\n", "Atualizar o secundário");
	pedido->opcode = OC_GET;
	Message *resposta = invoke(pedido);
	if (resposta == NULL)
		return;
	Server server;
	server.sockfd = sockfd;
	server.isConnected = 1;
	server.addr_port = NULL;
	Message *p = NULL;
	do {
		free_message(p);
		p = send_receive(&server, resposta);
		free_message(resposta);
		if (p == NULL || p->opcode == OC_UPDATE_STATE + 1) {
			p->opcode = OC_UPDATE_STATE + 1;
			send_message(sockfd, p);
			break;
		}
		resposta = invoke(p);
	} while (p != NULL);
	free_message(p);
	if (primary_server) {
		char *addr_port = read_bk_adrr_port();
		bk = network_connect(addr_port);
		free(addr_port);
	}
}

char *read_bk_adrr_port() {
	char *temp = malloc(25);
	if (temp == NULL)
		return NULL;
	if (open_file(file_name) == -1) {
		perror("ERRO: Nao foi possivel abrir o ficheiro de setup\n");
		close_all();
		exit(EXIT_FAILURE);
	}
	if (read_from_file("IP_PORT", temp) == -1) {
		close_file();
		free(temp);
		return NULL;
	}
	close_file();
	return temp;
}

int write_bk_addrr_port(const char *addr_port) {
	if (open_file(file_name) == -1) {
		perror("ERRO: Nao foi possivel abrir o ficheiro de setup\n");
		close_all();
		exit(EXIT_FAILURE);
	}
	int result = write_to_file("IP_PORT", addr_port);
	close_file();
	return result;
}

int network_receive_send(int sockfd){
  	int wait = 0;
  	int result;
  	Message *msg_pedido, *msg_resposta;
	if (sockfd < 0)
		return -1;
		
	msg_pedido = receive(sockfd);
	if (msg_pedido == NULL)
		return -1;
	
	if (primary_server == 0 && isPrimary(sockfd) == 0) { //se eu for secundario e o pedido nao é do primario
		primary_server = 1; //passo ser eu o primario
		start_thread();
	}
	if (primary_server)
		if (msg_pedido->opcode == OC_PUT || msg_pedido->opcode == OC_DEL || msg_pedido->opcode == OC_UPDATE) {
			share_with_thread = msg_pedido;
			pthread_mutex_unlock(&mutex_start);//comecar atualizar o secundario
			wait = 1;
		}
	if (msg_pedido->opcode == OC_HELLO)
		msg_resposta = processHello(sockfd);
	else if (msg_pedido->opcode == OC_UPDATE_STATE) {
		update_secondary(sockfd, msg_pedido);
		free_message(msg_pedido);
		return 0;
	}
	else {
		msg_resposta = process_message(msg_pedido);
	}
	free_message(msg_pedido);
	if (msg_resposta == NULL)
		return -1;
	if (wait)
		pthread_mutex_lock(&mutex_end);//espera ate que o secundario seja atualizado
	result = send_message(sockfd, msg_resposta);
	free_message(msg_resposta);
	return result;
}

void testInput(int argc) {
	if (argc != 3 && argc != 4){
		printf("Uso: ./server <dimensão da tabela> <porta TCP> <IP:Port>\n");
		printf("Exemplo de uso: ./table-server 54321 10\n");
		exit(EXIT_FAILURE);
	}
}

void init_fds(struct pollfd *fds) {
	int i;
	for (i = 0; i <= N_OF_FDS; i++) {
		fds[i].fd = -1;
		fds[i].events = 0;
	}
}

void add_fd(struct pollfd *fds, int sock){
	int i = 1;//0- stdin, 1 - server socket
	while (++i <= N_OF_FDS) //começa pelo i = 2
		if (fds[i].fd < 0) {
			fds[i].fd = sock;
			fds[i].events = POLLIN;
			break;
		}
}

void printInfo() {
	int i = 0;
	printf("**********\n");
	printf("Servidor: %s\n", primary_server ? "primário" : "secundário");
	printf("\n");
	char **keys = table_skel_get_keys();
	if (keys != NULL) {
		while(keys[i] != NULL) {
			Data *data = table_skel_get(keys[i]);
			if (data != NULL) {
				printf("Chave: %s Datasize: %d\n", keys[i], data->datasize);
				data_destroy(data);
			}
			i++;
		}
		table_skel_free_keys(keys);
	}
	printf("**********\n");
}

Server *connect_to_backup(int argc, char **argv) {
	Server *result = NULL;
	char *i;
	if (argc == 3) {//se for iniciado como secundario
		i = read_bk_adrr_port();
		result = network_connect(i);
		free(i);
	}
	else { //se for iniciado como primario
		i = read_bk_adrr_port();
		if (i == NULL) {
			write_bk_addrr_port(argv[3]);
			i = read_bk_adrr_port();
		}
		result = network_connect(i);
		free(i);
	}
	return result;
}