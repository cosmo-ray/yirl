/*
**Copyright (C) 2020 Matthias Gatto
**
**This program is free software: you can redistribute it and/or modify
**it under the terms of the GNU Lesser General Public License as published by
**the Free Software Foundation, either version 3 of the License, or
**(at your option) any later version.
**
**This program is distributed in the hope that it will be useful,
**but WITHOUT ANY WARRANTY; without even the implied warranty of
**MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**GNU General Public License for more details.
**
**You should have received a copy of the GNU Lesser General Public License
**along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <sys/types.h>
#ifdef _WIN32
/*
 * That is 80 % sure broken on windows, but with that gcc shut up about this
 * Also I don't really use network yet
 */
#include <winsock2.h>
typedef int socklen_t;
static int nb_init;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#include <errno.h>
#include <string.h>

#include "utils.h"
#include "simple-net.h"

int ysnTcpOpenConnect(const char *dst_ip, int port)
{
#ifdef _WIN32
	if (!nb_init) {
		WSADATA wsaData;
		int iResult = WSAStartup(MAKEWORD(2 ,2), &wsaData);
		if (iResult != 0) {
			DPRINT_ERR("error at WSASturtup\n");
			return -1;
		}
		++nb_init;
	}
#endif
	struct sockaddr_in s_nfo  = {0};
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	int r;

	if (fd < 0) {
		DPRINT_ERR("socket open fail: %s\n", strerror(errno));
		return fd;
	}
	s_nfo.sin_addr.s_addr = inet_addr(dst_ip);
	s_nfo.sin_port = htons(port);
	s_nfo.sin_family = AF_INET;
	r = connect(fd, (void *)&s_nfo, sizeof(s_nfo));
	if (r < 0) {
		DPRINT_ERR("connect fail: %s\n", strerror(errno));
		return r;
	}
	return fd;
}

union y_listen_ret ysnTcpOpenListener(const char *src_ip, int port)
{
	struct sockaddr_in s_nfo = {0};
	int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	socklen_t l;
	int r;

	if (fd < 0) {
		DPRINT_ERR("socket open fail: %s\n", strerror(errno));
		return (union y_listen_ret){ .r = fd };
	}
	s_nfo.sin_addr.s_addr = INADDR_ANY;
	s_nfo.sin_port = htons(port);
	s_nfo.sin_family = AF_INET;
	r = bind(fd, (void *)&s_nfo, sizeof(s_nfo));
	if (r < 0) {
		DPRINT_ERR("bind fail: %s\n", strerror(errno));
		return (union y_listen_ret){ .r = r };
	}

	if (listen(fd, 10000)) {
		DPRINT_ERR("listen fail: %s\n", strerror(errno));
		return (union y_listen_ret){ .r = -1 };
	}
	s_nfo.sin_addr.s_addr = INADDR_ANY;
	r = accept(fd, (void *)&s_nfo, &l);
	if (r < 0) {
		DPRINT_ERR("accept fail: %s\n", strerror(errno));
		return (union y_listen_ret){ .r = -1 };
	}
	return (union y_listen_ret){.sfd = fd, .afd = r};
}
