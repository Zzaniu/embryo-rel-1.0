
/**
 * copyright(c) 2017-3-6 shangwen wu 
 *
 * 系统文件结构相关实现
 * 
 */

#include <common.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/system.h>
#include <sys/syscallarg.h>
#include <sys/param.h>
#include <sys/malloc.h>
#include <sys/file.h>
#include <sys/proc.h>

int nfiles = 0;					//当前所有被打开的设备个数
struct list_head allfileslist = LIST_HEAD_INIT(allfileslist);//全局文件链表


/**
 * 描述：soc_close底层实现
 */
int sys_close(struct proc *p, void *args, register_t *retval)
{
	int fd;
	struct filedesc *fdesc;

	struct sys_ioctl_args /*{
	syscallarg(int) fd;
	} */ *ap = (struct sys_ioctl_args *)args;

	fd = SCARG(ap, fd);

	fdesc = p->p_fd;

#if 0
	/* 由于fd_nfiles与fd不同步导致不能轻易使用下面的判断 */
	if(fd >= fdesc->fd_nfiles)
		return EBADF;
#else
	if(fd >= NFILE)
		return EBADF;
#endif

	return frelease(p, fd);
}

int frelease(struct proc *p, int fd)
{
	int ret = 0;
	struct file *fp;
	struct filedesc *fdescp = p->p_fd;
	
	if(!(fp = fdescp->fd_ofiles[fd]))
		return EBADF;

	/* 只有当引用为0时，才真正释放资源 */
	if(--fp->f_refcnt)
		return 0;
	if(fp->f_refcnt < 0)
		panic("file refcnt < 0");

	/* 调用具体外设的close函数 */
	if(fp->f_ops->fo_close)
		ret = (*fp->f_ops->fo_close)(fp, p);
	
	ffree(p, fd);

	return ret;
}

/**
 * 描述：创建一个file描述，并将该结构与当前进程相关联
 */
int falloc(struct proc *p, int *fdp, struct file **fpp)
{
	int err;
	struct filedesc *fdescp = p->p_fd;
	struct file *fp;
	extern void __open_err(int fd);
	
	if(!fdescp)
		return EINVAL; 
	if(nfiles >= maxfiles) 
		return ENFILE;

	if(err = fdalloc(fdp)) 
		return err;
	if(*fdp >= NFILE) {
		err = EMFILE; 
		goto failed;
	}
	if(NULL == (fp = (struct file *)kmem_malloc(sizeof(struct file)))) {
		err = ENOMEM;
		goto failed;
	}
	
	/* 将当前file结构插入到allfile链表中 */
	list_add(&fp->f_list, &allfileslist);
	/* 将当前file保存到filedesc数组*/
	fdescp->fd_ofiles[*fdp] = fp;
	fp->f_refcnt = 1;	
	fdescp->fd_nfiles++;
	nfiles++;
	*fpp = fp;

	return 0;

failed:
	/* 调用用户空间的函数进行fd句柄释放操作 */
	//fdfree(*fdp);
	__open_err(*fdp);	

	return err;
}

/**
 * 描述：释放一个file结构
 */
void ffree(struct proc *p, int fd)
{
	struct filedesc *fdescp = p->p_fd;
	struct file *fp = fdescp->fd_ofiles[fd];

	list_del(&fp->f_list);
	nfiles--;
	kmem_free(fp);	
	fdescp->fd_ofiles[fd] = NULL;
	fdescp->fd_nfiles--;
	//fdfree(fd);
	//__open_err(fd);	
}

/**
 * 描述：获取一个对应于用户态访问的fd句柄
 */
int fdalloc(int *fdp)
{
	int fd;	
	extern int open(const char *fname, int mode);//defined in lib/open.c
	
	//bad code，内核态居然调用上层依赖性这么强的库函数，严重破坏了代码的分层设计
	/* 主要是为了得到一个当前进程空闲的文件描述符 */
	if(-1 == (fd = open("/dev/socket", 0))) {
		return errno;
	}

	*fdp = fd;

	return 0;
}

/**
 * 描述：释放fd句柄资源
 */
void fdfree(int fd)
{
	/* 注意：原本在这里释放空闲的fd描述符，但是这里的工作交给应用层调用close函数完成 */
	//close(fd);
}
