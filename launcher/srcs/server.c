/* ************************************************************************** */
/*                                                                            */
/*                                                                            */
/*   server.c                                                                 */
/*                                                                            */
/*   By: Mateo <teorodrip@protonmail.com>                                     */
/*                                                                            */
/*   Created: 2019/01/07 10:45:39 by Mateo                                    */
/*   Updated: 2019/02/05 12:40:29 by Mateo                                    */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/launcher.h"

// initialize the server
void init_server(server_t *srv)
{
	// create the socket
	if((srv->server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			dprintf(2,"Error: opening the socket\n");
			exit(EXIT_FAILURE);
		}
	// set te file descriptor of socket non blocking
	if (fcntl(srv->server_fd, F_SETFL, O_NONBLOCK) < 0)
		{
			dprintf(2,"Error: setting fd flag\n");
			exit(EXIT_FAILURE);
		}
	//to close the socket when program is finished
	if (setsockopt(srv->server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0)
		{
			dprintf(2,"Error: setting fd optinon\n");
			exit(EXIT_FAILURE);
		}
	// load the data in the socket structure
	srv->server_data.sin_family = AF_INET;
	srv->server_data.sin_addr.s_addr = htons(ADDR);
	srv->server_data.sin_port = htons(PORT);
	srv->server_data_len = sizeof(srv->server_data);
	// connect the socket
	if ((bind(srv->server_fd, (struct sockaddr *)&srv->server_data,
						srv->server_data_len)) < 0)
		{
			dprintf(2, "Error: binding the socket\n");
			exit(EXIT_FAILURE);
		}
	// listen for clients
	if((listen(srv->server_fd, MAX_CONNECTIONS)) < 0)
		{
			dprintf(2, "Error: listening the clients\n");
			exit(EXIT_FAILURE);
		}
}

// assign a client when a connection is made
static void assign_client(const int fd, client_t *new_cli, client_t **cli_head)
{
	client_t *tmp;
	client_t *tmp_prev;

	new_cli->client_fd = fd;
	new_cli->is_vm = 0;
	tmp_prev = *cli_head;
	// if there are no clients just put in head
	if (!tmp_prev)
		{
			new_cli->id = 0;
			new_cli->next = *cli_head;
			*cli_head = new_cli;
			return;
		}
	// search for the client id, to reutulize the id number of the clients
	// instead of increase it each time one disconnects and other connects
	tmp = tmp_prev->next;
	while (tmp)
		{
			if  (tmp_prev->id - tmp->id > 1)
				{
					new_cli->id = tmp->id + 1;
					tmp_prev->next = new_cli;
					new_cli->next = tmp;
					return;
				}
			tmp_prev = tmp;
			tmp = tmp->next;
		}
	// te last client should be the 0 so if not assign the 0 to the new client
	if (tmp_prev->id)
		{
			new_cli->id = tmp_prev->id - 1;
			tmp_prev->next = new_cli;
			new_cli->next = NULL;
		}
	// if all the list of clients has no holes just add the client in front
	else
		{
			new_cli->id = (*cli_head)->id + 1;
			new_cli->next = *cli_head;
			*cli_head = new_cli;
		}
}

void accept_client(const server_t *srv, client_t **cli_head)
{
	int fd;
	client_t *new_cli;

	// loop while there are clients to accept
	while ((fd = accept(srv->server_fd, (struct sockaddr *)&srv->server_data,
											(socklen_t *)&srv->server_data_len)) >= 0)
		{
			// allocate memory for the new client
			if (!(new_cli = (client_t *)malloc(sizeof(client_t))))
				{
					dprintf(2, "Error: in malloc accept_client\n");
					exit(EXIT_FAILURE);
				}
			// set connection non blocking
			if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
				{
					dprintf(2,"Error: setting fd flag\n");
					exit(EXIT_FAILURE);
				}
			// assign the client
			assign_client(fd, new_cli, cli_head);
			*cli_head = new_cli;
			printf("A client has made a connection\n");
		}
	if (errno != EAGAIN && errno != EWOULDBLOCK)
		{
			printf("Error: in accept connection\n");
			exit(EXIT_FAILURE);
		}
}

// disconnect client from list
void disconnect_client(client_t *prev, client_t **cli, client_t **head)
{
	client_t *tmp;

	if (*cli == *head)
		*head = (*cli)->next;
	else if ((*cli)->next == NULL)
		prev->next = NULL;
	else
		prev->next = (*cli)->next;
	close((*cli)->client_fd);
	tmp = *cli;
	*cli = prev->next;
	free(tmp);

}

// read the clients file descriptors to obtain his querys
void read_clients(client_t **head, tickers_t *tickers, uint64_t *flags)
{
	client_t *cli;
	client_t *prev;
	char buff[BUFF_SIZE];
	ssize_t readed;

	prev = *head;
	cli = *head;
	// loop trough all the client list
	while (cli)
		{
			// if there is something to read a query has been made
			while ((readed = read(cli->client_fd, buff, BUFF_SIZE)) > 0)
				{
					decode_data(buff, readed, *head, cli, tickers, flags);
				}
			// if 0 or error is readed a client has disconnected
			if (readed == 0 || (readed == -1 && (errno != EAGAIN && errno != EWOULDBLOCK)))
				{
					disconnect_client(prev, &cli, head);
					printf("Client disconnected\n");
					continue;
				}
			prev = cli;
			cli = cli->next;
		}
}
