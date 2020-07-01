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

#ifndef	YIRL_SIMPLE_NET_H_
#define	YIRL_SIMPLE_NET_H_

int ysnTcpOpenConnect(const char *dst_ip, int port);

union y_listen_ret {
	long long r;
	struct {
		int sfd;
		int afd;
	};
};

union y_listen_ret ysnTcpOpenListener(const char *dst_ip, int port);

#endif
