
/**
 * Copyright(c) 2018-11-27 Shangwen Wu	
 *
 * EXT2兼容文件系统相关实现 
 * 
 */
#include <common.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/list.h>
#include <sys/partition.h>
#include <sys/blkio.h>
#include <sys/direntinfo.h>
#include <sys/ext2io.h>
#include <sys/stat.h>
#include <fs/file.h>
#include <fs/fsio.h>
#include <fs/blkfile.h>
#include <fs/ext2.h>

#define SECTOR_SZ		512			//硬盘扇区字节数 bad code

/* ext2调试信息开关 */
#define EXT2_DEBUG	0
#if EXT2_DEBUG
#define EXT2_DBG(fmt, args...)		printf(fmt, ##args)
#else
#define EXT2_DBG(fmt, args...)		do{}while(0)	
#endif

#if EXT2_DEBUG
/**
 * 描述：打印超级块信息
 */
static void dump_super_block(struct ext2_super_block *sb)
{
	int i;

	EXT2_DBG("Super block info:\n");
	EXT2_DBG("%-*s%u\n", 40, "Inodes count", sb->s_inodes_count);
	EXT2_DBG("%-*s%u\n", 40, "Blocks count", sb->s_blocks_count);				
	EXT2_DBG("%-*s%u\n", 40, "Reserved blocks count", sb->s_r_blocks_count);			
	EXT2_DBG("%-*s%u\n", 40, "Free blocks count", sb->s_free_blocks_count);		
	EXT2_DBG("%-*s%u\n", 40, "Free inodes count", sb->s_free_inodes_count);		
	EXT2_DBG("%-*s%u\n", 40, "First Data Block", sb->s_first_data_block);			
	EXT2_DBG("%-*s%u\n", 40, "Block size(log2)", sb->s_log_block_size);  			
	EXT2_DBG("%-*s%d\n", 40, "Fragment size(log2)", sb->s_log_frag_size);   			
	EXT2_DBG("%-*s%u\n", 40, "Blocks per group", sb->s_blocks_per_group);			
	EXT2_DBG("%-*s%u\n", 40, "Fragments per group", sb->s_frags_per_group); 			
	EXT2_DBG("%-*s%u\n", 40, "Inodes per group", sb->s_inodes_per_group);			
	EXT2_DBG("%-*s%u\n", 40, "Mount time", sb->s_mtime);					
	EXT2_DBG("%-*s%u\n", 40, "Write time", sb->s_wtime);					
	EXT2_DBG("%-*s%hu\n", 40, "Mount count", sb->s_mnt_count);				
	EXT2_DBG("%-*s%hd\n", 40, "Maximal mount count", sb->s_max_mnt_count);			
	EXT2_DBG("%-*s0x%x\n", 40, "Magic signature", sb->s_magic);					
	EXT2_DBG("%-*s0x%x\n", 40, "File system state", sb->s_state);					
	EXT2_DBG("%-*s%hu\n", 40, "Behaviour when detecting errors", sb->s_errors);					
	EXT2_DBG("%-*s%hu\n", 40, "minor revision level", sb->s_minor_rev_level); 			
	EXT2_DBG("%-*s%u\n", 40, "time of last check", sb->s_lastcheck);				
	EXT2_DBG("%-*s%u\n", 40, "max. time between checks", sb->s_checkinterval);			
	EXT2_DBG("%-*s0x%x\n", 40, "OS", sb->s_creator_os);				
	EXT2_DBG("%-*s%u\n", 40, "Revision level", sb->s_rev_level);				
	EXT2_DBG("%-*s0x%x\n", 40, "Default uid for reserved blocks", sb->s_def_resuid);				
	EXT2_DBG("%-*s0x%x\n", 40, "Default gid for reserved blocks", sb->s_def_resgid);				
	EXT2_DBG("%-*s%u\n", 40, "First non-reserved inode", sb->s_first_ino);
	EXT2_DBG("%-*s%hu\n", 40, "size of inode structure", sb->s_inode_size); 				
	EXT2_DBG("%-*s%hu\n", 40, "block group of this superblock", sb->s_block_group_nr);
	EXT2_DBG("%-*s0x%x\n", 40, "compatible feature set", sb->s_feature_compat);
	EXT2_DBG("%-*s0x%x\n", 40, "incompatible feature set", sb->s_feature_incompat);
	EXT2_DBG("%-*s0x%x\n", 40, "readonly-compatible feature set", sb->s_feature_ro_compat); 		
	EXT2_DBG("%-*s", 40, "128-bit uuid for volume");
	for(i = 0; i < 16; ++i)	
		EXT2_DBG("%02x",sb->s_uuid[i]); 
	EXT2_DBG("\n");
	EXT2_DBG("%-*s%s\n", 40, "volume name", sb->s_volume_name); 		
	EXT2_DBG("%-*s%s\n", 40, "directory where last mounted", sb->s_last_mounted); 	
	EXT2_DBG("%-*s0x%x\n", 40, "For compression", sb->s_algorithm_usage_bitmap);   
	EXT2_DBG("%-*s%d\n", 40, "Nr of blocks to try to preallocate", sb->s_prealloc_blocks);			
	EXT2_DBG("%-*s%d\n", 40, "Nr to preallocate for dirs", sb->s_prealloc_dir_blocks);		
}

/**
 * 描述：打印inode信息
 */
static void dump_inode(struct ext2_inode *inode)
{
	int i;

	EXT2_DBG("Inode info:\n");
	EXT2_DBG("%-*s0x%x\n", 40, "File mode", inode->i_mode);
	EXT2_DBG("%-*s0x%x\n", 40, "Low 16 bits of Owner Uid", inode->i_uid);
	EXT2_DBG("%-*s%u\n", 40, "Size in bytes", inode->i_size);
	EXT2_DBG("%-*s%u\n", 40, "Access time", inode->i_atime);
	EXT2_DBG("%-*s%u\n", 40, "Creation time", inode->i_ctime);
	EXT2_DBG("%-*s%u\n", 40, "Modification time", inode->i_mtime);
	EXT2_DBG("%-*s%u\n", 40, "Deletion Time", inode->i_dtime);
	EXT2_DBG("%-*s0x%x\n", 40, "Low 16 bits of Group Id", inode->i_gid);
	EXT2_DBG("%-*s%hu\n", 40, "Links count", inode->i_links_count);
	EXT2_DBG("%-*s%u\n", 40, "Blocks count", inode->i_blocks);	
	EXT2_DBG("%-*s0x%x\n", 40, "File flags", inode->i_flags);
	EXT2_DBG("%-*s\n", 40, "Pointers to blocks");
	for(i = 0; i < EXT2_N_BLOCKS; ++i)
		EXT2_DBG(" %-s%-*0d%u\n", "block index ", 27, i, inode->i_block[i]);
	EXT2_DBG("%-*s0x%x\n", 40, "File version (for NFS)", inode->i_generation);
	EXT2_DBG("%-*s0x%x\n", 40, "File ACL", inode->i_file_acl);
	EXT2_DBG("%-*s0x%x\n", 40, "Directory ACL", inode->i_dir_acl);
	EXT2_DBG("%-*s0x%x\n", 40, "Fragment address", inode->i_faddr);
}

/**
 * 描述：打印块组描述符信息
 */
static void dump_grpdesc(struct ext2_group_desc *grpdesc)
{
	EXT2_DBG("Group descriptor info:\n");
	EXT2_DBG("%-*s%u\n", 40, "Blocks bitmap block", grpdesc->bg_block_bitmap); 
	EXT2_DBG("%-*s%u\n", 40, "Inodes bitmap block", grpdesc->bg_inode_bitmap);
	EXT2_DBG("%-*s%u\n", 40, "Inodes table block", grpdesc->bg_inode_table);
	EXT2_DBG("%-*s%hu\n", 40, "Free blocks count", grpdesc->bg_free_blocks_count); 
	EXT2_DBG("%-*s%hu\n", 40, "Free inodes count", grpdesc->bg_free_inodes_count);
	EXT2_DBG("%-*s%hu\n", 40, "Directories count", grpdesc->bg_used_dirs_count);
}

/**
 * 描述：打印目录子条目信息
 */
static void dump_dir_entry(struct ext2_dir_entry *ent)
{
	char *ftstr;
	char filename[EXT2_NAME_LEN + 1];

	EXT2_DBG("Dir-entry info:\n");
	EXT2_DBG("%-*s%u\n", 40, "Inode number", ent->inode); 
	EXT2_DBG("%-*s%hu\n", 40, "Directory entry length", ent->rec_len);
	EXT2_DBG("%-*s%d\n", 40, "Name length", ent->name_len);
	switch(ent->file_type) {
	case EXT2_FT_UNKNOWN:
		ftstr = "unkown";
		break;
	case EXT2_FT_REG_FILE:	
		ftstr = "regular file";
		break;
	case EXT2_FT_DIR:
		ftstr = "directory";
		break;
	case EXT2_FT_CHRDEV:
		ftstr = "char device";
		break;
	case EXT2_FT_BLKDEV:	
		ftstr = "block device";
		break;
	case EXT2_FT_FIFO:			
		ftstr = "fifo";
		break;
	case EXT2_FT_SOCK:				
		ftstr = "socket";
		break;
	case EXT2_FT_SYMLINK:				
		ftstr = "symbol link";
		break;
	}
	EXT2_DBG("%-*s%s\n", 40, "File type", ftstr);
	/* 注意：ent->name可能没有末尾0 */
	strncpy(filename, ent->name, ent->name_len);
	filename[ent->name_len] = 0;
	EXT2_DBG("%-*s%s\n", 40, "Sub file name", filename);
}
#endif

/**
 * 描述：读取超级块信息
 */
static int ext2_read_super_block(struct ext2_file *ef, struct ext2_super_block *sb)
{
	uint8_t buf[SZ_1K];
	struct block_file *bf = ef->blkfile;
	/* 超级块固定在起始块1K的偏移位置 */
	loff_t off = (loff_t)(ef->start_sect) * SECTOR_SZ + SZ_1K;
	
	bf->bf_ops->bfo_lseek(bf, off, SEEK_SET);
	
	if(bf->bf_ops->bfo_read(bf, buf, SZ_1K) != SZ_1K)
		return -1;
	
	memcpy((uint8_t *)sb, buf, sizeof(struct ext2_super_block));
#if EXT2_DEBUG
	dump_super_block(sb);
#endif

	return 0;
}

/**
 * 描述：根据inode号找到对用的inode结构
 */
static int ext2_get_inode_by_number(struct ext2_file *ef, ulong ino, struct ext2_inode **indp)
{
	loff_t off;
	ulong grp_idx, grp_blk, grp_offs;
	ulong inode_blk, inode_offs;
	struct block_file *bf = ef->blkfile; 	
	struct ext2_group_desc grpdesc;
	struct ext2_inode *inode = NULL;
	
	*indp = NULL;	
	/* 根据inode号找到对应的组描述符 */
	grp_idx = (ino - 1) / ef->info.i_inodes_per_group;	//获取组号
	grp_blk = grp_idx / ef->info.i_grpdes_per_block;	//获取对应组描述符所在的数据块位置
	grp_offs = grp_idx % ef->info.i_grpdes_per_block;	//获取对应组描述符所在的块内偏移

	/* GDT位于超级块后紧接着的下一个block */
	grp_blk += SZ_1K / ef->info.i_blk_sz + 1;
	off = (loff_t)(ef->start_sect) * SECTOR_SZ;
	off += (loff_t)grp_blk * ef->info.i_blk_sz + (loff_t)grp_offs * ef->info.i_grpdes_sz;
	bf->bf_ops->bfo_lseek(bf, off, SEEK_SET);
	if(bf->bf_ops->bfo_read(bf, (uint8_t *)&grpdesc, ef->info.i_grpdes_sz) != ef->info.i_grpdes_sz)
		return -1;
	
#if EXT2_DEBUG
		dump_grpdesc(&grpdesc);
#endif

	/* 计算inode在inode表中的偏移字节 */
	inode_offs = ((ino - 1) % ef->info.i_inodes_per_group) * ef->info.i_inodes_sz;
	/* 计算inode所在的数据块 */
	inode_blk = grpdesc.bg_inode_table + inode_offs / ef->info.i_blk_sz;	
	inode_offs %= ef->info.i_blk_sz;
	off = (loff_t)(ef->start_sect) * SECTOR_SZ;
	off += (loff_t)inode_blk * ef->info.i_blk_sz + inode_offs;

	if(!(inode = (struct ext2_inode *)malloc(sizeof(struct ext2_inode)))) {
		errno = ENOMEM;
		return -1;
	}

	bf->bf_ops->bfo_lseek(bf, off, SEEK_SET);
	if(bf->bf_ops->bfo_read(bf, (uint8_t *)inode, sizeof(*inode)) != sizeof(*inode)) {
		free(inode);
		return -1;
	}

#if EXT2_DEBUG
		dump_inode(inode);
#endif
		
	*indp = inode;

	return 0;
}

/**
 * @PMON
 * 描述：根据传入的block数组以及索引读取指定文件数据 
 * 注意：该函数idx_addr所指向的块，必须为数据块，而不能是间接索引块
 */
static int ext2_read_data_blk_by_index(struct ext2_file *ef, ulong start_idx, ulong end_idx, 
				u32 *idx_addr, uint8_t **buf, size_t *len, loff_t *pos, struct ext2_inode *inode)
{
	size_t resid_size = 0;			//剩下还有多少个字节需要一次性读完
	ulong resid_cnt = 0;			//剩下还有多少个块需要一次性读完
	loff_t off;
	struct block_file *bf = ef->blkfile; 	

	if(start_idx > end_idx)
		return 0;
	idx_addr += start_idx;

	resid_size = ef->info.i_blk_sz - (ulong)(*pos & (ef->info.i_blk_sz - 1));
	if(resid_size > *len) {
		resid_cnt = 0;
		resid_size = *len;
	} else if(resid_size == ef->info.i_blk_sz) {//pos对齐block边界
		resid_cnt = 0;
		resid_size = 0;
	} else {		//resid_size <= *len, pos未对齐block边界
		resid_cnt = 1;
	}

	off = (loff_t)ef->start_sect * SECTOR_SZ + (loff_t)(*idx_addr) * ef->info.i_blk_sz + ((*pos) & (ef->info.i_blk_sz - 1));
	/* 读去非块边界对齐数据，将非对齐访问转换为对齐访问 */
	if(resid_cnt) {
		bf->bf_ops->bfo_lseek(bf, off, SEEK_SET);
		if(bf->bf_ops->bfo_read(bf, *buf, resid_size) != resid_size) {
			return -1;
		}
		idx_addr += resid_cnt;
		start_idx += resid_cnt;
		*buf += resid_size;
		*pos += resid_size;
		*len -= resid_size;
		resid_cnt = 0;
		resid_size = 0;
		off = (loff_t)ef->start_sect * SECTOR_SZ + (loff_t)(*idx_addr) * ef->info.i_blk_sz;
	}
	/* 对齐访问 */
	while(*len && resid_size < *len && start_idx + resid_cnt <= end_idx) {
		if(start_idx + resid_cnt < end_idx &&
				idx_addr[resid_cnt] + 1 == idx_addr[resid_cnt + 1]) {
			if(resid_size + ef->info.i_blk_sz > *len) {
				resid_size = *len;
			} else {
				resid_size += ef->info.i_blk_sz;
				++resid_cnt;
			}
		} else {
			if(!resid_size) {
				if(ef->info.i_blk_sz > *len) {
					resid_size = *len;
				} else {
					resid_size = ef->info.i_blk_sz;
					resid_cnt = 1;
				}
			}
			bf->bf_ops->bfo_lseek(bf, off, SEEK_SET);
			if(bf->bf_ops->bfo_read(bf, *buf, resid_size) != resid_size) {
				return -1;
			}
			idx_addr += resid_cnt;
			start_idx += resid_cnt;
			*buf += resid_size;
			*pos += resid_size;
			*len -= resid_size;
			resid_cnt = 0;
			resid_size = 0;
			off = (loff_t)ef->start_sect * SECTOR_SZ + (loff_t)(*idx_addr) * ef->info.i_blk_sz;
		}
	}

	if(!*len)
		return 0;

	if(resid_size) {
		bf->bf_ops->bfo_lseek(bf, off, SEEK_SET);
		if(bf->bf_ops->bfo_read(bf, *buf, resid_size) != resid_size) {
			return -1;
		}
		*buf += resid_size;
		*pos += resid_size;
		*len -= resid_size;
	}

	return 0;
}

/**
 * 描述：读取指定inode中特定长度的数据
 * 注意：该函数刻意避开了对64位的取摩以及除法运算，取而代之为位运算，但是这里要求文件系
 * 		 统的块大小为2的幂，目前来说，ext2标准符合该要求
 * 返回：函数对于IO物理错误，将返回-1，而对于内存不足，文件过大等错误返回已经读取字节数
 */ 
static ssize_t ext2_read_data_blk(struct ext2_file *ef, uint8_t *buf0, size_t len0, loff_t pos0, 
				struct ext2_inode *inode)
{
	ulong i;
	uint8_t *buf = buf0; 
	size_t len = len0;
	loff_t pos = pos0, off;
	uint32_t *idx_buf = NULL, *idx2_buf = NULL;
	struct block_file *bf = ef->blkfile; 	

	/* DIR index 0 - 11 */
	if(ext2_read_data_blk_by_index(ef, (ulong)(pos >> ef->info.i_log_blk_sz), EXT2_NDIR_BLOCKS - 1, 
			inode->i_block, &buf, &len, &pos, inode)) {
		fprintf(stderr, "read DIR block index error\n");
		goto err;
	}

	/* INDIR index 12 */
	if(!len) 
		goto out;
	if(!(idx_buf = (uint32_t *)malloc(ef->info.i_blk_sz))) {
		errno = ENOMEM;
		goto out;
	}
	off = (loff_t)ef->start_sect * SECTOR_SZ + (loff_t)inode->i_block[EXT2_IND_BLOCK] * ef->info.i_blk_sz;
	bf->bf_ops->bfo_lseek(bf, off, SEEK_SET);
	if(bf->bf_ops->bfo_read(bf, (uint8_t *)idx_buf, ef->info.i_blk_sz) != ef->info.i_blk_sz) 
		goto err;
	if(ext2_read_data_blk_by_index(ef, (ulong)(pos >> ef->info.i_log_blk_sz) - EXT2_NDIR_BLOCKS, 
			ef->info.i_blk_sz / sizeof(u32) - 1, idx_buf, &buf, &len, &pos, inode)) {
		fprintf(stderr, "read INDIR block index error\n");
		goto err;
	}

	/* double INDIR index 13 */
	if(!len) 
		goto out;
	if(!(idx2_buf = (uint32_t *)malloc(ef->info.i_blk_sz))) {
		errno = ENOMEM;
		goto out;
	}
	off = (loff_t)ef->start_sect * SECTOR_SZ + (loff_t)inode->i_block[EXT2_DIND_BLOCK] * ef->info.i_blk_sz;
	bf->bf_ops->bfo_lseek(bf, off, SEEK_SET);
	if(bf->bf_ops->bfo_read(bf, (uint8_t *)idx_buf, ef->info.i_blk_sz) != ef->info.i_blk_sz) 
		goto err;
	for(i = 0; len && i < ef->info.i_blk_sz / sizeof(u32); ++i) {
		off = (loff_t)ef->start_sect * SECTOR_SZ + (loff_t)idx_buf[i] * ef->info.i_blk_sz;
		bf->bf_ops->bfo_lseek(bf, off, SEEK_SET);
		if(bf->bf_ops->bfo_read(bf, (uint8_t *)idx2_buf, ef->info.i_blk_sz) != ef->info.i_blk_sz) 
			goto err;
		if(ext2_read_data_blk_by_index(ef, (ulong )(pos >> ef->info.i_log_blk_sz) - 
				EXT2_NDIR_BLOCKS - (i + 1) * (ef->info.i_blk_sz / sizeof(u32)), 
				ef->info.i_blk_sz / sizeof(u32) - 1, idx2_buf, &buf, &len, &pos, inode)) {
			fprintf(stderr, "read doube INDIR block index error\n");
			goto err;
		}
	}
	
	/* third INDIR index 14 (unsupported now) */
	if(len) {
		fprintf(stderr, "3-level INDIR block index not supported now\n");
		errno = EFBIG;
		goto out;	
	}

out:
	if(idx_buf)
		free(idx_buf);
	if(idx2_buf)
		free(idx2_buf);

	return len0 - len;

err:
	if(idx_buf)
		free(idx_buf);
	if(idx2_buf)
		free(idx2_buf);

	return -1;
}

/**
 * 描述：根据文件路径找到对应inode
 */
static int ext2_lookup_inode(struct ext2_file *ef, const char *filepath, struct ext2_inode **indp)
{
	int found = 0;
	struct ext2_inode *inode;	//这里的inode均指向中间的目录文件
	uint8_t *buf = NULL, *cp;
	struct ext2_dir_entry *dir_ent;
	char file[EXT2_NAME_LEN + 1], src[EXT2_NAME_LEN + 1], *fnamep, *fcp;
	ulong ino = EXT2_ROOT_INO;	//从根目录的inode开始进行遍历查找

	fnamep = (char *)filepath;
	/* 过滤打头的斜杠 */
	while('/' == *fnamep)
		++fnamep;
	do {
		/* 查找上级目录的子文件信息 */
		if(ext2_get_inode_by_number(ef, ino, &inode)) {
			fprintf(stderr, "find inode[%d] failed\n", ino);
			return -1;
		}

		/* 以"/"拆分文件路径 */
		for(fcp = fnamep; *fcp && *fcp != '/'; ++fcp)
			;
		if(fcp - fnamep > EXT2_NAME_LEN) {
			errno = ENAMETOOLONG;
			return -1;
		}
		strncpy(file, fnamep, fcp - fnamep);
		file[fcp - fnamep] = 0;
		/* 找到最终目录 */
		if(!file[0]) {
			ef->inode_no = ino;
			*indp = inode;
			return 0;
		}
		fnamep = fcp;

		if(!(buf = (uint8_t *)malloc(inode->i_size))) {	/* buf保存目录的数据信息 */
			free(inode);
			errno = ENOMEM;
			return -1;
		}

		/* 读取目录文件的数据块 */
		if(ext2_read_data_blk(ef, buf, inode->i_size, 0, inode) != inode->i_size) {
			fprintf(stderr, "read inode[%d]'s data block failed\n", ino);
			return -1;
		}

		/* 遍历目录的子文件 */
		found = 0;
		/** 
		 * 注意：之所以可以使用下面判断作为循环退出条件是因为最后一个目录
		 * 		 表项的长度应当为：i_size - 前面目录项的总长，而非实际有效
		 * 		 数据长度
		 */
		for(cp = buf; cp < buf + inode->i_size;) {
			dir_ent = (struct ext2_dir_entry *)cp;
#if EXT2_DEBUG
			dump_dir_entry(dir_ent);
#endif
			/* 正常情况下，下面两个长度应不为0，对于lost+found目录，下面两个值可能会为0 */
			if(!dir_ent->rec_len || !dir_ent->name_len) {
				fprintf(stderr, "unexpected DIR info\n");
				break;
			}
			/* 查看当前文件名是否匹配 */
			strncpy(src, dir_ent->name, dir_ent->name_len);
			src[dir_ent->name_len] = 0;
			if(!strcmp(file, src)) {
				if(EXT2_FT_REG_FILE ==dir_ent->file_type) {
					free(buf); free(inode);
					if('/' == *fnamep) {
						errno = ENOTDIR;
						return -1;
					}
					if(ext2_get_inode_by_number(ef, dir_ent->inode, &inode)) {
						fprintf(stderr, "find inode[%d] failed\n", ino);
						return -1;
					}
					/* 找到最终文件 */
					ef->inode_no = dir_ent->inode;
					*indp = inode;
					return 0;
				}
				/* 目录文件 */
				found = 1;
				ino = dir_ent->inode;
				while('/' == *fnamep)
					++fnamep;
				break;
			}
			cp += dir_ent->rec_len;
		}
		free(buf); free(inode);
	} while(found); 
	
	/* 文件未找到 */
	errno = ENOENT;

	return -1;
}

static void ext2_setup_super_info(struct ext2_super_info *info, struct ext2_super_block *superblk)
{
	info->i_blk_sz = SZ_1K << superblk->s_log_block_size;	
	info->i_log_blk_sz = LOG_1K + superblk->s_log_block_size;	
	info->i_inodes_per_group = superblk->s_inodes_per_group;
	info->i_inodes_sz = superblk->s_inode_size;
	if(superblk->s_feature_incompat & EXT2_FEAT_INCOMP_64BIT)
		info->i_grpdes_sz = 64;
	else
		info->i_grpdes_sz = 32;
	info->i_grpdes_per_block = info->i_blk_sz / info->i_grpdes_sz;
}

static int ext2_open(struct fsio_file *ff, const char *path, int flags)
{
	int accmode = flags & O_ACCMODE, part = -1;
	struct ext2_file *ef = NULL;
	char *devname, *filename, *cp;
	struct block_file *bf = NULL;
	struct partition_info partitions[PART_INFO_MAX];
	struct ext2_super_block superblk;
	
	//目前暂不支持写或者创建文件
	if(accmode != O_RDONLY) {
		errno = EACCES;
		return -1;
	}
	
	if(!(ef = (struct ext2_file *)malloc(sizeof(struct ext2_file)))) {
		errno = ENOMEM;
		return -1;
	}
	memset(ef, 0, sizeof(struct ext2_file));
	ef->oflags = flags;
	ef->foffs = 0;
	
	/* 初始化文件所在块设备文件 */
	if(!(bf = (struct block_file *)malloc(sizeof(struct block_file)))) {
		errno = ENOMEM;	
		goto failed;
	}

	/* 解析设备名，分区号与文件路径名 */
	devname = (char *)(path);
	for(cp = devname; *cp && *cp != '/'; ++cp) 
		;
	if(!*cp) {
		/* 未指定设备名 */
		fprintf(stderr, "missing disk device\n");
		errno = EINVAL;	
		goto failed;
	}
	//块设备名 + “-” + 分区号 bad code
	if((cp - devname) >= BLK_FILENAME_MAX) {
		/* 设备名过长 */
		fprintf(stderr, "disk device name too long, max %d\n", BLK_FILENAME_MAX);
		errno = ENAMETOOLONG;
		goto failed;
	}
	filename = ++cp;
	for(cp = devname; (cp < filename - 1) && (*cp != '-'); ++cp) 
		;
	strncpy(bf->bf_name, devname, cp - devname);
	bf->bf_name[cp - devname] = 0;
	if('-' == *cp) {
		if(*++cp > '0' + PART_INFO_MAX || *cp < '1') {
			fprintf(stderr, "disk partition number invalid, %d\n", *cp - '1');
			errno = EINVAL;
			goto failed;
		}
		part = *cp - '1';
	}
	EXT2_DBG("%s: open device[%s], partition[%c], file[%s].\n", 
			__func__, bf->bf_name, part >= 0 ? part + '1' : 'x', filename);
	bf->bf_ops = &blkops;
	ef->blkfile = bf;
	
	if(bf->bf_ops->bfo_open) {
		if((errno = bf->bf_ops->bfo_open(bf, flags)) != 0) 
			goto failed;
	} else {
		errno = EINVAL;
		goto failed;
	}

	/* 获取文件所在分区的起始块位置 */
	if(errno = bf->bf_ops->bfo_ioctl(bf, BIOCGPARTINF, partitions) && errno != ENXIO) {
		fprintf(stderr, "read partition failed\n");
		goto close_blk;;
	}
	if(!errno) {
		if(part != -1) {
			/* 硬盘有分区表，并且路径指定了分区号 */
			if(partitions[part].pi_valid) {
				if(partitions[part].pi_sysid != 0x83) {
					errno = EINVAL;
					fprintf(stderr, "partition'format is not Linux, 0x%x\n", 
									partitions[part].pi_sysid);
					goto close_blk;;
				}
				ef->start_sect = partitions[part].pi_start;
			} else {
				errno = EINVAL;
				fprintf(stderr, "partition %d no exist\n", part);
				goto close_blk;;
			}
		} else {
			/* 硬盘有分区表，而路径没有指定分区号，则默认使用分区0的起始块 */
			if(partitions[0].pi_valid) {
				if(partitions[0].pi_sysid != 0x83) {
					errno = EINVAL;
					fprintf(stderr, "default partition'format is not Linux, 0x%x\n", 
									partitions[0].pi_sysid);
					goto close_blk;;
				}
				ef->start_sect = partitions[0].pi_start;
			} else {
				/* 硬盘有分区表，而路径没有指定分区号，且分区0不存在，则使用硬盘的块0作为起始块 */
				ef->start_sect = 0; 
			}
		}
	} else if(-1 == part) {
		/* 硬盘没有分区信息，并且路径未指定分区号，则使用硬盘的块0作为起始块 */
		ef->start_sect = 0;
	} else {
		/* 硬盘没有分区信息，而路径指定了分区号 */
		errno = EINVAL;
		fprintf(stderr, "partition %d no exist\n", part);
		goto close_blk;;
	}
	EXT2_DBG("%s: file start block: %u.\n", __func__, ef->start_sect);

	if(ext2_read_super_block(ef, &superblk)) {
		fprintf(stderr, "read super block error\n");
		goto close_blk;;
	}
	if(superblk.s_magic != EXT2_MAGIC) {
		errno = EIO;
		fprintf(stderr, "super block magic error, 0x%x\n", superblk.s_magic);
		goto close_blk;;
	}

	if(superblk.s_feature_incompat & EXT2_FEAT_INCOMP_EXTENTS) {
		errno = EOPNOTSUPP;
		fprintf(stderr, "ext4 filesystem not supported\n");
		goto close_blk;
	}
	ext2_setup_super_info(&ef->info, &superblk);
	if(64 == ef->info.i_grpdes_sz) {
		errno = EOPNOTSUPP;
		fprintf(stderr, "group descriptor size 64bit not supported\n");
		goto close_blk;
	}

	/* 查找指定文件路径的inode */
	if(ext2_lookup_inode(ef, filename, &ef->inode)) 
		goto close_blk;;

	ff->pri = ef;

	return 0;

close_blk:
	if(bf->bf_ops->bfo_close)
		bf->bf_ops->bfo_close(bf);
failed:
	if(bf)
		free(bf);
	if(ef)
		free(ef);

	return -1;
}

static int ext2_close(struct fsio_file *ff)
{
	struct ext2_file *ef = (struct ext2_file *)ff->pri;
	struct block_file *bf = ef->blkfile;

	EXT2_DBG("%s: close file.\n", __func__);
	
	if(ef->inode)
		free(ef->inode);

	if(bf->bf_ops->bfo_close)
		bf->bf_ops->bfo_close(bf);
	
	free(bf);
	free(ef);
	ff->pri = NULL;

	return 0;
}

static ssize_t ext2_read(struct fsio_file *ff, void *buf, size_t len)
{
	size_t res;
	struct ext2_file *ef = (struct ext2_file *)ff->pri;
	size_t offs = (size_t)ef->foffs;
	struct ext2_inode *inode = ef->inode;

	if(O_WRONLY == (ef->oflags & O_ACCMODE)) {
		errno = EPERM;
		return -1;
	}

	/* 不支持直接读目录文件 */
	if(S_ISDIR(inode->i_mode)) {	
		errno = EISDIR;
		return -1;
	}

	/* bad code，目前文件支持64位偏移，然后inode节点却仅支持32位大小 */
	if(len + offs > ef->inode->i_size)	
		len = ef->inode->i_size - offs;
		
	if((res = ext2_read_data_blk(ef, buf, len, (loff_t)offs, inode)) < 0) 
		return -1;

	ef->foffs += (loff_t)res;

	return res;
}

static ssize_t ext2_write(struct fsio_file *ff, const void *buf, size_t len)
{
	struct ext2_file *ef = (struct ext2_file *)ff->pri;

	if(O_RDONLY == (ef->oflags & O_ACCMODE)) {
		errno = EPERM;
		return -1;
	}
	
	errno = EOPNOTSUPP;			//目前暂不支持写或者创建文件

	return -1;
}

static off_t ext2_lseek(struct fsio_file *ff, off_t off, int whence)
{
	loff_t noff;
	struct ext2_file *ef = (struct ext2_file *)ff->pri;

	switch(whence) {
		case SEEK_CUR:
			noff = ef->foffs + off;
			break;
		case SEEK_SET:
			noff = off;
			break;
		case SEEK_END:
		default:
			errno = EPERM;
			return -1;
	}

	ef->foffs = noff;

	return noff;
}

static off_t ext2_llseek(struct fsio_file *ff, loff_t off, int whence)
{
	loff_t noff;
	struct ext2_file *ef = (struct ext2_file *)ff->pri;

	switch(whence) {
		case SEEK_CUR:
			noff = ef->foffs + off;
			break;
		case SEEK_SET:
			noff = off;
			break;
		case SEEK_END:
		default:
			errno = EPERM;
			return -1;
	}

	ef->foffs = noff;

	return noff;
}

static int ext2_ioctl(struct fsio_file *ff, unsigned long cmd, void *data)
{
	ulong idx = 0;
	struct stat *st;
	struct dirent_info *di;
	struct ext2_file *ef = (struct ext2_file *)ff->pri;
	struct ext2_inode *inode = ef->inode;
	uint8_t *buf = NULL, *cp;
	struct ext2_dir_entry *dir_ent;

	switch(cmd) {
	case EIOCGFSTAT:
		st = (struct stat *)data;
		if(!st) {
			errno = EINVAL;	
			return -1;
		}
		st->st_mode = inode->i_mode;
		st->st_ino = ef->inode_no;				
		st->st_dev = 0;					/* 暂不支持设备号 */
		st->st_rdev = 0;	
		st->st_nlink = inode->i_links_count;
		st->st_uid = inode->i_uid;		/* 这里的组ID和用户ID仅显示低16位 */
		st->st_gid = inode->i_gid;			
		st->st_size = inode->i_size;
		st->st_atime = inode->i_atime;
		st->st_mtime = inode->i_mtime;
		st->st_ctime = inode->i_ctime;
		st->st_blksize = ef->info.i_blk_sz;	
		st->st_blocks = inode->i_blocks;	
		break;
	case EIOCGDIRENT:
		di = (struct dirent_info *)data;
		if(!S_ISDIR(inode->i_mode)) {	
			errno = EINVAL;
			return -1;
		}
		if(!(buf = (uint8_t *)malloc(inode->i_size))) {
			errno = ENOMEM;
			return -1;
		}
		/* 读取目录文件的数据块 */
		if(ext2_read_data_blk(ef, buf, inode->i_size, 0, inode) != inode->i_size) {
			free(buf);
			return -1;
		}
		/** 
		 * 注意：之所以可以使用下面判断作为循环退出条件是因为最后一个目录
		 * 		 表项的长度应当为：i_size - 前面目录项的总长，而非实际有效
		 * 		 数据长度
		 */
		for(cp = buf; cp < buf + inode->i_size;) {
			dir_ent = (struct ext2_dir_entry *)cp;
			/* 正常情况下，下面两个长度应不为0，对于lost+found目录，下面两个值可能会为0 */
			if(!dir_ent->rec_len || !dir_ent->name_len) {
				fprintf(stderr, "unexpected DIR info\n");
				break;
			}
			if(idx == di->dir_idx) {
				strncpy(di->dir_name, dir_ent->name, dir_ent->name_len);
				di->dir_name[dir_ent->name_len] = 0;
				/* 找到对应目录项时，该字段将加1，用于判定是否到达文件末尾 */
				++di->dir_idx;
				break;
			}
			cp += dir_ent->rec_len;
			++idx;
		}
		free(buf);
		break;
	default:
		errno = ENOTTY;
		return -1;
	}
	
	return 0;
}

static struct fsio_ops ext2_ops = {
	.fio_open = ext2_open,
	.fio_close = ext2_close,
	.fio_read = ext2_read,
	.fio_write = ext2_write,
	.fio_lseek = ext2_lseek,
	.fio_llseek = ext2_llseek,
	.fio_ioctl = ext2_ioctl,
};

static struct fsio ext2_fio = {
	.fsname = "ext2",
	.ops = &ext2_ops,
};

/**
 * 描述：由于该函数处于ctor段，因此该函数将在__init函数中被调用
 */
static void __attribute__((constructor)) ext2_init(void)
{
	fsio_register(&ext2_fio);
}

