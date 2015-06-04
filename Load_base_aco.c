/*
EDZLできたよー（動作保証ZERO）
EDFから追加されたところはEA(Edf Addition)っていうのコメント付けてるから探してみてちょ！
*/

#include<stdio.h>
#include<string.h>
#include <stdlib.h>
#include <time.h>
#include<math.h>
//#define TSP 100
//#define Aco 1
//#define Edf 0
#define NUMBS 50		//タスク数
#define pNum 1		//プロセッサ数
#define C 0.1 //ACO定数
#define K 10.0//ACO定数
//#define apha 1//ACO定数
//#define beta 1//ACO定数

typedef struct{			//構造体の宣言
	int runtime;		//実行時間
	int deadline;		//デッドライン
	int index;	        //タスクのインデックス
	int runtimeRemaining;	//タスクの残り実行時間
	int dover;              //デッドラインオーバー量
	int dmiss;              //そのタスクがデッドラインミスをした回数
	int laxity;             //余裕時間 EA
	int flag;               //複数のプロセッサで同じタスクが実行されないようにするためのフラグ
	int endflag;            //タスクが終了したときのスケジュール呼び出しを一回だけにするためのフラグ
    double phromon;
    double taueta;
    double p;
    int k;
    int arrival;
	}Task;

typedef struct{
  int before_task;//前の時刻で実行されていたタスク
  int flag;       //そのプロセッサが使われてるか、使われてないか判定するフラグ
}Processor;

void BubSort(Task x[], int n, int time, int period); 
void Bub2Sort(Task x[], int n, int time, int period);
void ACO(Task *task,int time,int size,int apha,int beta);

int RandMakeTask(float SystemUlz,int a[],int b[]);
void ExeProb(Task *task,int currtime,int size,int apha,int beta);//タスク実行確率算出関数
//int ExeTask(Task *task,int currtime,int size);//タスク実行関数
void OldUpdatePhromon(Task *task,float ph,int num,int size);//タスクフェロモン更新関数
void FixSequence(Task *task,int currtime,int size);//タスク実行順序決定関数
void OldSuccessRate(Task *task,int currtime,int size);//タスク実行成功率算出関数
int SUFA(Task task_set[],int tNum);


int lcm(int x, int y);

int gcd(int x, int y);


int main()
{
    
    float SystemUlz=1.0;//4.0;//total システム利用率
    int a[NUMBS]={0},b[NUMBS]={0};
    int tNum=0;
    printf("ULZ:");
    scanf("%f",&SystemUlz);
    tNum=RandMakeTask(SystemUlz,a,b);
	Task tasks[tNum];	//タスクの構造体の配列
	Processor pr[pNum];     //プロセッサの構造体の配列

	FILE *fp;	     	//読み込むファイル

	int n=0;		//ファイル読み込み用ループ変数
	int i;			//ループ用変数
	int j;			//ループ用変数その2
	int k;                  //ループ用変数その3

	int period=0;           //タスクセットの周期(どの時刻まで繰り返すか)
	int sw=0;		//コンテキストスイッチの回数
	int time=0;		//時刻
	int sflag=0;            //スケジューラーを起動するか判定するフラグ

	int deadlinemiss=0;     //全タスクのデッドラインミス回数
	int deadlineover=0;     //全タスクのデッドラインオーバー量の合計
	int SW=-1; //switch citerion
    int Edf=0,Aco=0;
    int apha=1,beta=1;
    
    
 
	//インデックスの付加
	for(i=0;i<tNum;i++){
	  tasks[i].index=i;
	}

	//全タスクの残りの実行時間、デッドラインオーバー量、デッドラインミス回数の初期化
	for(i=0;i<tNum;i++){
        tasks[i].runtime=b[i];
        tasks[i].deadline=a[i];
	  tasks[i].runtimeRemaining = 0;
	  tasks[i].dover = 0;
	  tasks[i].dmiss = 0;
        tasks[i].phromon=1.0;
        tasks[i].taueta=0;
        tasks[i].p=0;
        tasks[i].k=1;
        tasks[i].arrival=0;

        
	}

	//プロセッサの初期化
	for(i=0;i<pNum;i++){
	  pr[i].before_task = tNum + 1;
	}

	//周期の計算
	for(i=0;i<tNum;i++){

	  if(i==0){
	    period = lcm(tasks[i].deadline,tasks[i+1].deadline);
	    i++;
	  }
	  
	  else{
	    //先ほど求めた2つの最小公倍数と新しいタスクの周期の最小公倍数を求める
	    period = lcm(period,tasks[i].deadline);
	  }

	}
    
	
	//周期確認用
    printf("周期%d\n",period);
    if (period<0){
        printf("##erro!!ENTER:Ctrl Z!!##\n");
        return 1;
    }
    int total_job=0;
    
    /****TOTALJOBS**/
    for(i=0;i<tNum;i++){
        total_job+=period/tasks[i].deadline;
    }
    
    printf("TOTAL JOBS :%d\n",total_job);
	
	for(time=0;time<period;time++){

	  //現在の時刻の表示
	  //printf("時刻%d\n",time);

	  //EDZlは毎時刻スケジューラーを起動するので EA
	  sflag = 1;
	  
	  //プロセッサを空いている状態へ
	  for(i=0;i<pNum;i++){
	    pr[i].flag = 0;
	  }

	  //スケジューラーの起動判定およびタスクのデッドラインミス判定、タスクの起動確認してからの実行時間調整
	  for(i=0;i<tNum;i++){

	    tasks[i].flag = 0;//全タスクについてのフラグを初期化

	    //タスクの起動時刻になったとき
	    if( time % tasks[i].deadline == 0){
            tasks[i].arrival=time;
            tasks[i].k=1;

	      //printf("タスク%dが起動\n",tasks[i].index);

	      //タスクが起動するとき、そのタスクに残り実行時間があれば、デッドラインミス
	      if(tasks[i].runtimeRemaining != 0){
		//printf("タスク%dがデッドラインミス\n",tasks[i].index);
		//printf("デッドラインオーバー量%d\n",tasks[i].runtimeRemaining);
		tasks[i].dmiss++;
		tasks[i].dover = tasks[i].dover + tasks[i].runtimeRemaining;
	      }

	      //タスクの残り実行時間をそのタスクの実行時間にする
	      tasks[i].runtimeRemaining = tasks[i].runtime;

	      //フラグ初期化
	      tasks[i].endflag = 0;

	      //スケジューラー起動
	      sflag = 1;

	    }

	    //タスクの残り実行時間がゼロになったタスクが発生したなら、プロセッサ割り当てのためスケジューラー起動
	    if(tasks[i].runtimeRemaining == 0 && tasks[i].endflag == 0){

	      //スケジューラー起動
	      sflag = 1;

	      //そのタスクが実行終了したことを表す
	      tasks[i].endflag = 1;

	      //printf("タスク%dが終了\n",tasks[i].index);

	    }

	  }//スケジューラーの起動判定〜の作業はここまで


	  //以下よりスケジューラーの作業

	  //スケジューラーが起動されたら
	  if(sflag == 1){

	    //printf("スケジューラー起動\n");

	/*    for(i=0;i<tNum;i++){
	      //タスクの余裕時間計算 EA
	      tasks[i].laxity = tasks[i].deadline - (time % tasks[i].deadline) - tasks[i].runtimeRemaining;
	      if(tasks[i].laxity==0)
		LLF=1;
}*/
	  //  if(LLF)
	    //BubSort(tasks,tNum,time,period);
	    //else
	    //優先度順に配列を並び替える関数バブルソートを呼び出し(配列の番号が若いものほど優先度が高くなる)
         
          SW=SUFA(tasks,tNum);
          //SW=0;
          if(SW==1) Edf=1;
          if(SW==0) Aco=1;
          //SW=-1;
          if(Edf){
          BubSort(tasks,tNum,time,period);
              //if(time==0)
                  //printf("EDF \n");
              Edf=0;
          }
              // ACO動作部分
         // ACO(tasks,time,tNum,apha,beta);
          if(Aco){
              //if(time==0)
                //  printf("ACO \n");
          ExeProb(tasks,time,tNum,apha,beta);//#1figure out Exe.Probability.##first
          FixSequence(tasks,time,tNum);
          OldSuccessRate(tasks,time,tNum);//include ##UPdate Pheromon
          ExeProb(tasks,time,tNum,apha,beta);//#1figure out Exe.Probability.##second
          FixSequence(tasks,time,tNum);
              Aco=0;
          }
	    //配列の中身確認（優先順番がどうなってるかみたいときに用いる）
	    	    for(i=0;i<tNum;i++){
	      //printf("タスク番号%d 残りの実行時間%d デッドライン%d pheromon%.2f p %.2f\n",tasks[i].index,tasks[i].runtimeRemaining,(time+tasks[i].deadline - (time %tasks[i].deadline)),tasks[i].phromon,tasks[i].p);
	      }

	    //無駄なコンテキストスイッチを発生させないように
	    //現時刻で実行されるタスクの中で前時刻で実行されていたタスクがあれば同じプロセッサで割り当てるようにする
	    for(i=0;i<pNum;i++){

	      for(j=0;j<tNum;j++){
		
		//前回実行されていたタスクと今回実行されるタスクが同じなら、同じプロセッサに割り当てる
		if(pr[i].before_task == tasks[j].index && tasks[j].endflag == 0 && j < pNum){
		  
		  //タスクの実行が行われるので、そのタスクの残りの実行時間を1引く
		  tasks[j].runtimeRemaining--;
		  
		  //プロセッサ割り当てがされたので、これ以上このプロセッサに割り当てないようフラグを1にする
		  pr[i].flag = 1;

		  //このタスクはプロセッサに割り当てられたので、複数割り当てられないようフラグを1にする
		  tasks[j].flag = 1;
		  
		  //確認用
		  //printf("前回に引き続き実行されるタスク%d\n",tasks[j].index);
		  
		}
		
	      }

	    }//無駄なコンテキストスイッチ〜作業ここまで

	    //余っているプロセッサにタスクを割り当てる
	    for(i=0;i<pNum;i++){
	      
	      //余っているプロセッサがあれば
	      if(pr[i].flag == 0){
		
		for(j=0;j<tNum;j++){//全タスクについて調べる
		  
		  //前時刻に実行されていないタスクかつ実行終了していないタスクの中で優先度が高いタスクを割り当てる
		  if(tasks[j].flag == 0 && tasks[j].endflag == 0){
		    
		    //確認用
		    //printf("余っているプロセッサ%dに入るタスク%d\n",i,tasks[j].index);
		    
		    //前時刻でのプロセッサがアイドル状態でなかった場合かつ
		    //前時刻で実行されていたタスクが実行終了していなかった場合
		    //コンテキストスイッチが発生する
		    
		    //コンテキストスイッチの発生調べ作業
		    if(pr[i].before_task != tNum +1){//プロセッサがアイドル状態でないなら
		      
		      for(k=0;k<tNum;k++){//前時刻で実行していたタスク探し
			
			//前時刻で実行していたタスクを発見
			if(pr[i].before_task == tasks[k].index){
			  
			  //前時刻で実行していたタスクが実行終了していなかったら
			  if(tasks[k].runtimeRemaining != 0){
 
			    sw++;//コンテキストスイッチの発生

			    //確認用
			    //printf("コンテキストスイッチ発生 タスク番号%dがタスク番号%dから奪った\n",tasks[j].index,pr[i].before_task);
			  }
			  
			}

		      }

		    }//コンテキストスイッチの発生調べ作業ここまで

		    //現時刻で実行されるタスクをプロセッサに格納
		    pr[i].before_task = tasks[j].index;

		    //実行されるタスクの残り実行時間をマイナス1する
		    tasks[j].runtimeRemaining--;
		    
		    //このタスクが複数のプロセッサに割り当てられないようフラグを1にする
		    tasks[j].flag = 1;

		    //プロセッサ割り当てがされたので、これ以上このプロセッサに割り当てないようフラグを1にする
		    pr[i].flag = 1;
		      
		    //このプロセッサに複数のタスクが実行されないようループを終了させるためのもの
		    j = tNum;


		  }//優先度が高いタスク割り当てここまで

		}//全タスク調べここまで

		//プロセッサが余っていたら
		if(pr[i].flag == 0){

		  //tNum + 1がアイドル状態を表すとする
		  //実際、使用するタスク番号と被らなければ、どんな値でも問題はない
		  pr[i].before_task = tNum + 1;

		}

	      }//プロセッサが余っているならの閉じかっこ

	    }//全プロセッサ調べここまで

	    
	    //スケジューラーの作業が終わったのでフラグを0にする
	    sflag = 0;
	    
	  }//スケジューラーが起動されたらはここまで
	  
	  
	  //スケジューラーが起動されなかった場合
	  else{
	    
	    /*配列の中身確認（優先順番がどうなってるかみたいときに用いる）
	    for(i=0;i<tNum;i++){

	      printf("タスク番号%d 残りの実行時間%d デッドライン%d\n",tasks[i].index,tasks[i].runtimeRemaining,(time+tasks[i].deadline - (time %tasks[i].deadline)));

	      }*/

	    //スケジューラーが起動しなかったということは前時刻と同じタスクを現時刻でも引き続き実行を行うことである

	    //前時刻で実行されていたタスク探し
	    for(i=0;i<pNum;i++){
	      for(j=0;j<pNum;j++){
		if(pr[i].before_task == tasks[j].index){

		  //前時刻で実行されていたタスクは現時刻でも実行されるのでそのタスクの残り実行時間をマイナス1する
		  tasks[j].runtimeRemaining--;

		}

	      }

	    }//前時刻で実行されていたタスク探しここまで

	  }//スケジューラーが起動されなかった場合ここまで


	  //見やすさ用
	  //printf("\n");


	  //今回実行されるタスクとプロセッサについての表示
	 // for(i=0;i<pNum;i++){
	   // if(pr[i].before_task != tNum + 1)
	      //printf("プロセッサ%d 実行されるタスク%d\n",i,pr[i].before_task);
	    //else
	      //printf("プロセッサ%d アイドル状態\n",i);
	  //}
	  
	  //見やすさ用
	  //printf("\n");

	}//EDZL動作はここまで
			

	if(time == period){
	  for(i=0;i<tNum;i++){
	    if(tasks[i].runtimeRemaining != 0){
	      tasks[i].dover = tasks[i].dover + tasks[i].runtimeRemaining;
	      tasks[i].dmiss++;
	      //printf("タスク%dがデッドラインミス,",tasks[i].index);
	      //printf("デッドラインオーバー量%d\n",tasks[i].runtimeRemaining);
	    }
	  }
	}

	//最終結果の表示
	
	//見やすさ用
	printf("\n");

	//各タスクがどのくらいデッドラインミスをしてオーバーしたかの詳細がしりたい場合
	printf("%d単位時間実行した、それぞれのタスクの結果\n",time);

	for(i=0;i<tNum;i++){

	  //各タスクがどのくらいデッドラインミスをしてオーバーしたかの詳細がしりたい場合
	  printf("タスク%dのデッドラインミス回数 %d回 デッドラインオーバー量 %d単位時間\n",tasks[i].index,tasks[i].dmiss,tasks[i].dover);

	  deadlinemiss = deadlinemiss + tasks[i].dmiss;
	  deadlineover = deadlineover + tasks[i].dover;

	}//各タスクの詳細ここまで

	//見やすさ用
	printf("\n\n");

	//スケジュール成功か失敗の判定
	if(deadlinemiss == 0)
	  printf("スケジュール成功\n");
	else
	  printf("スケジュール失敗\n");

	//コンテキストスイッチと全タスクの合計でのデッドラインミス回数とデッドラインオーバー量の表示
	printf("コンテキストスイッチ%d回\n",sw);
	printf("全タスクのデッドラインミス ratio %.2f%% デッドラインオーバー量 %d単位時間\n",1.0-(double)deadlinemiss/total_job,deadlineover);

	return 0;

}//main 関数の閉じかっこ

/* デッドラインが早い順にソートする */
void BubSort(Task x[], int n, int time, int period){
  int i, j;
  int k = 0;//EA
  Task temp;

  //残り実行時間がないものは優先度を最後にさせるためのもの
  for(i=0;i<n;i++){

    if(x[i].runtimeRemaining == 0){

      x[i].deadline = x[i].deadline + period; 

    }

  }

  for (i=0;i<n-1;i++){

    for (j=n-1;j>i;j--){

      if ((time+x[j-1].deadline - (time %x[j-1].deadline)) > (time+x[j].deadline - (time %x[j].deadline))){	/* 前の要素の方が大きかったら */

	temp=x[j];        				/* 交換する */
	x[j]=x[j-1];
	x[j-1]=temp;

      }

      else if((time+x[j-1].deadline - (time %x[j-1].deadline)) == (time+x[j].deadline - (time %x[j].deadline))){
	
	if(x[j-1].index > x[j].index){
	  temp=x[j];        				/* 交換する */
	  x[j]=x[j-1];
	  x[j-1]=temp;
	  
	}
	
	
      }
      
    }

  }

  //優先度を最後にするためにデッドラインをいじったので、それを元のデッドラインに戻す作業をしている
  for(i=0;i<n;i++){

    if(x[i].runtimeRemaining == 0){

      x[i].deadline = x[i].deadline - period; 

    }
  }



}
/********LLF***********/
void Bub2Sort(Task x[], int n, int time, int period){
  int i, j;
  int k = 0;//EA
  Task temp;

  //残り実行時間がないものは優先度を最後にさせるためのもの
  for(i=0;i<n;i++){

    if(x[i].runtimeRemaining == 0){

      x[i].deadline = x[i].deadline + period; 

    }

  }

  for (i=0;i<n-1;i++){

    for (j=n-1;j>i;j--){

      if ((time+x[j-1].laxity) > (time+x[j].laxity)){	/* 前の要素の方が大きかったら */

	temp=x[j];        				/* 交換する */
	x[j]=x[j-1];
	x[j-1]=temp;

      }

      else if((time+x[j-1].laxity) == (time+x[j].laxity)){
	
	if(x[j-1].index > x[j].index){
	  temp=x[j];        				/* 交換する */
	  x[j]=x[j-1];
	  x[j-1]=temp;
	  
	}
	
	
      }
      
    }

  }

  //優先度を最後にするためにデッドラインをいじったので、それを元のデッドラインに戻す作業をしている
  for(i=0;i<n;i++){

    if(x[i].runtimeRemaining == 0){

      x[i].deadline = x[i].deadline - period; 

    }
  }


}
//aco
/*void Aco(Task *task,int time,int size,int apha,int beta ){

ExeProb(task,time,size,apha,beta);//#1figure out Exe.Probability.##first
FixSequence(task,time,size);
OldSuccessRate(task,time,size);//include ##UPdate Pheromon
ExeProb(task,time,size,apha,beta);//#1figure out Exe.Probability.##second
FixSequence(task,time,size);
}
*/
/*************Exe.Probability**********************/
void ExeProb(Task *task,int currtime,int size,int apha,int beta){
    
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
void FixSequence(Task *task,int currtime,int size ){
    int i,j;
    Task temptask;
    for(j=0;j<size;j++)
        for(i=0;i<size;i++){
            if( task[i].p<task[i+1].p){
                temptask=task[i];
                task[i]=task[i+1];
                task[i+1]=temptask;
            }	
            
        }
    
}
/*************UpdatePhromon**********************/
void OldUpdatePhromon(Task *task,float ph,int num,int size){
    int i,j;
    int ture=1;
    float rho=0.8;
    
    //task[numb]~task[size-1]
    j=1;
    for(i=num;i<size;i++){
        /***OLD contrlo MIN*****************/
        task[i].phromon=task[i].phromon*rho;
        //if(task[i].phromon<MINph)
        //task[i].phromon=MINph;
        if(task[i].k==1){
            /**NEW contrlo MIN**************/
            //task[i].phromon=task[i].phromon*rho;
            //if(task[i].phromon<MINph)
            //task[i].phromon+=MINph;
            /*******************contrlo MAX*************/
            task[i].phromon+=(float)ph*C/j;
            //if(task[i].phromon>MAXph)
            //task[i].phromon=MAXph;
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
            task[i].phromon=task[i].phromon*rho;
            //if(task[i].phromon<MINph)
            //task[i].phromon=MINph;
            if(task[i].k==1){
                /**NEW contrlo MIN**************/
                //task[i].phromon=task[i].phromon*rho;
                //if(task[i].phromon<MINph)
                //task[i].phromon+=MINph;
                /****************contrlo MAX****************/
                task[i].phromon+=(float)ph*C/j;
                //if(task[i].phromon>MAXph)
                //task[i].phromon=MAXph;
                j++;
            }
            
        }
    }
    
    
}
/*************SuccessRate**********************/
void OldSuccessRate(Task *task,int currtime,int size){
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
                    ulz+=(float)task[i].runtime/(float)(task[i].deadline);
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
                    ulz+=(float)task[i].runtime/(float)(task[i].deadline);
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
                        ulz+=(float)task[i].runtime/(float)(task[i].deadline);//exe/deadline(absoulte)
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
  /*  if(printout){
        for(i=0;i<size;i++)
            printf("SucRate[%d]=%.2f\n",i,SucRate[i]);
    }*/
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
    OldUpdatePhromon(task,bestSucRate,best,size);
    for(i=0;i<size;i++){
        if(i!=best&&secdSucRate<SucRate[i]){
            secdSucRate=SucRate[i];//value of PH2
            secd=i;
        }
    }
    /******************************second update******************************************/
    OldUpdatePhromon(task,secdSucRate,secd,size);
    /*if(printout){
        printf("1st-SucRate is SucRate[%d]=%.2f\n",best,SucRate[best]);
        printf("2nd-SucRate is SucRate[%d]=%.2f\n",secd,SucRate[secd]);
    }*/
}

// 最大公約数を求める関数
int gcd(int x, int y)
{
    int r;

    if(x == 0 || y == 0)  // 引数チェック
    {
        fprintf(stderr, "引数エラーです。\n");
        return 0;
    }

    // ユーグリッドの互除法
    while((r = x % y) != 0) // yで割り切れるまでループ
    {
        x = y;
        y = r;
    }
    return y;
}

// 最大公約数を求める関数
int lcm(int x, int y)
{
    if(x == 0 || y == 0)  // 引数チェック
    {
        fprintf(stderr, "引数エラーです。\n");
        return 0;
    }

    return (x * y / gcd(x, y));
}


/*****************************/
int RandMakeTask(float SystemUlz,int a[],int b[]){
    int i=0;
    int j=0;
    int tmp=0;
    //  int a[NUM],b[NUM];
    float ulz=0;
    float ULZ=SystemUlz;
    //int tane=1;
    do{
        // srand(10);
        //srand((unsigned)time(NULL));
        a[i]=(rand()%10+1)*5;
        // printf("rand a[%d]=%d\n",i,a[i]);
        
        do{
            //printf("search b[i]... ...\n");
            // srand(10);
            //srand((unsigned)time(NULL));
            tmp=rand()%29+1;
        }while(tmp>a[i]);
        
        b[i]=tmp;
        //printf("rand b[%d]=%d\n",i,b[i]);
        
        ulz+=(float)b[i]/a[i];
        ulz*=1000;
        ulz=floor(ulz);
        ulz/=1000;
        // printf("ULZ check  ulz:%.5f\n",ulz);
        
        
        
        if(ulz<ULZ && ULZ-ulz<0.1 && ulz!=ULZ){
            //printf("$$$$$$$$$$$$$$$$$$$mathing NOW!!\n");
            float tmp2=-1;
            int cnt1=1;
            int cnt2=2;
            float seisu=0;
            do {
                tmp2=ULZ-ulz;
                tmp2*=1000;
                tmp2=floor(tmp2);
                tmp2/=1000;
                tmp2*=pow(10,cnt1);
                //printf("$$$$$$ %f ",tmp2);
                seisu=floor(tmp2);
                //printf("%f \n",seisu);
                cnt2--;
                cnt1++;
            }while(tmp2!=seisu);
            
            if(tmp2==seisu){
                i++;
                b[i]=(int)tmp2+1;
                a[i]=(int)pow(10,(double)cnt1-1);
                //	printf("$$$$$$MATCH a[%d]=%d,b[%d]=%d\n",i,a[i],i,b[i]);
                ulz+=(float)b[i]/a[i];
                ulz*=100;
                ulz=floor(ulz);
                ulz/=100;
                
                // printf("ULZ check  ulz:%.5f\n",ulz);
                
                
            }
        }
        if(ulz>ULZ){
            ulz-=(float)b[i]/a[i];
            // printf("Over %.2f!\nSerach a[%d],b[%d] again!\nULZ back to old ulz: %.5f\n",ULZ,i,i,ulz);
        }
        else
            i++;
        //sleep(1);
    }while(ulz!=ULZ);
    // printf("\n*******************************\n*         TASK NUMB:%d\n*******************************\n",i);
    //   for(j=0;j<i;j++)
    // printf("%d %d\n",a[j],b[j]);
    // printf("*******************************\nulz=%.3f\n*******************************\n",ulz);
    
    return i;
    
    
}

int SUFA(Task task_set[],int tNum)
{
    int Ui = 0;
    int Umax = 0;
    int Sum_Ui = 0;
    int i;
    int success_flag = 0;//1なら成功、0なら失敗
    
    for(i=0;i<tNum;i++)
    {
        
        Ui = task_set[i].runtimeRemaining * 100 / task_set[i].deadline;
        Sum_Ui = Sum_Ui + Ui;
        
        if(Umax < Ui)
        {
            Umax = Ui;
        }
        
    }
    
    if(Sum_Ui <= (pNum * (100-Umax)) + Umax )
    {
        success_flag = 1;
    }
    
   // printf("Umaxの値%d,Uiの合計利用率%d,右辺の合計利用率%d\n",Umax,Sum_Ui,(pNum * (100-Umax)) + Umax);
    
    return success_flag;
}




