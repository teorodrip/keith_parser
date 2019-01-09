// ************************************************************************** //
//                                                                            //
//                                                                            //
//   parser.hpp                                                               //
//                                                                            //
//   By: Mateo <teorodrip@protonmail.com>                                     //
//                                                                            //
//   Created: 2019/01/08 19:02:25 by Mateo                                    //
//   Updated: 2019/01/09 18:39:07 by Mateo                                    //
//                                                                            //
// ************************************************************************** //

#ifndef PARSER_HPP
#define PARSER_HPP

#include <sys/socket.h>
#include <netinet/in.h>
#include <xlsxio_read.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <iostream>

#define META_INFO_LEN 3
#define PORT 8080
#define ADDR INADDR_ANY
#define BUFF_SIZE 1024
#define FILE_NAME "./sheet2.xlsx"
#define DEFAULT_PATH "./outputs_windows_"

typedef struct tickers_s
{
	size_t len;
	char **tickers;
} tickers_t;

class client
{
private:
	struct sockaddr_in serv_addr;
	int sockfd;

public:
	client();
	void init();
	size_t get_number_tickers();
	unsigned char get_watching_directories();
};

class dir_watcher
{
private:
	std::string path;
	int fd_notify;

	void manage_event(struct inotify_event *event);
public:
	dir_watcher();
	dir_watcher(const unsigned char machine_id);
	void watch_directory();
};

#endif