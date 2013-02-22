// VimMake: CC=(musl-gcc) CFLAGS=(-Wall)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct trie;
typedef struct trie{
   struct trie** children;
   unsigned node_freq, sum_freq;
}trie;
static const size_t factor=26, maxDepth=512;

trie* newTrie(){
   trie* t=(trie*)calloc(1,sizeof(trie));
   if(!(t && (t->children=(trie**)calloc(factor,sizeof(trie*))))){
	perror("newTrie allocation failed"); exit(1);
   }
   return t;
}
void rmTrie(trie* t){
   if(t){
	free(t->children); free(t);
   }
}
static int sanityWord(const char*str){
   while(*str && *str>='a' && *str<='z')++str;
   return !*str;
}
void addWord(trie*t, const char*str){
   if(!(t&&str&&sanityWord(str)))return;
   trie *cur=t,*prev=cur; const char* p=str;
   while(cur&&*p){
	++cur->sum_freq;
	prev=cur; cur=cur->children[*p++-'a'];
   }
   if((*p||!cur)&&p-str)--p;
   while(*p){
	prev=prev->children[*p-'a']=newTrie(*p);
	++prev->sum_freq; ++p;
   }
   ++(cur?cur:prev)->node_freq;
}
void rmWord(trie*t, const char*str){
   if(!(t&&str&&sanityWord(str)))return;
   unsigned ppath=0, len=strlen(str);
   trie *cur=t, *path[len]; path[0]=cur;
   const char* p=str;
   while(cur&&*p)path[ppath++]=cur=cur->children[*p++-'a'];
   if(*p)return;
   for(ppath=0;ppath<len&&--path[ppath]->sum_freq;++ppath);
   while(ppath<len){
	putchar(str[ppath]);fflush(stdout);
	rmTrie(path[ppath++]);
   }
   puts("");
}
void showNodes(const trie*t){	   /* bfs */
   if(!t)return;
   int indx;
   for(indx=0; indx<factor; ++indx)
	if(t->children[indx]){
	   printf("\"%c\"(%u,%u)-:", indx+'a', t->sum_freq,
		   t->node_freq);
	   showNodes(t->children[indx]);
	}
}
void showWords(const trie*t){
   if(!t)return;
   char word[maxDepth+1], path[maxDepth+1];
   memset(path,0,maxDepth+1); memset(word,0,maxDepth+1);
   trie*trace[maxDepth+1]; trace[0]=(trie*)t;
   int depth=0;
   while(depth>=0 && depth<maxDepth){
	while(path[depth]<factor &&
		!trace[depth]->children[(int)path[depth]])
	   ++path[depth];
	if(path[depth]>=factor){   /* regress */
	   path[depth]=0; ++path[--depth];
	}else{
	   word[depth]='a'+path[depth];
	   trace[depth+1]=trace[depth]->children[(int)path[depth]];
	   path[++depth]=0;
	   if(trace[depth]->node_freq){
		word[depth]=0;
		printf("%s: %u\n", word,trace[depth]->node_freq);
	   }
	}
   }
}
void buildTrie(const char*fname,trie*root,void(*action)
	(trie*,const char*)){
   if(!fname)return;
   FILE* fp=fopen(fname,"r");
   if(!fp){
	fprintf(stderr,"Cannot open file %s\n",fname); return;
   }
   size_t sz=maxDepth*2+1;
   /* no realloc by getline should occur on word */
   char cword[sz], *word=cword, word2[maxDepth+1], *beg, *end;
   while(getline(&word,&sz,fp)>=0){
	beg=word;
	while(beg){
	   while(*beg && !(*beg>='a' && *beg<='z'))++beg;
	   if(!*beg)break;
	   end=beg;
	   while(*end && *end>='a' && *end<='z')++end;
	   if(!*end)break;
	   memcpy(word2,beg,end-beg); word2[end-beg]=0;
	   action(root,word2); beg=++end;
	}
   }
   fclose(fp);
}
void destroyTrie(trie*root){
   if(!root)return;
   char path[maxDepth+1];
   memset(path,0,maxDepth+1);
   trie*trace[maxDepth+1]; trace[0]=root;
   int depth=0;
   while(depth>=0 && depth<maxDepth){
	while(path[depth]<factor &&
		!trace[depth]->children[(int)path[depth]])
	   ++path[depth];
	if(path[depth]>=factor){   /* rm node */
	   rmTrie(trace[depth]);
	   path[depth--]=0; ++path[depth];
	}else{
	   trace[depth+1]=trace[depth]->children[(int)path[depth]];
	   path[++depth]=0;
	}
   }
}
void prompt(const trie*root,const char*str){
   if(!root||!str||strlen(str)<3)return;
   const trie*t=root; const char*p=str;
   while(*p&&t)t=t->children[*p++-'a'];
   if(!t)return;
   char word[maxDepth+1], path[maxDepth+1], dif=p-str;
   memcpy(word,str,p-str); memset(path,0,maxDepth+1);
   trie*trace[maxDepth+1]; trace[0]=(trie*)t;
   int depth=0;
   while(depth>=0 && depth<maxDepth){  /* tail match */
	while(path[depth]<factor &&
		!trace[depth]->children[(int)path[depth]])
	   ++path[depth];
	if(path[depth]>=factor){
	   path[depth--]=0; ++path[depth];
	}else{
	   word[dif+depth]='a'+path[depth];
	   trace[depth+1]=trace[depth]->children[(int)path[depth]];
	   path[++depth]=0;
	   if(trace[depth]->node_freq){
		word[dif+depth]=0;
		printf("[%s]:%u\n", word,trace[depth]->node_freq);
	   }
	}
   }
}

int main(int argc, char*argv[]){
   if(argc<2)return
	!printf("Usage: %s dictFname [partialWord1 [partialWord2] ...]\n",argv[0]);
   trie* root=newTrie(0);
   buildTrie(argv[1],root, addWord);
//    showWords(root);
//    buildTrie(argv[1],root, rmWord); /* TODO: buggy */
//    showNodes(root); puts("");
   if(argc>2){
	int indx=2;
	for(; indx<argc; ++indx){
	   prompt(root,argv[indx]);
	   puts("");
	}
   }
   destroyTrie(root);
   return 0;
}
