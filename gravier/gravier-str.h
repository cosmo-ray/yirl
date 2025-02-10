#ifndef GRAVIER_STR_H_
#define GRAVIER_STR_H_

void graviver_str_small_replace(char *str, const char *what, const char *with)
{
	const char *origin = str;
	int with_len = strlen(with);
	int what_len = strlen(what);
	int str_len = strlen(str);
	const char *cpy_src;
again:
	str = strstr(str, what);
	if (!str)
		return;
	strcpy(str, with);
	cpy_src = str + what_len;
	str += with_len;
	memmove(str, cpy_src, str_len - (cpy_src - origin) + 1);
	goto again;
}

#endif
