/*
 ============================================================================
 Name        : RTA_EDF.c
 Author      :
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

#define pNum 4//プロセッサ数
#define SUFS 30
#define SUFE 50
#define SUFI 1
#define RFF 1000//ReadFileFolder フォルダ1つにつき何個のファイルを読み込むか（システム利用率毎に何個のファイルを読み込ませるか）

typedef struct//構造体の宣言 タスク
{
	int runtime;//実行時間
	int period;//タスクの周期
	int index;//タスクの番号
	int runtimeRemaining;//タスクの残り実行時間
	int dover;//タスクのデッドラインオーバー量
	int dmiss;//タスクがデッドラインミスをした回数
	int useflag;//複数のプロセッサで同じタスクが実行されないようにするためのフラグ
	int endflag;//タスクが実行終了したときのスケジュール呼び出しを一回だけにするためのフラグ
	int pi;//前の時間にどのプロセッサで実行されていたかを覚える変数
}Task;

int RTA(Task task_set[],int tNum); //RTA

int Most_min(int x,int y, int z);//RTA

int Workload_RK(Task task_i,int L);//RTA

int Workload_DK(Task task_k,Task task_i);//RTA

int The_Absolute_value(int x);//RTA

int Start_Scheduling(Task task_set[],int tNum,int sw[],int sss[]);

void Dispatcher(Task task_set[],Task *pro[],Task processor_idol,int time,int tNum,int sw[]);

int Get_Task_Num(char fn[]);

void Read_Task_Auto(Task x[],int tNum, char fn[]);

int Get_Hyper_Period(Task x[],int tNum);

int lcm(int x, int y);

int gcd(int x, int y);

void EDF_Sort(Task x[], int n, int time);

void Result_Sort(Task k[], int n);

void File_Read(char rtf[],int suf,int rt);


//メイン関数。主に結果表示に関することを記述している
int main(void)
{
	int i;//ループ用変数
	int tNum;//タスクの数
	int deadlinemiss = 0;//1タスクセットのデッドラインミス回数の合計
	int deadlineover = 0;//1タスクセットのデッドラインオーバー量の合計
	int sw[1] = {0};//コンテキストスイッチの回数
	int sss[1] = {0};//SchedulerStartSum スケジューラ起動回数
	int job_num = 0;//全ジョブの数
	int suf;//現在のシステム利用率
	int read_taskfile = 0;
	int success_all = 0;
	int dm_all = 0;
	int do_all = 0;

	int rta_flag = 0; //RTA
	int rta_success_all = 0;//RTA

	FILE *outputfile;

	char fn[100];
	char rtfd[100];//ReadTasksetFileDirectoryファイル読み込み用

	sprintf(fn,"C:/Users/RYO/Documents/Testdata/processor%dRTA_EDF.csv",pNum);

	outputfile = fopen(fn,"w");
	if (outputfile == NULL)
	{
		printf("エラー書き込み\n");
		exit(1);
	}

	fprintf(outputfile,"%dタスクセットを与えた結果\n",RFF);
	fprintf(outputfile,"システム利用率,反応時間解析での成功率,スケジュール成功率,ジョブの成功率,コンテキストスイッチ回数,デッドラインオーバー量,スケジューラ起動回数\n");

	for(suf=SUFS;suf<=SUFE;suf=suf+SUFI)
	{
		sw[0] = 0;
		sss[0] = 0;
		dm_all = 0;
		do_all = 0;
		job_num = 0;
		success_all = 0;
		rta_success_all = 0;

		for(read_taskfile=1;read_taskfile<=RFF;read_taskfile++)
		{
			File_Read(rtfd,suf,read_taskfile);

			//printf("%s",rtfd);

			tNum = Get_Task_Num(rtfd);//タスクの数を取得

			Task task_set[tNum];//タスクの数が分かったので、ここで必要なタスク分の領域だけ確保する。

			Read_Task_Auto(task_set, tNum, rtfd);

			/*printf("読み込んだタスクセットの表示\n");
			for(i=0;i<tNum;i++)
			{
				printf("タスク%d 実行時間%d 周期%d\n",task_set[i].index,task_set[i].runtime,task_set[i].period);
			}*/

			rta_flag = RTA(task_set,tNum);//RTA

			if(rta_flag == 1)
			{
				rta_success_all++;
			}

			job_num = job_num + Start_Scheduling(task_set,tNum,sw,sss);//タスクの実行時間、周期が決まったのでスケジューリングスタート。

			deadlinemiss = 0;
			deadlineover = 0;

			for(i=0;i<tNum;i++)
			{
				//各タスクがどのくらいデッドラインミスをしてオーバーしたかの詳細がしりたい場合
				//printf("タスク%dのデッドラインミス回数 %d回 デッドラインオーバー量 %d単位時間\n",task_set[i].index,task_set[i].dmiss,task_set[i].dover);

				deadlinemiss = deadlinemiss + task_set[i].dmiss;
				deadlineover = deadlineover + task_set[i].dover;
			}//各タスクの詳細ここまで




			//以下は結果の出力 必要なものだけ使う

			//スケジュール成功か失敗の判定
			if(deadlinemiss == 0)
			{
				//printf("スケジュール成功\n");
				success_all++;
			}
			else
			{
				//printf("スケジュール失敗\n");
			}

			dm_all = dm_all + deadlinemiss;
			do_all = do_all + deadlineover;
		}

		fprintf(outputfile,"%d,%f,%f,%f,%f,%f,%f\n",suf,(float)rta_success_all*100/(read_taskfile - 1),(float)success_all*100/(read_taskfile - 1), (float)(job_num-dm_all)*100/job_num,(float)sw[0]/(read_taskfile - 1),(float)do_all/(read_taskfile-1),(float)sss[0]/(read_taskfile - 1));
	}

	fclose(outputfile);

	return 0;
}

int RTA(Task task_set[],int tNum)//RTA
{
	int Rk = 0;//タスクkの反応時間の値
	int Rkprev = 0;//タスクkの前の反応時間の値
	int Ck = 0;//タスクkの実行時間
	int Tk = 0;
	int Lk = 0;//タスクkの最低余裕時間
	int count = 0;
	int success_flag = 1;//1なら成功、0なら失敗
	int i,k;
	int interference_sum = 0;//干渉長の合計

	for(k=0;k<tNum;k++)
	{
		Ck = task_set[k].runtime;//タスクkの実行時間を入れる
		Rk = task_set[k].runtime;//反応時間は最小でも実行時間分かかるので初期値は実行時間分となる
		Tk = task_set[k].period;
		count = 0;
		do
		{
			Rkprev = Rk;
			count++;
			interference_sum = 0;

			for(i=0;i<tNum;i++)
			{
				if(i != k)
				{
					interference_sum = interference_sum + Most_min( Workload_RK(task_set[i],Rk), Workload_DK(task_set[k], task_set[i]),(Rk-Ck+1) );
				}
			}

			Rk = Ck + (interference_sum / pNum);

		}while( !(The_Absolute_value(Rkprev - Rk) < 1) && count < 100000);


		Lk = Tk - Rk;

		if(Lk < 0)
		{
			success_flag = 0;
		}
	}

	return success_flag;

}

int Most_min(int x,int y,int z)//RTA
{
	if(x <= y && x <= z)
	{
		return x;
	}

	if(y <= z)
	{
		return y;
	}
	else
	{
		return z;
	}
}

int Workload_RK(Task task_i,int L)//RTA
{
	int Ni = 0;
	int Ti = task_i.period;
	int Ci = task_i.runtime;
	int W;
	int min;

	Ni = (L + Ti - Ci)/Ti;

	if(Ci <= (L + Ti - Ci - (Ni*Ti)) )
	{
		min = Ci;
	}
	else
	{
		min = (L + Ti - Ci - (Ni*Ti));
	}

	W = (Ni*Ci) + min;

	return W;
}

int Workload_DK(Task task_k,Task task_i)//RTA
{
	int Ni = 0;
	int Ti = task_i.period;
	int Tk = task_k.period;
	int Ci = task_i.runtime;
	int W;
	int max = 0;
	int min;

	Ni = Tk/Ti;

	if((Tk - (Ni * Ti)) > max)
	{
		max = (Tk - (Ni * Ti));
	}

	if(Ci <= max )
	{
		min = Ci;
	}
	else
	{
		min = max;
	}

	W = (Ni*Ci) + min;

	return W;
}

int The_Absolute_value(int x)//RTA
{

	if(x < 0)
	{
		x = (-1)*x;
	}

	return x;
}


// スケジューリングの関数
int Start_Scheduling(Task task_set[],int tNum, int sw[], int sss[])
{
	int i,j;	//forループ用変数
	int time;	//現在の時刻
	int hyper_period;	//ハイパー周期の値。全タスクの周期の最小公倍数で求まり、この時間までスケジュールする
	int sflag = 0;	//スケジューラ起動判定フラグ。0ならスケジューラを起動しない。1ならスケジューラを起動する
	int job_num=0;	//全ジョブの数

	Task processor_idol;	//プロセッサのアイドル状態を表す
	processor_idol.runtimeRemaining = 0;	//便宜上に設定している
	processor_idol.index = tNum +1;	//すべてのタスク数＋1という値がこのプログラムでのアイドル状態を表す。

	Task *pro[pNum];	//プロセッサ。実行するタスクと同じアドレスにアクセスさせるために今回はポインタを用いる

	//プロセッサの初期化（プロセッサの値をアイドル状態にする）
	for(i=0;i<pNum;i++)
	{
		pro[i] = &processor_idol;
	}

	//全タスクの残りの実行時間、デッドラインオーバー量、デッドラインミス回数、前時刻でどのプロセッサで実行されていたかの初期化
	for(i=0;i<tNum;i++)
	{
		task_set[i].runtimeRemaining = 0;
		task_set[i].dover = 0;
		task_set[i].dmiss = 0;
		task_set[i].pi = pNum + 1;
	}


	//初期設定終了。以下より動作に入る


	//ハイパー周期の取得
	hyper_period = Get_Hyper_Period(task_set, tNum);
	//printf("ハイパー周期%d\n\n",hyper_period);

	for(time=0;time<hyper_period;time++)
	{
		//printf("時刻%d\n",time);//時刻確認用

		//スケジューラーの起動判定およびタスクのデッドラインミス判定、タスクの起動確認してからの実行時間調整
		for(i=0;i<tNum;i++)
		{
			task_set[i].useflag = 0;//全タスクについてのフラグを初期化

			//タスクの起動時刻になったとき
			if( time % task_set[i].period == 0)
			{
				//printf("タスク%dが起動\n",task_set[i].index);	//タスクの起動確認用

				job_num++; //新しくジョブが起動されたので、ジョブの数を+1する

				//タスクが起動するとき、前のジョブの残り実行時間があれば、デッドラインミス
				if(task_set[i].runtimeRemaining != 0)
				{
					//printf("タスク%dがデッドラインミス\n",tasks[i].index); //どのタスクがデッドラインミスを起こしたか表示
					//printf("デッドラインオーバー量%d\n",tasks[i].runtimeRemaining);	そのタスクがどれくらいオーバーしたかを表示
					task_set[i].dmiss++;	//タスクが失敗したのでデッドラインミス回数を+1する
					task_set[i].dover = task_set[i].dover + task_set[i].runtimeRemaining;	//オーバーした時間だけデッドラインオーバー量にプラスする
				}

				//タスクの残り実行時間をそのタスクの実行時間にする
				task_set[i].runtimeRemaining = task_set[i].runtime;

				task_set[i].endflag = 0;	//エンドフラグ初期化

				sflag = 1;	//スケジューラー起動
			}//タスクの起動確認終了

			//タスクの残り実行時間がゼロになったタスクが発生したなら、スケジューラー起動
			if(task_set[i].runtimeRemaining == 0 && task_set[i].endflag == 0)
			{
				sflag = 1;	//スケジューラー起動

				task_set[i].endflag = 1;	//そのタスクが実行終了したことを表す

				//printf("タスク%dが終了\n",task_set[i].index);	//タスクの実行終了確認用
			}
		}//スケジューラーの起動判定〜の作業はここまで

		//スケジューラが起動されたら
		if(sflag == 1)
		{
			sss[0]++;

			//printf("スケジューラー起動\n"); //スケジューラ起動確認用

			//優先度順に配列を並び替える(配列の番号が若いものほど優先度が高くなる)
			EDF_Sort(task_set,tNum,time);

			/*//タスクの優先順番確認用
			for(i=0;i<tNum;i++)
			{
				if(task_set[i].runtimeRemaining != 0)
				{
					printf("タスク番号%d 残りの実行時間%d デッドライン%d\n",task_set[i].index,task_set[i].runtimeRemaining,(time+task_set[i].period - (time %task_set[i].period)));
				}
			}*/

			//優先度変更で前の時間に実行していたタスクのアドレスが変わってしまったので、正しいアドレスにアクセスする処理
			for(j=0;j<tNum;j++)
			{
				if(task_set[j].pi != pNum + 1)
				{
					pro[task_set[j].pi] = &task_set[j];
				}
			}

			//ディスパッチャの起動
			Dispatcher(task_set,pro,processor_idol,time,tNum,sw);

			sflag = 0;	//スケジューラ起動の処理が終わったので、フラグを初期化
		}

		//スケジューラーが起動されなかった場合
		else
		{
			/*//タスクの優先順番確認用
			for(i=0;i<tNum;i++)
			{
				if(task_set[i].runtimeRemaining != 0)
				{
					printf("タスク番号%d 残りの実行時間%d デッドライン%d\n",task_set[i].index,task_set[i].runtimeRemaining,(time+task_set[i].period - (time %task_set[i].period)));
				}
			}*/

			//前時刻と同じタスクを現時刻でも引き続き実行を行う
			for(i=0;i<pNum;i++)
			{
				if(pro[i]->runtimeRemaining != 0)
				{
					//前時刻のタスクは現時刻でも実行されるのでそのタスクの残り実行時間をマイナス1する
					pro[i]->runtimeRemaining--;
				}
			}
		}//スケジューラーが起動されなかった場合ここまで

		/*//現時刻でのプロセッサの状態を表示
		for(i=0;i<pNum;i++)
		{
			if(pro[i]->index != tNum + 1)
				printf("プロセッサ%d 実行されるタスク%d\n",i,pro[i]->index);
			else
				printf("プロセッサ%d アイドル状態\n",i);
		}

		printf("\n");*/

	}//timeの閉じ(スケジューリング終了)

	//タスクの起動時に失敗するかどうか判定していたため、最後の一回ぷんの判定が残っているのでそれの処理
	for(i=0;i<tNum;i++)
	{
		if(task_set[i].runtimeRemaining != 0)
		{
			task_set[i].dover = task_set[i].dover + task_set[i].runtimeRemaining;
			task_set[i].dmiss++;
			//printf("タスク%dがデッドラインミス,",task_set[i].index);
			//printf("デッドラインオーバー量%d\n",task_set[i].runtimeRemaining);
		}
	}

	//printf("\n");

	//Result_Sort(task_set,tNum);

	return job_num; //ジョブがすべてで何個あったかをmain関数に返す
}

// コンテキストスイッチの発生、CPUにタスクを割り当てるなどのディスパッチャの動作をしている関数
void Dispatcher(Task task_set[],Task *pro[],Task processor_idol,int time,int tNum,int sw[])
{
	int i,j;
	int pm_useflag[pNum];	//プロセッサが使われているか使われていないかを管理するフラグ

	//プロセッサを空いている状態へ
	for(i=0;i<pNum;i++)
	{
		pm_useflag[i] = 0;
	}


	//無駄なコンテキストスイッチを発生させないように
	//現時刻で実行されるタスクの中で前時刻で実行されていたタスクがあれば
	//同じプロセッサで割り当てるようにする作業

	j=0;
	for(i=0;i<tNum;i++)
	{
		if(task_set[i].runtimeRemaining != 0)
		{
			//pNum + 1は前回実行されていなかったタスクを表す
			//したがってタスクが実行されている、かつ自分よりも優先度が高いタスクがプロセッサの数より少なかったら
			if(task_set[i].pi != pNum + 1 && j < pNum)
			{
				//タスクの実行が行われるので、そのタスクの残りの実行時間を1引く
				task_set[i].runtimeRemaining--;

				//これ以上このプロセッサに割り当てないようフラグを1にする
				pm_useflag[task_set[i].pi] = 1;

				//このタスクが複数のプロセッサに割り当てられないようフラグを1にする
				task_set[i].useflag = 1;

				j++;

				//前回に引き続き実行されるタスク確認用
				//printf("前回に引き続き実行されるタスク%d\n",task_set[j].index);
			}
			else
			{
				task_set[i].pi = pNum + 1;
				j++;
			}
		}
		else
		{
			task_set[i].pi = pNum + 1;
		}
	}

	//余っているプロセッサにタスクを割り当てる
	for(i=0;i<pNum;i++)
	{
		//余っているプロセッサがあれば
		if(pm_useflag[i] == 0)
		{
			for(j=0;j<tNum;j++)
			{
				//実行終了していないタスクの中で優先度が高いタスクを割り当てる
				if(task_set[j].useflag == 0 && task_set[j].endflag == 0)
				{
					//余っているプロセッサに入るタスク確認用
					//printf("余っているプロセッサ%dに入るタスク%d\n",i,tasks[j].index);

					//コンテキストスイッチの発生調べ
					//プロセッサがアイドル状態でないなら
					if(pro[i]->index != tNum +1)
					{
						//前時刻で実行していたタスクが実行終了していなかったら
						if(pro[i]->runtimeRemaining != 0 && (time%pro[i]->period != 0))
						{
							sw[0]++;	//コンテキストスイッチの発生

							//確認用
							//printf("コンテキストスイッチ発生 タスク番号%dがタスク番号%dから奪った\n",task_set[j].index,pro[i]->index);
						}
					}//コンテキストスイッチの発生調べここまで

					pro[i] =  &task_set[j];	//現時刻で実行されるタスクをプロセッサに格納

					task_set[j].pi = i;	//どのプロセッサで実行されたかを格納

					task_set[j].runtimeRemaining--;	//実行されるタスクの残り実行時間をマイナス1する

					task_set[j].useflag = 1;	//このタスクが複数のプロセッサに割り当てられないようフラグを1にする

					pm_useflag[i] = 1;	//これ以上このプロセッサに割り当てないようフラグを1にする

					j = tNum;	//このプロセッサに複数のタスクが実行されないようループを終了させるためのもの

				}//優先度が高いタスク割り当てここまで
			}//全タスク調べここまで
		}//プロセッサが余っていたらここまで

		//上の処理を実行してまだプロセッサが余っていたら
		if(pm_useflag[i] == 0)
		{
			pro[i] = &processor_idol;	//プロセッサをアイドル状態へ
		}
	}//プロセッサが余っているならの閉じかっこ
}

// タスクの数を求める関数
int Get_Task_Num(char fn[])
{
	int tNum=0;


	FILE *fp;//読み込むファイル
	char tmp[100];//行数を得るための配列　RF

	//行数を獲得するための作業
	fp=fopen(fn,"r");
	if(fp==NULL)
	{
		printf("ファイルを開くことが出来ませんでした．\n");
		return 0;
	}

	//タスクの数を獲得するためのループ
	while( fgets(tmp, 100, fp) != NULL )
	{
		tNum++;
	}

	fclose(fp);//タスクの数を獲得するための作業終了


	return tNum;
}

// // タスクの情報をファイルから読み込むための関数。ここでタスクの実行時間・周期をファイルから読み込み、タスク番号をつける
void Read_Task_Auto(Task x[], int tNum, char fn[])
{
	FILE *fp;//読み込むファイル
	int n = 0;

	//puts(fn);
	//タスクセットファイルから実行時間、周期の読み込み作業
	fp=fopen(fn,"r");
	if(fp==NULL)
	{
		printf("Read_task_Auto ファイルを開くことが出来ませんでした\n");
	}
	//ファイルが終わりでないうちは，読み込みを続ける
	while (n<tNum)
	{
		fscanf(fp,"%d,%d",&x[n].runtime,&x[n].period);
		x[n].index = n;
		n++;
	}

	fclose(fp);//タスクセットファイルから実行時間、周期の読み込み作業終了
}

// ハイパー周期を求める関数
int Get_Hyper_Period(Task x[], int tNum)
{
	int i;
	int hyper_period;

	if(tNum != 1)
	{
		//周期の計算
		for(i=0;i<tNum;i++)
		{
			if(i==0)
			{
				hyper_period = lcm(x[i].period,x[i+1].period);
				i++;
			}
			else
			{
				//先ほど求めた2つの最小公倍数と新しいタスクの周期の最小公倍数を求める
				hyper_period = lcm(hyper_period,x[i].period);
			}
		}
	}
	else
	{
		hyper_period = x[0].period;
	}

	return hyper_period;
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

// デッドラインが早い順にソートする
void EDF_Sort(Task x[], int n, int time)
{
	int i, j;
	Task temp;

	//デッドラインが早いものほど、優先度を高く（配列の番号が若い順に）する作業
	for(i=0;i<n-1;i++)
	{
		for(j=n-1;j>i;j--)
		{
			//デッドラインが早いものを配列の前へ
			if ( (time+x[j-1].period - (time %x[j-1].period)) > (time+x[j].period - (time %x[j].period)) )
			{
				/*交換する */
				temp=x[j];
				x[j]=x[j-1];
				x[j-1]=temp;
			}
			//デッドラインが同じならばタスク番号が若い方を配列の前へ
			else if( (time+x[j-1].period - (time %x[j-1].period)) == (time+x[j].period - (time %x[j].period)) )
			{
				if(x[j-1].index > x[j].index)
				{
					//交換する
					temp=x[j];
					x[j]=x[j-1];
					x[j-1]=temp;
				}
			}
		}
	}//デッドラインが早いものほど、優先度を高く（配列の番号が若い順に）する作業の閉じ
}

// 結果表示用にタスク番号が若い順に配列を並び替える関数
void Result_Sort(Task x[], int n)
{
	int i, j;
	Task temp;

	//デッドラインが早いものほど、優先度を高く（配列の番号が若い順に）する作業
	for(i=0;i<n-1;i++)
	{
		for(j=n-1;j>i;j--)
		{
			if(x[j-1].index > x[j].index)
			{
				//交換する
				temp=x[j];
				x[j]=x[j-1];
				x[j-1]=temp;
			}
		}
	}//デッドラインが早いものほど、優先度を高く（配列の番号が若い順に）する作業の閉じ
}

void File_Read(char rtfd[],int suf,int rt)
{
	char ntcp_directory[] = "C:/Users/RYO/Documents/Testdata";//newtaskcreateプログラムが保管されているディレクトリ

	if(suf < 100)
	{
		if(rt < 10)
		{
			sprintf(rtfd,"%s/p%d/0%d/000%d.csv",ntcp_directory,pNum,suf,rt);
		}
		else if(rt < 100)
		{
			sprintf(rtfd,"%s/p%d/0%d/00%d.csv",ntcp_directory,pNum,suf,rt);
		}
		else if(rt < 1000)
		{
			sprintf(rtfd,"%s/p%d/0%d/0%d.csv",ntcp_directory,pNum,suf,rt);
		}
		else
		{
			sprintf(rtfd,"%s/p%d/0%d/%d.csv",ntcp_directory,pNum,suf,rt);
		}
	}

	//sufが100ならば
	else
	{
		if(rt < 10)
		{
			sprintf(rtfd,"%s/p%d/%d/000%d.csv",ntcp_directory,pNum,suf,rt);
		}
		else if(rt < 100)
		{
			sprintf(rtfd,"%s/p%d/%d/00%d.csv",ntcp_directory,pNum,suf,rt);
		}
		else if(rt < 1000)
		{
			sprintf(rtfd,"%s/p%d/%d/0%d.csv",ntcp_directory,pNum,suf,rt);
		}
		else
		{
			sprintf(rtfd,"%s/p%d/%d/%d.csv",ntcp_directory,pNum,suf,rt);
		}
	}
}
