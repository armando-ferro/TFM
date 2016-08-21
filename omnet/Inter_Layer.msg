//Author Ainhoa Sayans
//Representa un paquete a nivel de enlace


packet inter_layer
{
    int origen;		//dirección origen de la capa superior o inferior (superior en caso de descenso inferior en caso de ascenso)
    int destino;	//dirección destino de la capa inferior en todo memento

}