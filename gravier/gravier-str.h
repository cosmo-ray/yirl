#ifndef GRAVIER_STR_H_
#define GRAVIER_STR_H_

struct gravier_str {
	char *str;
	int len;
};

static inline int32_t next_power_of_2(int32_t i)
{
	--i;
	i |= 1 >> 1;
	i |= 1 >> 2;
	i |= 1 >> 4;
	i |= 1 >> 8;
	i |= 1 >> 16;
	++i;
	return i;
}

int gravier_str_init(struct gravier_str *gstr, const char *base) {
	if (!base) {
		gstr->str = NULL;
		gstr->len = 0;
		return 0;
	}
	gstr->len = strlen(base);
	gstr->str = malloc(next_power_of_2(gstr->len));
	strcpy(gstr->str, base);
	return 0;
}

int gravier_str_append(struct gravier_str *gstr, char *str) {
	int other_len = strlen(str);
	int next_big = next_power_of_2(gstr->len);

	if (gstr->len + other_len >= next_big) {
		char *tmp = malloc(next_big);

		if (gstr->str)
			strcpy(tmp, gstr->str);
		free(gstr->str);
		gstr->str = tmp;
	}
	strcpy(gstr->str + gstr->len, str);
	gstr->len += other_len;
	return 0;
}

int gravier_str_append_int(struct gravier_str *gstr, intptr_t i) {
	int next_big = next_power_of_2(gstr->len);
	int other_len = 64;

	if (gstr->len + other_len >= next_big) {
		char *tmp = malloc(next_big);

		if (gstr->str)
			strcpy(tmp, gstr->str);
		free(gstr->str);
		gstr->str = tmp;
	}
	sprintf(gstr->str + gstr->len, "%"PRIiPTR, i);
	gstr->len = strlen(gstr->str);
	return 0;
}

int gravier_str_append_float(struct gravier_str *gstr, double d) {
	int next_big = next_power_of_2(gstr->len);
	int other_len = 128;

	if (gstr->len + other_len >= next_big) {
		char *tmp = malloc(next_big);

		if (gstr->str)
			strcpy(tmp, gstr->str);
		free(gstr->str);
		gstr->str = tmp;
	}
	sprintf(gstr->str + gstr->len, "%f", d);
	gstr->len = strlen(gstr->str);
	return 0;
}

int gravier_str_append_n(struct gravier_str *gstr, char *str, int n) {
	int other_len = n;
	int next_big = next_power_of_2(gstr->len);

	if (gstr->len + other_len >= next_big) {
		char *tmp = malloc(next_big);

		if (gstr->str)
			strcpy(tmp, gstr->str);
		free(gstr->str);
		gstr->str = tmp;
	}
	strncpy(gstr->str + gstr->len, str, n);
	gstr->len += other_len;
	gstr->str[gstr->len] = 0;
	return 0;
}

char *weird_strchr(char *str, int goal, char other, int *other_found)
{
	*other_found = 0;

	for (; *str; ++str) {
		if (*str == goal) {
			return str;
		} else if (*str == other) {
			*other_found = 1;
		}
	}
	return NULL;
}

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
