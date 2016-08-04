(* ::Package:: *)

BeginPackage["simulateTx`"]

(*Funciones*)
SetInitPar::usage="SetInitPar[speed,pt,ackt] inicializa los par\[AAcute]metros del sistema utilizado (no la parte gr\[AAcute]fica)";
(*PacketLength::usage="PacketLength[lmb] devuelve el tama\[NTilde]o de un paquete seg\[UAcute]n el tiempo dado dada";*)
GetPacketArrival::usage="GetPacketArrival[lmb,mu,initTime,n] Genera la llegada de N paquetes que comienzan a llegar
				 en initTime llegan con tasa lmb y un timpo de servicio mu";
FifoSW::usage="FifoSW[lpkt,error] simula el paso por un servicio FIFO Stop & Wait con un probabilidad de error 'erro'";
ToTime::usage="ToTime[lpkt] pasa a tiempo la lista de paquetes que est\[AAcute] en modo tama\[NTilde]o de paquete";
LaunchSimTxSW::usage="LaunchSimTxSW[tasa,tp,p,n,lamba] realiza la simulaci\[OAcute]n completa de un sistema FIFO Stop & Wait
					tasa de llegada, tiempo de propagaci\[OAcute]n tp, error p, paquetes n y tiempo de servicio lambda";
FifoGBN::usage="FifoGBN[lpkt,error] simula el paso por un servicio FIFO Go Back N con una probabilida de error 'error'";
LaunchSimTxGBN::usage="LaunchSimTxGBN[tasa,tp,p,n,lamba] realiza la simulaci\[OAcute]n completa de un sistema FIFO Go BackN
					tasa de llegada, tiempo de propagaci\[OAcute]n tp, error p , paquetes n y tiempo de servicio lamba";
Begin["`private`"]

Module[{lspeed=1,tp,tack,RndNumber = 4406,a=314159269,c=453806245,m=2^31},
(*Array de packete
[1]Tiempo de llegada
[2]Tiempo de insercion (o tama\[NTilde]o de paquete)
[3]Numero de secuencia
[4]Error
[5]N\[UAcute]mero de repeticion
*)
SetInitPar[speed_,pt_,ackt_]:=(lspeed=speed;tp=pt;tack=ackt);
PacketLength[lmb_]:=Module[{x},If[(x=Round[lmb*lspeed])==0,1,x]];

(*Obtener los paquetes*)
GetPacketArrival[lmb_,mu_,initTime_:0,n_]:=NestWhileList[{acumTime+=lmb,PacketLength[mu],#[[3]]+1,0,0}&,{acumTime=initTime+lmb,PacketLength[mu],0,0,0},(#[[3]]<n)&];
SetAttributes[GetPacketArrival,HoldAll];

(*Cola SW*)
FifoSW[lpkt_,error_]:=Module[{time=lpkt[[1,1]]},
GetDepartureSW[pkt_]:=Module[{return=pkt[[1]]},(If[time>=pkt[[1]],return = time;time+=(pkt[[2]]/lspeed),time=pkt[[1]]+(pkt[[2]]/lspeed)];time+=2*tp+tack;return)];
GetPacketTxSW[pkt_]:={GetDepartureSW[pkt],pkt[[2]],pkt[[3]],If[Random[]<error,1,0],pkt[[5]]+1};
Flatten[Map[NestWhileList[GetPacketTxSW[#]&,GetPacketTxSW[#],(#[[4]]==1)&]&,lpkt],1]
];

(*Pasar a tiempo*)
ToTime[pkt_]:=Map[{#[[1]],#[[2]]/lspeed,#[[3]],#[[4]],#[[5]]}&,pkt];

(*Funci\[OAcute]n para lanzar una simulaic\[OAcute]n de SW*)
LaunchSimTxSW[tasa_,tp_,p_,n_,lambda_]:=(SetInitPar[1,tp,0];ToTime[FifoSW[GetPacketArrival[tasa,lambda,0,n],p]]);
SetAttributes[LaunchSimTxSW,HoldAll];

(*Cola GBN*)
FifoGBN[arrivals_,pcomb_]:=
(*Se inicializa el checktime al momento de llegada del primer paquete*)
Module[{checkTime,nrTx,checkIni,endTime},checkTime=arrivals[[1,1]];
(*Devuelve un paquete dentro del periodo de error, esto es, paquetes que se tendr\[AAcute]n que retrasmitir ya que ha habido un paquete err\[OAcute]neo con anterioridad,
ini=cuando llega el paquete,
end=cuando termina el periodo de error,
perror=probabilidad de error,
pck=paquete*)
GetPacketInBadPeriod[ini_,end_,perror_,pck_]:=(
If[ini+pck[[2]]>end,checkTime =ini+pck[[2]],checkTime=checkTime];
(*Sin desalojo*)
(*Se rellena el paquete con la posibilidad de que sea err\[OAcute]neo*)
{ini,pck[[2]],pck[[3]],If[Random[]<perror,3,2],pck[[5]]+1}
);

(*
pck=pakete,
npck=indice,
perror= probabilidad de error,
arr=lista de paquetes,
Devuelve el paquete enviado y si hay error todos los paquetes en el periodo del error*)
GetPacketRTxGBN[pck_,npck_,perror_,arr_]:=
(*CheckIni es el equivalente a checktiem dentro de la ventan de error ya que este se usar\[AAcute] para comprobar el final de la ventana de error*)
Module[{nPck=npck},
(*Se compruebe en que momento saldr\[AAcute] el paquete y se actualiza checkini*)
If[checkTime>=pck[[1]],checkIni=checkTime;,checkIni=pck[[1]];];
(*Hasta sabe si hay error o no el checktime contine el tiempo abosluto en el que se termina de enviar al paquete*)
checkTime=checkIni+(pck[[2]]*lspeed);
(*se comprueba si hay error o no*)
If[Random[]<=perror,
(*si hay error poner checktime a cuando llega el nack*)
checkTime+=2 tp+tack;
endTime=checkTime;
(*se inicializa el paquete err\[OAcute]neo y se envian todo lso de la ventan de error*)
NestWhileList[(GetPacketInBadPeriod[checkIni,endTime,perror,Unevaluated[arr[[nPck]]]])&,{checkIni,pck[[2]],pck[[3]],1,pck[[5]]+1},
(*1-que no te salgas del array
2-Que el paquete llege dentro del periodo de error
 3-el ack no ha llegado*)
(++nPck<=Length[arr]&&arr[[nPck]][[1]]<endTime&&(checkIni=Max[arr[[nPck]][[1]],(checkIni+arr[[nPck-1]][[2]])])<endTime)&]
(*Si no hay error se envia el paquete*)
,{{checkIni,pck[[2]],pck[[3]],0,pck[[5]]+1}}
]
];
(*Mismo funcioanmiento que en SW
NesWhilList Reenvia el paquete (y toda su ventana de error) hasta que se envie correctamente
Map recorre el array de llegadas
Flatten extrae los paquetes del arra*)
Flatten[
MapIndexed[
(*#1=paquete,
#2=el \[IAcute]ndice*)
(Module[{pck=#1,npck=First[#2]},
(*Inicalizaci\[OAcute]n, llama GetPacketRTxGBN2, la condici\[OAcute]n comprueba si el primer paquete es err\[OAcute]neo o no. si es erroneo vuelve intenter enviar*)
NestWhileList[(GetPacketRTxGBN[pck,npck,pcomb,Unevaluated[arrivals]]&),GetPacketRTxGBN[#1,First[#2],pcomb,Unevaluated[arrivals]],(#[[1,4]]==1)&]
])&
,arrivals
]
,2]
];

(*Funci\[OAcute]n para lanzar una simulaci\[OAcute]n de GBN*)
LaunchSimTxGBN[tasa_,tp_,p_,n_,lambda_]:=(SetInitPar[1,tp,0];ToTime[FifoGBN[GetPacketArrival[tasa,lambda,0,n],p]]);
(*se deben mantener sin evaluar los par\[AAcute]metros de entrada ya que se usa una funci\[OAcute]n que hace esto.*)
SetAttributes[LaunchSimTxGBN,HoldAll];

]

End[]
EndPackage[]
