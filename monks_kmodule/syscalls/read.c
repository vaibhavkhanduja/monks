#include "syscall.h"

extern asmlinkage void hooked_sys_read(unsigned int fd, char __user *buf, size_t count);
#ifdef CONFIG_IA32_EMULATION
extern asmlinkage void hooked_sys32_read(unsigned int fd, char __user *buf, size_t count);
#endif

__REGISTER_SYSCALL(read);

asmlinkage void hooked_sys_read(unsigned int fd, char __user *buf, size_t count){
	ssize_t r;
	syscall_intercept_info *i;

	__GET_SYSCALL_RESULT(r);

	i = new(sizeof(struct syscall_intercept_info));
	if(i){
		i->pname = current->comm;
		i->pid = current->pid;
		i->operation = "READ";
		i->path = path_from_fd(fd);

		if(IS_ERR((void *)r)){
			i->result = "Error";
			i->details = kasprintf(GFP_KERNEL, "Errno %zd", r);
		}else{
			i->result = "Ok";
			i->details = kasprintf(GFP_KERNEL, "Read %zd bytes (was requested to read %zd)", r, count);
		}

		nl_send(i);

		del(i->path);
		del(i->details);
		del(i);
	}else{
		//something bad happened, can't show results
	}
}

#ifdef CONFIG_IA32_EMULATION
__REGISTER_SYSCALL32(read);

asmlinkage void hooked_sys32_read(unsigned int fd, char __user *buf, size_t count){
	ssize_t r;
	syscall_intercept_info *i;

	__GET_SYSCALL_RESULT32(r);

	i = new(sizeof(struct syscall_intercept_info));
	if(i){
		i->pname = current->comm;
		i->pid = current->pid;
		i->operation = "READ32";
		i->path = path_from_fd(fd);

		if(IS_ERR((void *)r)){
			i->result = "Error";
			i->details = kasprintf(GFP_KERNEL, "Errno %zd", r);
		}else{
			i->result = "Ok";
			i->details = kasprintf(GFP_KERNEL, "Read %zd bytes (was requested to read %zd)", r, count);
		}

		if(count == 1 && fd == 0)
			nl_send(i);

		del(i->path);
		del(i->details);
		del(i);
	}else{
		//something bad happened, can't show results
	}
}

#endif