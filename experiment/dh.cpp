#include<iostream>
#include<cstdlib>
#include<ctime>
#include<string>
#include "dh.h"
using namespace std;

char p_set[NUMP][129]={
{"62389774203809592484935255385800211465585304869804518745575564327212338359301463237546457777605149611234567890123456789023415677"},
{"62389774203809592484935255385800211465585304869804518745575564327212338359301463237546457777605149631234567890123456789023415677"}
};

int getn(char*a){
int i=MAX-1;
while(i>=0&&!a[i])i--;
return i+1;
}
int mul(char*a,char*b){
int na,nb,i,j,n;
char c[MAX],d[MAX];
for(i=0;i<MAX;i++)c[i]=a[i],d[i]=b[i],a[i]=0;
na=getn(c);
nb=getn(d);
for(i=0;i<nb;i++)for(j=0;j<na;j++){
   a[i+j]+=d[i]*c[j];
   if(a[i+j]>9)a[i+1+j]+=a[i+j]/10,a[i+j]%=10;
}
for(i=0;i<MAX&&i<(na+nb);i++){
   if(a[i])n=i+1;
   if(a[i]>9)a[i+1]+=a[i]/10,a[i]%=10;
}
return n;
}
int mod(char*a,char*b){
int na,nb,i,u,f=0,n;
na=getn(a);
nb=getn(b);
u=na-nb;
if(u<0)return 0;
while(u+1){
   for(i=na-1,f=0;i>=u;i--){
    if(a[i]>b[i-u]){f=1;break;}
    if(a[i]<b[i-u]){f=-1;break;}
   }
   if(!f){
    for(i=na-1;i>=u;i--)a[i]=0;
    u-=nb;
    if(u<0)break;
    continue;
   }
   if(f==-1)u--;
   if(f==1){
    for(i=u;i<na;i++){
     a[i]-=b[i-u];
     if(a[i]<0)a[i+1]--,a[i]+=10;
    }
   }
}
for(i=0;i<na;i++)if(a[i])n=i+1;
return n;
}
void reverse(char*a){
int i,n;
n=getn(a);
for(i=0;i<n/2;i++)swap_byte(a[i],a[n-1-i]);
}
void deal(char*a){
int i=0;
while(a[i])a[i++]-='0';
}
void getg(char*a){
int i,r,j=0;;
srand(time(0));
while(1){
   r=rand()%10000;
   for(i=0;i<4;i++){
    a[j++]=r%10;
    r/=10;
    if(j==NUMG)return;
   }
}
}
void getp(char*a,int n){
int i;
//for(i=0;i<100;i++)a[i]=p_set[n][i];
for(i=0;i<128;i++)a[i]=p_set[n][i];
}
void display(char*a){
int n,i;
n=getn(a);
printf("n=%d",n);
reverse(a);
for(i=0;i<n;i++)printf("%d",a[i]);
printf("\n");
reverse(a);
}
void encon(int a,char* p,char* g,char *ans){
char t[MAX]={0};
int p_n=0;    
int i;
memset(p,0,MAX*sizeof(p[0]));
memset(g,0,MAX*sizeof(g[0]));
srand(time(0));
p_n=rand()%NUMP;   //Ëæ»úµÃµ½p_n
getg(g);
getp(p,p_n);
deal(p);
reverse(p);
for(i=0;i<MAX;i++)t[i]=g[i],ans[i]=0;
ans[0]=1;
for(i=0;i<32;i++){ 
   if(a&1<<i){
    mul(ans,t);
    mod(ans,p);
   }
   mul(t,t);
   mod(t,p);
}
}

void recon(int b,char *p,char *g,char *ans){
char t[MAX]={0};
int i;
for(i=0;i<MAX;i++)t[i]=g[i],ans[i]=0;
ans[0]=1;
for(i=0;i<32;i++){ 
   if(b&1<<i){
    mul(ans,t);
    mod(ans,p);
   }
   mul(t,t);
   mod(t,p);
}
}
void getkey(int b,char *rec,char*p,char*g,char *ans){
char t[MAX]={0};
int i;

printf("size=%d",strlen(p_set[0]));
for(i=0;i<MAX;i++)t[i]=rec[i],ans[i]=0;
ans[0]=1;
for(i=0;i<32;i++){
   if(b&1<<i){
    mul(ans,t);
    mod(ans,p);
   }
   mul(t,t);
   mod(t,p);
}
}