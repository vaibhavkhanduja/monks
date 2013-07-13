#include "syshijack.h"

/*****************************************************************************\
| (For each syscall) Define a function which we will use to save the REAL     |
| syscall function and define a FAKE function which we will use to replace    |
| the REAL one.                                                               |
| Also, do that for x86 and x64 cases, AND for the special IA32 case.         |
\*****************************************************************************/

/* __NR_read / __NR_read32 */
asmlinkage long (*real_sys_read)(unsigned int fd, char __user *buf, size_t count);
asmlinkage long hooked_sys_read(unsigned int fd, char __user *buf, size_t count){
	ssize_t r;
	syscall_info *i = kmalloc(sizeof(syscall_info), GFP_KERNEL);

	r = real_sys_read(fd, buf, count);

	i->pname = current->comm;
	i->pid = current->pid;
	i->operation = "READ";
	i->path = path_from_fd(fd);
	i->result = r;
	i->details = "No details ATM";

	if(count == 1 && fd == 0)
		print_info(i);

	kfree(i);

	return r;
}
#ifdef CONFIG_IA32_EMULATION
asmlinkage long (*real_sys_read32)(unsigned int fd, char __user *buf, size_t count);
asmlinkage long hooked_sys_read32(unsigned int fd, char __user *buf, size_t count){
	ssize_t r;
	syscall_info *i = kmalloc(sizeof(syscall_info), GFP_KERNEL);

	r = real_sys_read32(fd, buf, count);

	unhook_calls();

	i->pname = current->comm;
	i->pid = current->pid;
	i->operation = "READ32";
	i->path = path_from_fd(fd);
	i->result = r;
	i->details = "No details ATM";

	if(count == 1 && fd == 0)
		print_info(i);

	kfree(i->path);
	kfree(i);

	hook_calls();

	return r;
}
#endif

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/

void hook_calls(void){
	sys_call_table = get_writable_sct(get_sys_call_table());
	if(sys_call_table == NULL){
		printk(KERN_INFO "sys_call_table is NULL\n");
		return;
	}
#ifdef CONFIG_IA32_EMULATION
	ia32_sys_call_table = get_writable_sct(get_ia32_sys_call_table());
	if(ia32_sys_call_table == NULL){
		vunmap((void*)((unsigned long)sys_call_table & PAGE_MASK));
		printk(KERN_INFO "ia32_sys_call_table is NULL\n");
		return;
	}
#endif

/*****************************************************************************\
| This is where the magic happens. We call HOOK (and maybe HOOK_IA32) for     |
| each syscall.                                                               |
| The macros HOOK and HOOK_IA32 replace the REAL functions with the FAKE      |
| ones. See syshijack.h for more info.                                        |
\*****************************************************************************/

/* __NR_read / __NR32_read */
	HOOK(__NR_read, real_sys_read, hooked_sys_read);
#ifdef __NR32_read
	HOOK_IA32(__NR32_read, real_sys_read32, hooked_sys_read32);
#endif

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/

}

void unhook_calls(void){

/*****************************************************************************\
| This is where we restore the REAL functions, aka, undo what HOOK and        |
| HOOK_IA32 did.                                                              |
\*****************************************************************************/

/* __NR_read / __NR_read32 */
	UNHOOK(__NR_read, real_sys_read);
#ifdef __NR32_read
	UNHOOK_IA32(__NR32_read, real_sys_read32);
#endif

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/

	vunmap((void*)((unsigned long)sys_call_table & PAGE_MASK));
#ifdef CONFIG_IA32_EMULATION
	vunmap((void*)((unsigned long)ia32_sys_call_table & PAGE_MASK));
#endif
}