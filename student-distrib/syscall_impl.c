/* syscall_impl, implement syscalls
 * vim:ts=4 sw=4 noexpandtab
 */
#include "types.h"
#include "syscall.h"
#include "lib.h"

int32_t sys_open(const uint8_t *filename)
{
	return 0;
}

int32_t sys_read(int32_t fd, void *buf, int32_t nbytes)
{
	return 0;
}

int32_t sys_write(int32_t fd, const void *buf, int32_t nbytes)
{
	return 0;
}

int32_t sys_close(int32_t fd)
{
	return 0;
}

int32_t sys_exec(const uint8_t *command)
{
	return 0;
}

int32_t sys_halt(uint8_t status)
{
	return 0;
}

