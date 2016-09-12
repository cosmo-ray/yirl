/*
**Copyright (C) 2016 Matthias Gatto
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

#ifndef _STACK_
#define _STACK_

typedef struct int8_stack {
	int8_t *values;
	size_t len;
	size_t total_size;
} int8_stack;

typedef struct int16_stack {
	int16_t *values;
	size_t len;
	size_t total_size;
} int16_stack;

typedef struct int32_stack {
	int32_t *values;
	size_t len;
	size_t total_size;
} int32_stack;

typedef struct int64_stack {
	int64_t *values;
	size_t len;
	size_t total_size;
} int64_stack;

typedef struct ptr_stack {
	void **values;
	size_t len;
	size_t total_size;
} ptr_stack;

#define STACK_CREATE(name, type)		\
	type##_stack name = {NULL, 0, 0}	\

#define stack_push(stack, val)	do {					\
		if (unlikely(stack.len == stack.total_size)) {		\
			stack.total_size += 1024;			\
			stack.values = realloc(stack.values,		\
					       stack.total_size *	\
					       sizeof(val));		\
		}							\
		stack.values[stack.len] = val;				\
		stack.len += 1;						\
	} while (0)

/**
 * @brief pop a value from @stack and asignit it to @ret
 * @stack the stack
 * @ret where the poped value is store
 * @fail_val the value assigne to ret if stack is empty
 */
#define STACK_POP(stack, ret, fail_val) do {			\
		if (!stack.len) {				\
			ret = fail_val;				\
		} else {					\
			stack.len -= 1;				\
			ret = stack.values[stack.len];		\
		}						\
	} while (0)

static inline int8_t int8_stack_pop(int8_stack *stack, int8_t fail_ret)
{
	if (!stack->len)			\
		return fail_ret;		\
	stack->len -= 1;			\
	return stack->values[stack->len];
}

static inline int16_t int16_stack_pop(int16_stack *stack, int16_t fail_ret)
{
	if (!stack->len)			\
		return fail_ret;		\
	stack->len -= 1;			\
	return stack->values[stack->len];
}

static inline int32_t int32_stack_pop(int32_stack *stack, int32_t fail_ret)
{
	if (!stack->len)			\
		return fail_ret;		\
	stack->len -= 1;			\
	return stack->values[stack->len];
}

static inline int64_t int64_stack_pop(int64_stack *stack, int64_t fail_ret)
{
	if (!stack->len)
		return fail_ret;
	stack->len -= 1;
	return stack->values[stack->len];
}

static inline void *ptr_stack_pop(ptr_stack *stack, void *fail_ret)
{
	if (!stack->len)			\
		return fail_ret;		\
	stack->len -= 1;			\
	return stack->values[stack->len];
}

#define stack_pop(stack, fail_val) _Generic((stack),			\
					    int8_stack: int8_stack_pop,	\
					    int16_stack: int16_stack_pop, \
					    int32_stack: int32_stack_pop, \
					    int64_stack: int64_stack_pop, \
					    ptr_stack: ptr_stack_pop	\
		)(&(stack), fail_val)

#define stack_destroy(stack) (free(stack.values))

#endif
