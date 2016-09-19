(* ::Package:: *)

BeginPackage["queues`"]

(*Rand::usage = "Random number";*)
Stire::usage = "Stire[array] Genera una versi\[OAcute]n escalonada de un array en formato {x,y}";
ArrivalAcumSeries::usage = "ArrivalAcumserires[lmb,mu,n] genera una lista de n paquetes c
							on una distribuci\[OAcute]n de llegad lambada y un distribuci\[OAcute]n de tama\[NTilde]o mu";
FifoDepartureTime1::usage= "FifoDeparturetiem1[array] completa el array de llegadas simulando una
							cola M/M/1";
Queue1Sim::usage= "Queue1Sim[lmb,mu,n] simula una cola M/M/1 con n paquetes, distribuci\[OAcute]n de
					llegada lambda y distribuci\[OAcute]n de servicio mu";
FifoDepartureTime2::usage="FifoDepartureTime2[array] completa el array de llegada simulando una
							cola M/M/2";
Queue2Sim::usage="Queue2Sim[lmb,mu,n] simula una cola M/M/2 con n paquetes, distribuci\[OAcute]n de
				llegada lambada y distribuci\[OAcute]n de servicio mu";
FifoDepartureTime1N::usage="FifoDepartureTime1N[array, N] completa el array de llegadas simulando
				una cola M/M/1/N";
Queue1NSim::usage="Queue1NSim[lmb,mu,n,N] simula una cola M/M/1/N con n paquetes, distribuci\[OAcute]n
				de llegada lambada y distribuci\[OAcute]n de servicio mu";
StatusAction::usage="StatusAction[queue] calcula las acciones de subida y bada de estados en una
				lista de paquetes que represente una cola ";
StateTime::usage="StateTime[queue] Devuelve el estado de la cola en cada momento";
StatusProbPasta::usage="StatusProbPasta[queue] Calcula la probabilidad de estado seg\[UAcute]n
						la propiedad Pasta de la distribuci\[OAcute]n de Poisson";
StatusProbTime::usage="StatusProbPasta[queue] Calcula la probabilidad de estado de una
					cola en funci\[OAcute]n del tiempo en cada estado";
MeanSystemTime::usage="MeanSystemTime[queue]Calcula el tiempo medio de estado en el sistema de una cola";
QueueThAcum::usage="QueueThAcum[queue] Calcula el throughput de la cola en pkt/s";

Begin["`private`"]
Module[{Z0=2000,a=314159269, c=453806245,m=2^21},
(*Funci\[OAcute]n que escalona un array de elementos del tipo {x,y}*)
Stire[array_]:=Module[{last={0,0}},Prepend[Flatten[Map[{{#[[1]],last[[2]]},last=#}&,array],1],{0,0}]];
(*Funci\[OAcute]n de generaci\[OAcute]n de llegadas*)
ArrivalAcumSeries[lmb_,m_,n_]:=Module[{t=lmb,nsec=0},NestWhileList[{t+=lmb,t,m,++nsec}&,{t,t,m,nsec},(#[[4]]<n)&]];
SetAttributes[ArrivalAcumSeries,HoldAll];
(*Funciones de cola con un solo servidor*)
FifoDepartureTime1[arrivals_]:=Module[{t=0,last=0},
Map[(If[(t+last)>=#[[1]],{#[[1]],t+=last,last=#[[3]],#[[4]]},{#[[1]],t=#[[1]],last=#[[3]],#[[4]]}])&,arrivals]];
Queue1Sim[lmb_,m_,n_]:=FifoDepartureTime1[ArrivalAcumSeries[lmb,m,n]];
SetAttributes[Queue1Sim,HoldAll];
(*Funciones de cola con dos servidores*)
FifoDepartureTime2[arrivals_]:=Module[{t1=0,t2=0,last1=0,last2=0,min},
Map[If[(min=Min[(t1+last1),(t2+last2)])==(t1+last1),
If[min>=#[[1]],{#[[1]],t1+=last1,last1=#[[3]],#[[4]]},{#[[1]],t1=#[[1]],last1=#[[3]],#[[4]]}],
If[min>=#[[1]],{#[[1]],t2+=last2,last2=#[[3]],#[[4]]},{#[[1]],t2=#[[1]],last2=#[[3]],#[[4]]}]]&,arrivals]];
Queue2Sim[lmb_,m_,n_]:=FifoDepartureTime2[ArrivalAcumSeries[lmb,m,n]];
SetAttributes[Queue2Sim,HoldAll];
(*Funciones de cola con un servidor y cola finita*)
FifoDepartureTime1N[arrivals_,length_]:=Module[{t=0,last=0,result={arrivals[[1]]}},
GetAceptance[pkt_]:=Module[{status=StateTime[result],lastState},
Do[If[n[[1]]<=pkt[[1]],lastState=n[[2]],Break],{n,status}];
If[lastState< length,1,0]
];
Do[If[GetAceptance[n]==1,(*Aceptado*)If[(t+last)>=n[[1]],result=Append[result,{n[[1]],t+=last,last=n[[3]],n[[4]]}],
result=Append[result,{n[[1]],t=n[[1]],last=n[[3]],n[[4]]}]]
,(*Descartado*)result=Append[result,{n[[1]],Null,n[[3]],n[[4]]}]],{n,arrivals[[2;;]]}];
Return[result]
];
Queue1NSim[lmb_,m_,n_,leng_]:=FifoDepartureTime1N[ArrivalAcumSeries[lmb,m,n],leng];
SetAttributes[Queue1NSim,HoldAll];
(*Funci\[OAcute]n para calcular el estado del sistema*)
(*StatusAction[queue_]:=Union[Map[{#[[1]],1}&,queue],Map[{(#[[2]]+#[[3]]),-1}&,queue]];*)
StatusAction[queue_]:=Module[{up={},down={}},
Do[If[n[[2]]===Null,,up=Append[up,{n[[1]],1}]],{n,queue}];
Do[If[n[[2]]===Null,,down=Append[down,{n[[2]]+n[[3]],-1}]],{n,queue}];
Union[up,down]
];
StateTime[queue_]:= Module[{pkt=0},Map[{#[[1]],pkt+=#[[2]]}&,StatusAction[queue]]];
StatusProbPasta[queue_]:=Module[{Sstatus=StateTime[queue],MaxStatus,Status,actState=0,length=0},
Do[If[n===Null,,length+=1],{n,queue}];
MaxStatus = Max[Map[#[[2]]&,Sstatus]];
Status =Module[{n=0}, NestWhileList[{n++,0}&,{n++,0},(#[[1]]<MaxStatus)&]];
Do[If[n[[2]]>actState,Status[[(actState=n[[2]])]][[2]]+=1,actState=n[[2]]],{n,Sstatus}];
Map[{#[[1]],#[[2]]/length//N}&,Status]
];
StatusProbTime[queue_]:=Module[{Sstatus=StateTime[queue],MaxStatus,Status,finishTime,last={0,0}},
Iterate[n_]:=(Status[[last[[2]]+1]][[2]]+=(n[[1]]-last[[1]]);last=n);
finishTime = Last[Sstatus][[1]];
MaxStatus = Max[Map[#[[2]]&,Sstatus]];
Status =Module[{n=0}, NestWhileList[{n++,0}&,{n++,0},(#[[1]]<MaxStatus)&]];
Do[Iterate[n],{n,Sstatus}];
Map[{#[[1]],#[[2]]/finishTime//N}&,Status]
];
(*Funci\[OAcute]n para calcular el tiempo medio en el sistema*)
(*MeanSystemTime[queue_]:=Mean[Map[((#[[2]]+#[[3]])-#[[1]])&,queue]];*)
MeanSystemTime[queue_]:=Module[{x={}},Do[If[n[[2]]===Null,,AppendTo[x,((n[[2]]+n[[3]])-n[[1]])]],{n,queue}];Mean[x]];
QueueThAcum[lpkt_]:=Module[{last={0,0}},Map[If[#[[2]]===Null,last={#[[2]],(last[[1]]*last[[2]])/#[[2]]},last={#[[2]],#[[4]]/#[[2]]}]&,lpkt]];
]
End[]
EndPackage[]









