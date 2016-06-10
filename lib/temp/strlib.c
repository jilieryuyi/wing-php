#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//字符串查找 查找首次出现的位置
int strpos(char *source,char *find){
	char *start = source;
	while(*start!='\0'){
		char *temp = start;
		char *find_start =find;
		int is_find = 1;
		while(*find_start!='\0'){
			if(*find_start!=*temp){
				is_find = 0;
				break;
			}
			temp++;
			find_start++;	
		}
		
		if(is_find)return start-source-1;
		start++;
	}
	return -1;
}
//字符串反转
void strrev(char *source,char *back){
	char *end = source+strlen(source)-1;
	while(end!=source){
		*back++ = *end;
		end--;
	}
	*back = *source;
}
//字符串替换
void str_replace(char *source,char *find,char *replace,char *back,int max_replace=0){
	
	char *start = source;
	int replace_times = 0;
	char *s_r=0;
	char *temp=0;
	char *find_start=0;
	int is_find = 1;

	while(*start!='\0'){
		temp = start;
		find_start =find;
		is_find = 1;

		while(*find_start!='\0'){
			if(*find_start!=*temp){
				is_find = 0;
				break;
			}
			temp++;
			find_start++;	
		}
		
		if(((replace_times < max_replace)|| max_replace<=0 ) && is_find ){
					s_r = replace;
					while (*s_r!='\0')
					{
						*back++=*s_r++;
					}
					start+=strlen(find);
					replace_times++;
				
		}else{
			*back++=*start++;
		}
	}
}

//字符串格式化
void str_format(char *source,char *target,char *format_str=",",int format_num=3){
	int len = strlen(source);
	int start = len%format_num;
	int i=start,count=1;
	char *st = source,*ft=format_str;
	for(i=start;i<=len;i+=format_num){
		ft=format_str;
		while(st<(source+i))*target++=*st++;
		while(*ft!='\0'&&*st!='\0'&&i!=0)*target++=*ft++;
	}	
}