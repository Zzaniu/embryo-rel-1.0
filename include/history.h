
/**
 * Copyright(c) 2016-8-28 Shangwen Wu	
 *
 * 历史记录命令实现头文件
 * 
 */

#ifndef __HISTORY_H__
#define __HISTORY_H__

#define HIST_MAX_NUM	256

#define HIST_NEXT(n)	((n)+1>=HIST_MAX_NUM?0:(n)+1) //下一个位置
#define HIST_LAST(n)	((n)-1<0?HIST_MAX_NUM-1:(n)-1)	//上一个位置

extern int add_hist_cmd(const char *cmd);
extern char *get_last_cmd(void);
extern char *read_last_cmd(void);
extern char *get_next_cmd(void);
extern char *read_next_cmd(void);
extern int pos_is_latest(void);
extern int pos_is_new(void);
extern void pos_set_new(void);
extern char *find_bang(const char *str);
extern char *find_hist_by_prefix(const char *pfx);
extern char *find_hist_by_num(int num);

#endif //__HISTORY_H__

