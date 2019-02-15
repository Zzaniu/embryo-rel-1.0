
/**
 * Copyright(c) 2018-11-27 @PMON Shangwen Wu	
 *
 * EXT2文件系统相关定义
 * 
 */
#ifndef __FS_EXT2_H__
#define __FS_EXT2_H__

/* inode节点的块索引，前面12个为直接索引，后面三个分别为1、2、3级简介索引 */
#define	EXT2_NDIR_BLOCKS		12
#define	EXT2_IND_BLOCK			EXT2_NDIR_BLOCKS
#define	EXT2_DIND_BLOCK			(EXT2_IND_BLOCK + 1)
#define	EXT2_TIND_BLOCK			(EXT2_DIND_BLOCK + 1)
#define	EXT2_N_BLOCKS			(EXT2_TIND_BLOCK + 1)
#define EXT2_NAME_LEN 			255	/* ext2文件名的最大长度 */
#define EXT2_ROOT_INO			2	/* 根目录的inode节点号 */

#define EXT2_FEAT_INCOMP_META_BG	0x10
#define EXT2_FEAT_INCOMP_EXTENTS	0x40
#define EXT2_FEAT_INCOMP_64BIT		0x80
#define EXT2_FEAT_INCOMP_FLEX_BG	0x200
#define EXT2_FEAT_INCOMP_EA_INODE	0x400

#define EXT2_MAGIC				0xef53

/* 文件类型 */
enum {
	EXT2_FT_UNKNOWN,
	EXT2_FT_REG_FILE,				/* 1: 一般文件 */
	EXT2_FT_DIR,					/* 2: 目录文件 */
	EXT2_FT_CHRDEV,					/* 3: 字符设备 */
	EXT2_FT_BLKDEV,					/* 4: 块设备 */
	EXT2_FT_FIFO,					/* 5: FIFO */
	EXT2_FT_SOCK,					/* 6: socket */
	EXT2_FT_SYMLINK,				/* 7: 软链接 */
	EXT2_FT_MAX
};

/*
 * super block结构定义
 */
struct ext2_super_block {
	u32	s_inodes_count;				/* Inodes count */
	u32	s_blocks_count;				/* Blocks count */
	u32	s_r_blocks_count;			/* Reserved blocks count */
	u32	s_free_blocks_count;		/* Free blocks count */
	u32	s_free_inodes_count;		/* Free inodes count */
	u32	s_first_data_block;			/* First Data Block */
	u32	s_log_block_size;  			/* Block size */
	s32	s_log_frag_size;   			/* Fragment size */
	u32	s_blocks_per_group;			/* # Blocks per group */
	u32	s_frags_per_group; 			/* # Fragments per group */
	u32	s_inodes_per_group;			/* # Inodes per group */
	u32	s_mtime;					/* Mount time */
	u32	s_wtime;					/* Write time */
	u16	s_mnt_count;				/* Mount count */
	s16	s_max_mnt_count;			/* Maximal mount count */
	u16	s_magic;					/* Magic signature */
	u16	s_state;					/* File system state */
	u16	s_errors;					/* Behaviour when detecting errors */
	u16	s_minor_rev_level; 			/* minor revision level */
	u32	s_lastcheck;				/* time of last check */
	u32	s_checkinterval;			/* max. time between checks */
	u32	s_creator_os;				/* OS */
	u32	s_rev_level;				/* Revision level */
	u16	s_def_resuid;				/* Default uid for reserved blocks */
	u16	s_def_resgid;				/* Default gid for reserved blocks */
	/*
	 * These fields are for EXT2_DYNAMIC_REV superblocks only.
	 *
	 * Note: the difference between the compatible feature set and
	 * the incompatible feature set is that if there is a bit set
	 * in the incompatible feature set that the kernel doesn't
	 * know about, it should refuse to mount the filesystem.
	 *
	 * e2fsck's requirements are more strict; if it doesn't know
	 * about a feature in either the compatible or incompatible
	 * feature set, it must abort and not try to meddle with
	 * things it doesn't understand...
	 */
	u32	s_first_ino; 				/* First non-reserved inode */
	u16 s_inode_size; 				/* size of inode structure */
	u16	s_block_group_nr; 			/* block group # of this superblock */
	u32	s_feature_compat; 			/* compatible feature set */
	u32	s_feature_incompat; 		/* incompatible feature set */
	u32	s_feature_ro_compat; 		/* readonly-compatible feature set */
	u8	s_uuid[16];					/* 128-bit uuid for volume */
	char	s_volume_name[16]; 		/* volume name */
	char	s_last_mounted[64]; 	/* directory where last mounted */
	u32	s_algorithm_usage_bitmap;   /* For compression */
	/*
	 * Performance hints.  Directory preallocation should only
	 * happen if the EXT2_COMPAT_PREALLOC flag is on.
	 */
	u8	s_prealloc_blocks;			/* Nr of blocks to try to preallocate*/
	u8	s_prealloc_dir_blocks;		/* Nr to preallocate for dirs */
	u16	s_padding1;
	u32	s_reserved[204];			/* Padding to the end of the block */
};

/*
 * inode结构定义
 */
struct ext2_inode {
	u16	i_mode;						/* File mode */
	u16	i_uid;						/* Low 16 bits of Owner Uid */
	u32	i_size;						/* Size in bytes */
	u32	i_atime;					/* Access time */
	u32	i_ctime;					/* Creation time */
	u32	i_mtime;					/* Modification time */
	u32	i_dtime;					/* Deletion Time */
	u16	i_gid;						/* Low 16 bits of Group Id */
	u16	i_links_count;				/* Links count */
	u32	i_blocks;					/* Blocks count */
	u32	i_flags;					/* File flags */
	union {
		struct {
			u32  l_i_reserved1;
		} linux1;
		struct {
			u32  h_i_translator;
		} hurd1;
		struct {
			u32  m_i_reserved1;
		} masix1;
	} osd1;							/* OS dependent 1 */
	u32	i_block[EXT2_N_BLOCKS];		/* Pointers to blocks */
	u32	i_generation;				/* File version (for NFS) */
	u32	i_file_acl;					/* File ACL */
	u32	i_dir_acl;					/* Directory ACL */
	u32	i_faddr;					/* Fragment address */
	union {
		struct {
			u8	l_i_frag;			/* Fragment number */
			u8	l_i_fsize;			/* Fragment size */
			u16	i_pad1;
			u16	l_i_uid_high;		/* these 2 fields    */
			u16	l_i_gid_high;		/* were reserved2[0] */
			u32	l_i_reserved2;
		} linux2;
		struct {
			u8	h_i_frag;			/* Fragment number */
			u8	h_i_fsize;			/* Fragment size */
			u16	h_i_mode_high;
			u16	h_i_uid_high;
			u16	h_i_gid_high;
			u32	h_i_author;
		} hurd2;
		struct {
			u8	m_i_frag;			/* Fragment number */
			u8	m_i_fsize;			/* Fragment size */
			u16	m_pad1;
			u32	m_i_reserved2[2];
		} masix2;
	} osd2;							/* OS dependent 2 */
};

/*
 * The new version of the directory entry.  Since EXT2 structures are
 * stored in intel byte order, and the name_len field could never be
 * bigger than 255 chars, it's safe to reclaim the extra byte for the
 * file_type field.
 */
struct ext2_dir_entry {
	u32	inode;						/* Inode number */
	u16	rec_len;					/* Directory entry length */
	u8 name_len;					/* Name length */
	u8 file_type;					/* File type */
	char name[EXT2_NAME_LEN];		/* File name 注意：name可能没有末尾0 */
};

/**
 * block组描述符定义
 */
struct ext2_group_desc {
	u32	bg_block_bitmap;			/* Blocks bitmap block */
	u32	bg_inode_bitmap;			/* Inodes bitmap block */
	u32	bg_inode_table;				/* Inodes table block */
	u16	bg_free_blocks_count;		/* Free blocks count */
	u16	bg_free_inodes_count;		/* Free inodes count */
	u16	bg_used_dirs_count;			/* Directories count */
	u16	bg_pad;
	u32	bg_reserved[3];
};

struct block_file;

struct ext2_super_info {
	ulong i_inodes_per_group;		/* 每个块组有多少个inode */
	ulong i_grpdes_per_block;		/* 一个块能容纳多少个组描述符 */
	ulong i_inodes_sz;				/* inode节点大小 */
	ulong i_grpdes_sz;				/* 组描述符大小 */
	ulong i_blk_sz;					/* 块大小 */
	ulong i_log_blk_sz;				/* 块大小(log2) */
};

struct ext2_file {
	struct block_file *blkfile; 	/* 文件所在的块设备 */
	struct ext2_inode *inode;		/* EXT2 inode */
	loff_t foffs;					/* file offset */
	struct ext2_super_info info;	/* 一些与当前文件系统相关的参数信息 */
	ulong start_sect;				/* 文件所在分区的起始扇区 */
	u32 inode_no;					/* inode节点号 */
	int oflags;						/* open flags */
};

#endif //__FS_EXT2_H__

