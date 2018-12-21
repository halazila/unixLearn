#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#define	MAXFILELEN 1000 // max filename length 
#define MAXLINELEN 1000 // max char number in a line

char line[MAXLINELEN];

struct node{
	char filename[MAXFILELEN]; 
	struct node *pnext;
};
struct nlink{
	struct node *phead;
	struct node *ptail;
};

void link_pop(struct nlink *plink);
void link_push(struct nlink *plink, struct node *pnode);
void *find_str(char *fname, const char *str);

int
main(int argc, char *argv[])
{
	char *ret;
	DIR *dirp;
	struct dirent *dp;
	char *path;
	char *dstr;
	struct stat *pstat;
	char filename[MAXFILELEN];
	struct node *pnode = NULL;
	struct nlink *plink = NULL;

	if(argc !=3 || strcmp(argv[1], "--help") == 0){
		printf("Usage: %s string file|path, find [string] in [file|path], if found, print file name and the first matched line in file.\n", argv[0]);
		exit(2);
	}
	path = (char*)malloc(strlen(argv[2])*sizeof(char));
	dstr = (char*)malloc(strlen(argv[1])*sizeof(char));
	pstat = malloc(sizeof(struct stat));
	strcpy(path, argv[2]);
	strcpy(dstr, argv[1]);
	printf("target string:%s; source file:%s\n", dstr, path);
	
	pnode = malloc(sizeof(struct node));
	strcpy(pnode->filename, path);
	pnode->pnext = NULL;
	plink = malloc(sizeof(struct nlink));
	plink->ptail = (plink->phead = NULL);
	link_push(plink, pnode);
	while(plink->phead != NULL){
		strcpy(filename, plink->phead->filename);
		link_pop(plink);
		errno = 0;
		if(lstat(filename, pstat) < 0)
			printf("lstat() %s error:%s\n", filename, strerror(errno));
		else{
			if(S_ISREG(pstat->st_mode)){
				ret = (char*)find_str(filename, dstr);
				if(ret != NULL)
					printf("file:%s, line:%s\n", filename, ret);
			}
			else if(S_ISDIR(pstat->st_mode)){
				errno = 0;
				dirp = opendir(filename);
				if(dirp == NULL)
					printf("opendir() %s error:%s\n", filename, strerror(errno));
				else{
					char tmp[MAXFILELEN];
					memset(tmp, '\0', MAXFILELEN);
					while((dp = readdir(dirp)) != NULL){
						if(strcmp(dp->d_name,".") == 0 || strcmp(dp->d_name, "..") == 0)
							continue;
						strcpy(tmp, filename);
						if(!(*(tmp + strlen(tmp) - 1) == '/'))
							strcat(tmp, "/");
						strcat(tmp, dp->d_name);
						struct node *tmpnode;
						tmpnode = malloc(sizeof(struct node));
						strcpy(tmpnode->filename, tmp);
						tmpnode->pnext = NULL;
						link_push(plink, tmpnode);
					}
				}
				closedir(dirp);
			}
			else 
				printf("no supporting file:%s\n", filename);
		}
	}

	free(pstat);	
	free(path);
	free(dstr);
	free(plink);
	exit(0);
}

void *find_str(char *fname, const char *str){
	FILE *fp;
	errno = 0;
	if((fp = fopen(fname, "r")) == NULL){
		printf("open %s error:%s\n", fname, strerror(errno));
		return (void*)NULL;
	}
	while(fgets(line, MAXLINELEN, fp) != NULL){
		if(strstr(line, str) == NULL)
			continue;
		else {
			fclose(fp);
			return line;
		}
	}
	fclose(fp);
	return (void*)NULL;
}

void link_push(struct nlink *plink, struct node *pnode){
	if(pnode != NULL){
		if(plink == NULL){
			plink = malloc(sizeof(struct nlink));
			plink->phead = pnode;
			plink->ptail = pnode;
		}
		else{
			if(plink->ptail == NULL)
				plink->phead = (plink->ptail = pnode);
			else
				plink->ptail = (plink->ptail->pnext = pnode);
		}
	}
}

void link_pop(struct nlink *plink){
	if(plink != NULL){
		if(plink->phead != NULL){
			struct node *p = plink->phead;
			plink->phead = plink->phead->pnext;
			if(plink->ptail == p)
				plink->ptail = plink->phead;
			free(p);
		}
	}
}

