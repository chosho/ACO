/*******
task  DL  Exe
1      10  3
2      10  4
3      10  5
4      9   8
5      8   6

******/
#include<stdio.h>
#include<string.h>
#include <time.h>
#include<math.h>
#define C 0.1
#define K 10.0
#define apha 1
#define beta 1
#define printout 0
#define maxtime 361
#define MAXph 2.0
#define MINph 0.0
struct task{
	int taskname;
	int arrival;
	int deadline;
                int exe;//run time
                int rexe;// reman time
	double phromon;
	double taueta;
	double p;
	int k;//executable 1;can't exe 0
};
typedef struct task T;
/***************Function Prototype****************************/
void ExeProb(T *task,int currtime,int size);
int ExeTask(T *task,int currtime,int size);
void UpdatePhromon(T *task,float ph,int num,int size);
void FixSequence(T *task,int currtime,int size);
void SuccessRate(T *task,int currtime,int size);
int TaskRelease(T *task,int currtime,int size);
void PrintOut(T *task,int currtime,int size);


int main(){
/***********clock************************/
/*clock_t start, end;

  start = clock();
  printf("***********************************************************************\n");
  printf( "star time:%d\n", start );
/***********clock************************/

 T newtask[]={
   {1,0,10,3,3,1.0,0,0,1},
   {2,0,10,4,4,1.0,0,0,1},
   {3,0,10,5,5,1.0,0,0,1},
   {4,0,9,8,8,1.0,0,0,1},
   {5,0,8,6,6,1.0,0,0,1},
 
 
 }; 

int size =sizeof(newtask)/sizeof(T); //size=Numbers of Task ,last task--> task[size-1]
int timer=0;
int CompletesTure=0;
int cnt=0;
 int i;
int total_job=0;
		/****TOTALJOBS**/
		for(i=0;i<size;i++){
		  total_job+=maxtime/newtask[i].deadline;
		}

		printf("TOTAL JOBS :%d\n",total_job);
/***************AcO loop***********/
while(timer<maxtime){
if(TaskRelease(newtask,timer,size)||CompletesTure){
ExeProb(newtask,timer,size);//#1figure out Exe.Probability.##first
FixSequence(newtask,timer,size);
SuccessRate(newtask,timer,size);//include ##UPdate Pheromon
ExeProb(newtask,timer,size);//#1figure out Exe.Probability.##second
FixSequence(newtask,timer,size);
if(printout)
PrintOut(newtask,timer,size);
}
CompletesTure=ExeTask(newtask,timer,size);//## Exe Task
if(CompletesTure)
cnt++;
timer++;
}

printf("\n");
printf("\n");
printf("             RESULT   ACO Shah                         \n");
 printf("     maxtime=%d\n     apha=%d beat=%d \n     SuccessJob=%d(%4.1f%%)\n      \n",maxtime,apha,beta,cnt,(float)cnt/total_job*100);
/***********clock************************/
/* end = clock();
  printf("***********************************************************************\n");
  printf( "end time:%d\n", end );
  printf( "exetime :%d[ms]\n", end - start );
    printf("***********************************************************************\n");
	
/***********clock************************/
return 0;
}
/*************Exe.Probability**********************/
void ExeProb(T *task,int currtime,int size){

double sum =0;
int ct;
for(ct=0;ct<size;ct++){
if(task[ct].k==1){
  task[ct].taueta=pow(task[ct].phromon,apha)*pow(K/(task[ct].deadline+task[ct].arrival-currtime),beta); 

sum+=task[ct].taueta;
}
}
for(ct=0;ct<size;ct++){

task[ct].p=task[ct].taueta/sum;
}
}
/*************FixExe.Sequence**********************/
void FixSequence(T *task,int currtime,int size ){
int i,j;
T temptask;
for(j=0;j<size;j++)
for(i=0;i<size;i++){
if( task[i].p<task[i+1].p){
	temptask=task[i];
	task[i]=task[i+1];
	task[i+1]=temptask;
	    }	
		
}

}
/*****************ExeTask********************/
int ExeTask(T *task,int currtime,int size){
int j;
int complete=0;
for(j=0;j<size;j++){

if(task[j].k==1){
task[j].rexe--;
if(task[j].rexe==0){
task[j].k=0;
complete=1;

}
if(printout){
printf("*********************************************************************\n");
printf("Time %d:Task %d is EXE--------------------\n",currtime,task[j].taskname);
printf("*********************************************************************\n");
}
break;
}

}
if (complete)
return 1;
else 
return 0; 
}
/*************************WORK SPACE**************/
/*************UpdatePhromon**********************/
void UpdatePhromon(T *task,float ph,int num,int size){
int i,j;
int ture=1;
float rho=0.8;

//task[numb]~task[size-1]
j=1;
for(i=num;i<size;i++){
/***OLD contrlo MIN*****************/
//task[i].phromon=task[i].phromon*rho;
//if(task[i].phromon<MINph)
//task[i].phromon=MINph;
if(task[i].k==1){
/**NEW contrlo MIN**************/
task[i].phromon=task[i].phromon*rho;
if(task[i].phromon<MINph)
task[i].phromon=MINph;
/*******************contrlo MAX*************/
task[i].phromon+=(float)ph*C/j;
if(task[i].phromon>MAXph)
task[i].phromon=MAXph;
j++;
ture=1;
}
if(task[i].k==0&&i==j){
ture=0;}
}
//task[0]~task[Numb-1]
if(ture==1){
j--;
for(i=0;i<num;i++){
/***OLD contrlo MIN*******************/
//task[i].phromon=task[i].phromon*rho;
//if(task[i].phromon<MINph)
//task[i].phromon=MINph;
if(task[i].k==1){
/**NEW contrlo MIN**************/
task[i].phromon=task[i].phromon*rho;
if(task[i].phromon<MINph)
task[i].phromon=MINph;
/****************contrlo MAX****************/
task[i].phromon+=(float)ph*C/j;
if(task[i].phromon>MAXph)
task[i].phromon=MAXph;
j++;
}

}
}


}
/*************SuccessRate**********************/
void SuccessRate(T *task,int currtime,int size){
int i,j;
int SumSucTask=0;
int SumFailTask=0;
float SucRate[size];
float ulz=0;
int ture=1;
for(j=0;j<size;j++){
if(j==0){// =0 case
for(i=j;i<size;i++){
if(task[i].k==1){
ulz+=(float)task[i].exe/(float)(task[i].deadline);
if(ulz<=1.0)
SumSucTask++;
else
SumFailTask++;
}
else if(i==0){break;}
}

}


else{//!=0 case
//task[numb]~task[size-1]
for(i=j;i<size;i++){
if(task[i].k==1){
ulz+=(float)task[i].exe/(float)(task[i].deadline);
if(ulz<=1.0)
SumSucTask++;
else
SumFailTask++;
ture=1;
}
if(task[j].k==0&&i==j){
ture=0;
break;}
}
//task[0]~task[Numb-1]
if(ture==1){
for(i=0;i<j;i++){
if(task[i].k==1){
ulz+=(float)task[i].exe/(float)(task[i].deadline);//exe/deadline(absoulte)
if(ulz<=1.0)
SumSucTask++;
else
SumFailTask++;
}

}
}

}

SucRate[j]=(float)SumSucTask/(float)(SumFailTask+1);//value of PH 
ulz=0;
SumSucTask=0;
SumFailTask=0;

}
if(printout){
for(i=0;i<size;i++)
printf("SucRate[%d]=%.2f\n",i,SucRate[i]);
}
/*****find best squeen **/
int best=0;
int secd=0;
float bestSucRate=0;
float secdSucRate=0;
//for(j=0;j<size;j++)
for(i=0;i<size;i++){
if(bestSucRate<SucRate[i]){
bestSucRate=SucRate[i];//value of PH1
best=i;
}

}
/******************************first update******************************************/
UpdatePhromon(task,bestSucRate,best,size);
for(i=0;i<size;i++){
if(i!=best&&secdSucRate<SucRate[i]){
secdSucRate=SucRate[i];//value of PH2
secd=i;
}
}
/******************************second update******************************************/
UpdatePhromon(task,secdSucRate,secd,size);
if(printout){
printf("1st-SucRate is SucRate[%d]=%.2f\n",best,SucRate[best]);
printf("2nd-SucRate is SucRate[%d]=%.2f\n",secd,SucRate[secd]);
}
}
/*****************New TaskRelease *********************************/
int TaskRelease(T *task,int currtime,int size){
  int i;
  int cnt=0; 
  for(i=0;i<5;i++){
    if(currtime%task[i].deadline==0){
task[i].arrival=currtime;
task[i].k=1;
task[i].rexe=task[i].exe;
 cnt ++;
}

  }

if (cnt>0)
return 1;
else
return 0;

}



/*****************TaskRelease *********************************/
/*int TaskRelease(T *task,int currtime,int size){
int i;
//int ture=0;
int cnt=0;
if(currtime%10==0){
for(i=0;i<size;i++){
if(task[i].taskname==1){
task[i].arrival=currtime;
task[i].k=1;
task[i].exe=3;
if(printout)
printf("Time%d Task %d releace \n",currtime,task[i].taskname);
cnt++;}
if(task[i].taskname==2){
task[i].arrival=currtime;
task[i].k=1;
task[i].exe=4;
if(printout)
printf("Time%d Task %d releace \n",currtime,task[i].taskname);
cnt++;}
if(task[i].taskname==3){
task[i].arrival=currtime;
task[i].k=1;
task[i].exe=5;
if(printout)
printf("Time%d Task %d releace \n",currtime,task[i].taskname);
cnt++;}

}
}
 if(currtime%9==0)
for(i=0;i<size;i++){
if(task[i].taskname==4){
task[i].arrival=currtime;
task[i].k=1;
task[i].exe=8;
if(printout)
printf("Time%d Task %d releace \n",currtime,task[i].taskname);
cnt++;}
}
 if(currtime%8==0)
for(i=0;i<size;i++){
if(task[i].taskname==5){
task[i].arrival=currtime;
task[i].k=1;
task[i].exe=6;
if(printout)
printf("Time%d Task %d releace \n",currtime,task[i].taskname);
cnt++;}
}
if (cnt>0)
return 1;
else
return 0;
 /*
 if(currtime%10==0)
for(i=0;i<size;i++){
if(task[i].deadline==10){
task[i].arrival=currtime;
task[i].k=1;
task[i].exe=3;
printf("Time%d Task %s releace \n",currtime,task[i].taskname);
}
}
 if(currtime%20==0)
for(i=0;i<size;i++){
if(task[i].deadline==20){
task[i].arrival=currtime;
task[i].k=1;
task[i].exe=12;
printf("Time%d Task %s releace \n",currtime,task[i].taskname);
}
}
}
*/


/*************************************PrintOut**************************************************/
void PrintOut(T *task,int currtime,int size){
int i;
printf("***********************************************************************\n");
printf("TIME  ");
printf("taskname  ");
printf("Arrival Time  ");
printf("Absolute Deadline  / Rho ");
printf("Exe.Time  ");
printf("Probability\n");
printf("***********************************************************************\n");

for(i=0;i<size;i++){
if(task[i].k==1){
printf("  %d  ",currtime);
printf("    %d  ",task[i].taskname);
printf("    %d  ",task[i].arrival);
//printf("            %d  ",newtask[i].deadline);
printf("            %d  ",task[i].deadline+task[i].arrival);
printf("%.3f ",task[i].phromon); //
if(i<3)printf("                %d  ",task[i].exe);
else printf("               %d  ",task[i].exe);
if(i<4)printf("       %.3f  \n",task[i].p);
else printf("      %.3f  \n",task[i].p);
}

}
}








