//Author Ainhoa Sayans
//Representa un paquete a nivel de enlace


packet inter_layer
{
    int origen;		//direcci�n origen de la capa superior o inferior (superior en caso de descenso inferior en caso de ascenso)
    int destino;	//direcci�n destino de la capa inferior en todo memento
    int protocol;

}